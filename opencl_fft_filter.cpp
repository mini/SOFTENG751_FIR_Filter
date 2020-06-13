#include "opencl_fft_filter.h"

void filter::OpenCLFFT::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	uint64_t FFT_size = pow(2, (ceil(log(inputLength + weightsLength - 1) / log(2))));
	clfftPlanHandle forwardPlan;
	clfftPlanHandle backwardPlan;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { FFT_size };
	const size_t globalDimension = FFT_size;

	float* padded_input = (float*)calloc(FFT_size, sizeof(float));
	float* padded_weights = (float*)calloc(FFT_size, sizeof(float));

	memcpy(padded_input, input, inputLength * sizeof(float));
	memcpy(padded_weights, weights, weightsLength * sizeof(float));


	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, FFT_size * sizeof(float), padded_weights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	err = clfftCreateDefaultPlan(&forwardPlan, context, dim, clLengths);
	err = clfftSetPlanPrecision(forwardPlan, CLFFT_SINGLE);
	err = clfftSetLayout(forwardPlan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	err = clfftSetResultLocation(forwardPlan, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(forwardPlan, 1, &command_queue, NULL, NULL);

	err = clfftCreateDefaultPlan(&backwardPlan, context, dim, clLengths);
	err = clfftSetPlanPrecision(backwardPlan, CLFFT_SINGLE);
	err = clfftSetLayout(backwardPlan, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	err = clfftSetResultLocation(backwardPlan, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(backwardPlan, 1, &command_queue, NULL, NULL);

	err = clfftEnqueueTransform(forwardPlan, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);
	cl_float2* weightsFD = (cl_float2*)calloc(FFT_size, sizeof(cl_float2));;
	err = clEnqueueReadBuffer(command_queue, freqWeightsBuffer, CL_TRUE, 0, FFT_size * sizeof(cl_float2), weightsFD, 0, NULL, NULL);

	OpenCLFFT::doFilter(padded_input, weightsFD, output, inputLength, weightsLength, FFT_size, forwardPlan, backwardPlan);

	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);

	err = clfftDestroyPlan(&forwardPlan);
	err = clfftDestroyPlan(&backwardPlan);
	clfftTeardown();
}

void filter::OpenCLFFT::doFilter(float* input, cl_float2* weightsFD, float* output, uint64_t inputLength, uint64_t weightsLength, uint64_t FFT_size, clfftPlanHandle forwardPlan, clfftPlanHandle backwardsPlan) {
	// fft on input
	cl_mem timeBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, FFT_size * sizeof(float), input, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(cl_float2), NULL, &err);
	checkError("clCreateBuffer 0");
	err = clfftEnqueueTransform(forwardPlan, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &timeBuffer, &freqInputBuffer, NULL);

	cl_mem freqOutputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(cl_float2), NULL, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, FFT_size * sizeof(cl_float2), weightsFD, &err);
	checkError("clCreateBuffer 0");
	const size_t globalDimension = FFT_size;
	
	// circular transform kernel
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &freqInputBuffer);
	checkError("clSetKernelArg 0");
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &freqWeightsBuffer);
	checkError("clSetKernelArg 1");
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &freqOutputBuffer);
	checkError("clSetKernelArg 3");
	err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalDimension, NULL, 0, NULL, NULL);
	checkError("clEnqueueNDRangeKernel");
	err = clFinish(command_queue);

	// ifft on output
	err = clfftEnqueueTransform(backwardsPlan, CLFFT_BACKWARD, 1, &command_queue, 0, NULL, NULL, &freqOutputBuffer, &timeBuffer, NULL);
	err = clEnqueueReadBuffer(command_queue, timeBuffer, CL_TRUE, 0, (inputLength + weightsLength - 1) * sizeof(float), output, 0, NULL, NULL);

	clReleaseMemObject(freqInputBuffer);
	clReleaseMemObject(freqOutputBuffer);
	clReleaseMemObject(timeBuffer);
	clReleaseMemObject(freqWeightsBuffer);
}
