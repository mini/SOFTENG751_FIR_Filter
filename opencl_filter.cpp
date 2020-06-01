#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iterator>
#include <fstream>

#include "base_filter.h"
#include "CL/cl.h"

#define ENABLE_GPU true // Leave on, one of the getDevices funcs for CPU fails

namespace filter {
	class OpenCLTimeDomain;
}

class filter::OpenCLTimeDomain: public filter::BaseFilter {
public:

	OpenCLTimeDomain(void) {

		// Get platforms
		cl_uint num_platforms;
		cl_platform_id platform;
		char msg[4096];

		err = clGetPlatformIDs(1, NULL, &num_platforms);
		checkError(err, "clGetPlatformIDs");

		if (num_platforms) {
			cl_platform_id* platforms = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id));
			err = clGetPlatformIDs(num_platforms, platforms, NULL);
			checkError(err, "clGetPlatformIDs");
			platform = platforms[0];
			free(platforms);
		} else {
			printf("What\n"); // Should never happen
			return;
		}

		printf("Platforms: %d\n", num_platforms);

		// Get device, with CPU fallback

		cl_uint num_devices;
		cl_device_id device;
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
		checkError(err, "clGetDeviceIDs GPU 1");
		if (num_devices && ENABLE_GPU) {
			cl_device_id* devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
			checkError(err, "clGetDeviceIDs GPU 2");
			device = devices[0];
			free(devices);
		} else {
			printf("Switching to CPU\n");
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
			checkError(err, "clGetDeviceIDs CPU 1");

			cl_device_id* devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, num_devices, devices, NULL);
			checkError(err, "clGetDeviceIDs CPU 2");
			device = devices[0];
			free(devices);
		}

#ifdef _DEBUG // Printing device version info
		err = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 4096, msg, NULL);
		checkError(err, "clGetPlatformInfo");
		printf("\t%s\n", msg);
		err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 4096, msg, NULL);
		checkError(err, "clGetPlatformInfo");
		printf("\t%s\n", msg);
		err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 4096, msg, NULL);
		checkError(err, "clGetPlatformInfo");
		printf("\t%s\n", msg);
#endif

		// Make context
		cl_context_properties props[]{ CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
		context = clCreateContext(props, 1, &device, NULL, NULL, &err);
		checkError(err, "clCreateContext");

		// Make command queue
		command_queue = clCreateCommandQueue(context, device, 0, &err);
		checkError(err, "clCreateCommandQueue");

		// Get and build kernel
		std::string contents = readFile("filter.cl", &err);
		checkError(err, "readFile");
		const char* src = contents.c_str();
		size_t source_size = strlen(src);
		program = clCreateProgramWithSource(context, 1, &src, &source_size, &err);
		checkError(err, "clCreateProgramWithSource");

		if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS) {
			fprintf(stderr, "Kernel build failed: ");
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(msg), msg, NULL);
			fprintf(stderr, "%s\n", msg);
		}

		// Create new kernel
		kernel = clCreateKernel(program, "filter", &err);
		checkError(err, "clCreateKernel");
	}

	~OpenCLTimeDomain() {
		clReleaseMemObject(samplesBuffer);
		clReleaseMemObject(weightsBuffer);
		clReleaseMemObject(outputBuffer);
		clReleaseProgram(program);
		clReleaseKernel(kernel);
		clReleaseCommandQueue(command_queue);
		clReleaseContext(context);
	}

	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
		// Set args
		samplesBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputLength * sizeof(float), input, &err);
		checkError(err, "clCreateBuffer 0");
		weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weightsLength * sizeof(float), weights, &err);
		checkError(err, "clCreateBuffer 1");
		outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, (inputLength + weightsLength) * sizeof(float), output, &err);
		checkError(err, "clCreateBuffer 2");

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &samplesBuffer);
		checkError(err, "clSetKernelArg 0");
		err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &weightsBuffer);
		checkError(err, "clSetKernelArg 1");
		err = clSetKernelArg(kernel, 2, sizeof(cl_ulong), &weightsLength);
		checkError(err, "clSetKernelArg 2");
		err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &outputBuffer);
		checkError(err, "clSetKernelArg 3");

		// Run
		const size_t globalDimension = inputLength + weightsLength;
		err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalDimension, NULL, 0, NULL, NULL);
		checkError(err, "clEnqueueNDRangeKernel");


		// Read results
		err = clFinish(command_queue);
		checkError(err, "clFinish");

		err = clEnqueueReadBuffer(command_queue, outputBuffer, CL_TRUE, 0, (inputLength + weightsLength) * sizeof(float), output, 0, NULL, NULL);
		checkError(err, "clEnqueueReadBuffer");
	}

private:
	cl_int err;
	cl_context context;
	cl_kernel kernel;
	cl_command_queue command_queue;
	
	cl_mem samplesBuffer, weightsBuffer, outputBuffer;
	cl_program program;

	std::string readFile(const char* filename, cl_int* err) {
		std::ifstream file(filename, std::ios::binary | std::ios::in);
		std::string contents{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
		file.close();
#ifdef _DEBUG
		printf("\n-----Kernel Source------\n%s\n--------------\n\n", contents.c_str());
#endif
		return contents;
	}

	static void checkError(cl_int error, const char* name) {
		if (error != CL_SUCCESS) {
			fprintf(stderr, "Error(%d): %s\nExiting\n", error, name);
			exit(error);
		}
	}
};