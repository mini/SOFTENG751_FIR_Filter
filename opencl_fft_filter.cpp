#include "opencl_fft_filter.h"
#include "clFFT.h"

void filter::OpenCLFFT::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	uint64_t FFT_size = pow(2, (ceil(log(inputLength + weightsLength - 1) / log(2))));
	clfftPlanHandle planHandle;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { FFT_size };

	float* freqInput = (float*)calloc(FFT_size, sizeof(float));
	float* freqOutput = (float*)calloc(FFT_size, sizeof(float));
	float* freqWeights = (float*)calloc(FFT_size, sizeof(float));

	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	cl_mem timeBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputLength * sizeof(float), input, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqOutputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weightsLength * sizeof(float), weights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	err = clfftCreateDefaultPlan(&planHandle, context, dim, clLengths);
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
	err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);

	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &timeBuffer, &freqInputBuffer, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &freqInputBuffer);
	checkError("clSetKernelArg 0");
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &freqWeightsBuffer);
	checkError("clSetKernelArg 1");
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &freqOutputBuffer);
	checkError("clSetKernelArg 3");

	// Run
	const size_t globalDimension = FFT_size;
	err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalDimension, NULL, 0, NULL, NULL);
	checkError("clEnqueueNDRangeKernel");



	err = clfftSetLayout(planHandle, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);

	err = clfftEnqueueTransform(planHandle, CLFFT_BACKWARD, 1, &command_queue, 0, NULL, NULL, &freqOutputBuffer, &timeBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, timeBuffer, CL_TRUE, 0, inputLength * sizeof(float), output, 0, NULL, NULL);

	clReleaseMemObject(timeBuffer);
	clReleaseMemObject(freqInputBuffer);
	clReleaseMemObject(freqOutputBuffer);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);

	err = clfftDestroyPlan(&planHandle);
	clfftTeardown();
}
