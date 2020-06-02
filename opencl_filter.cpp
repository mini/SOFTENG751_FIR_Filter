#include "opencl_filter.h"

#define ENABLE_GPU true // For debugging

/*
	TODO:
		- Port to C++ version (cl.hpp)
		- Loop through platforms to get best device, currently hardcoded to first
		- Add chunking code to handle large files, could do 1gig chunks or paramterised based off of gpu memory
		- Maybe look into parrallel devices
*/

filter::OpenCLTimeDomain::OpenCLTimeDomain() {
	// Get platforms
	cl_uint num_platforms = 0;
	cl_platform_id platform;
	char msg[4096];

	err = clGetPlatformIDs(0, NULL, &num_platforms);
	printf("Platforms: %u\n", num_platforms);

	if (num_platforms) {
		cl_platform_id* platforms = new cl_platform_id[num_platforms * sizeof(cl_platform_id)];
		err = clGetPlatformIDs(num_platforms, platforms, NULL);
		checkError("clGetPlatformIDs 2");
		platform = platforms[0];
		delete[] platforms;
	} else {
		printf("No OpenCL runtime/platform installed?\n");
		exit(err);
		return;
	}

#ifdef _DEBUG // Print version info
	err = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 4096, msg, NULL);
	checkError("clGetPlatformInfo");
	printf("\t%s\n", msg);
	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 4096, msg, NULL);
	checkError("clGetPlatformInfo");
	printf("\t%s\n", msg);
	err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 4096, msg, NULL);
	checkError("clGetPlatformInfo");
	printf("\t%s\n", msg);
#endif

	// Get device, with CPU fallback
	cl_uint num_devices = 0;
	cl_device_id device;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
	printf("GPU Devices: %u\n", num_devices);
	if (num_devices && ENABLE_GPU) {
		cl_device_id* devices = new cl_device_id[num_devices * sizeof(cl_device_id)];
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
		checkError("clGetDeviceIDs GPU 2");
		device = devices[0];
		delete[] devices;
	} else {
		printf("Switching to CPU\n");
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
		if (!num_devices) {
			printf("No compatible devices\n");
			checkError("clGetDeviceIDs CPU 1");
			exit(err);
		}
		printf("CPU Devices: %u\n", num_devices);
		cl_device_id* devices = new cl_device_id[num_devices * sizeof(cl_device_id)];
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, num_devices, devices, NULL);
		checkError("clGetDeviceIDs CPU 2");
		device = devices[0];
		delete[] devices;
	}

	// Make context
	cl_context_properties props[]{ CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
	context = clCreateContext(props, 1, &device, NULL, NULL, &err);
	checkError("clCreateContext");

	// Make command queue
	command_queue = clCreateCommandQueue(context, device, 0, &err);
	checkError("clCreateCommandQueue");

	// Get and build kernel
	std::string contents = readFile("filter.cl", &err);
	checkError("readFile");
	const char* src = contents.c_str();
	size_t source_size = strlen(src);
	program = clCreateProgramWithSource(context, 1, &src, &source_size, &err);
	checkError("clCreateProgramWithSource");

	if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS) {
		fprintf(stderr, "Kernel build failed: ");
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(msg), msg, NULL);
		fprintf(stderr, "%s\n", msg);
	}

	// Create new kernel
	kernel = clCreateKernel(program, "filter", &err);
	checkError("clCreateKernel");
}

filter::OpenCLTimeDomain::~OpenCLTimeDomain() {
	clReleaseMemObject(samplesBuffer);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(outputBuffer);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
}

void filter::OpenCLTimeDomain::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	// Set args
	samplesBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputLength * sizeof(float), input, &err);
	checkError("clCreateBuffer 0");
	weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weightsLength * sizeof(float), weights, &err);
	checkError("clCreateBuffer 1");
	outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, (inputLength + weightsLength) * sizeof(float), output, &err);
	checkError("clCreateBuffer 2");

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &samplesBuffer);
	checkError("clSetKernelArg 0");
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &weightsBuffer);
	checkError("clSetKernelArg 1");
	err = clSetKernelArg(kernel, 2, sizeof(cl_ulong), &weightsLength);
	checkError("clSetKernelArg 2");
	err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &outputBuffer);
	checkError("clSetKernelArg 3");

	// Run
	const size_t globalDimension = inputLength + weightsLength;
	err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalDimension, NULL, 0, NULL, NULL);
	checkError("clEnqueueNDRangeKernel");


	// Read results
	err = clFinish(command_queue);
	checkError("clFinish");

	err = clEnqueueReadBuffer(command_queue, outputBuffer, CL_TRUE, 0, (inputLength + weightsLength) * sizeof(float), output, 0, NULL, NULL);
	checkError("clEnqueueReadBuffer");
}



std::string filter::OpenCLTimeDomain::readFile(const char* filename, cl_int* err) {
	std::ifstream file(filename, std::ios::binary | std::ios::in);
	std::string contents{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	file.close();
#ifdef _DEBUG
	printf("\n-----Kernel Source------\n%s\n--------------\n\n", contents.c_str());
#endif
	return contents;
}

void filter::OpenCLTimeDomain::checkError(const char* name) {
	if (err != CL_SUCCESS) {
		fprintf(stderr, "Error %s: %d = %s\nExiting\n", name, err, getErrorString(err));
		exit(err);
	}
}

// Sourced from https://stackoverflow.com/q/24326432
const char* filter::OpenCLTimeDomain::getErrorString(cl_int error) {
	switch (error) {
		// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}