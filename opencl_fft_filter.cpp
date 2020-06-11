#include "opencl_fft_filter.h"
#include "clFFT.h"


void filter::OpenCLFFT::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	uint64_t FFT_size = 2^(int)(ceil(log(inputLength + weightsLength - 1)/log(2)));
	clfftPlanHandle planHandle;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { FFT_size };

	float* paddedInput = (float*)malloc(FFT_size * sizeof(float));
	memcpy(paddedInput, input, sizeof(input));
	memset(paddedInput + sizeof(input), 0, (FFT_size - inputLength) * sizeof(float));

	float* paddedWeights = (float*)malloc(FFT_size * sizeof(float));
	memcpy(paddedWeights, weights, sizeof(weights));
	memset(paddedWeights + sizeof(weights), 0, (FFT_size - weightsLength) * sizeof(float));

	float* freqInput = (float*)calloc(FFT_size, sizeof(float));
	float* freqOutput = (float*)calloc(FFT_size, sizeof(float));
	float* freqWeights = (float*)calloc(FFT_size, sizeof(float));

	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	cl_mem timeInputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, FFT_size * sizeof(float), paddedInput, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), freqInput, &err);
	checkError("clCreateBuffer 0");

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, FFT_size * sizeof(float), paddedWeights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), freqWeights, &err);
	checkError("clCreateBuffer 0");

	err = clfftCreateDefaultPlan(&planHandle, context, dim, clLengths);
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE_FAST);
	err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
	err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);

	err = clEnqueueWriteBuffer(command_queue, timeInputBuffer, CL_TRUE, 0, FFT_size * sizeof(float), paddedInput, 0, NULL, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &timeInputBuffer, &freqInputBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, freqInputBuffer, CL_TRUE, 0, FFT_size * sizeof(float), freqInput, 0, NULL, NULL);

	err = clEnqueueWriteBuffer(command_queue, weightsBuffer, CL_TRUE, 0, FFT_size * sizeof(float), paddedWeights, 0, NULL, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, freqWeightsBuffer, CL_TRUE, 0, FFT_size * sizeof(float), freqWeights, 0, NULL, NULL);

	OpenCLTimeDomain::doFilter(freqInput, FFT_size, freqWeights, FFT_size, output);

	clReleaseMemObject(timeInputBuffer);
	clReleaseMemObject(freqInputBuffer);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);
	err = clfftDestroyPlan(&planHandle);
	clfftTeardown();
}
