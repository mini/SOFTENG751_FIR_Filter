#include "opencl_fft_filter.h"
#include "clFFT.h"


void filter::OpenCLFFT::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	uint64_t FFT_size = pow(2, (ceil(log(inputLength + weightsLength - 1) / log(2))));
	clfftPlanHandle planHandle;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { FFT_size };

	float* freqInput = (float*)calloc(inputLength, sizeof(float));
	float* freqOutput = (float*)calloc(inputLength, sizeof(float));
	float* freqWeights = (float*)calloc(weightsLength, sizeof(float));

	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	cl_mem timeInputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputLength * sizeof(float), input, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, inputLength * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weightsLength * sizeof(float), weights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, weightsLength * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	err = clfftCreateDefaultPlan(&planHandle, context, dim, clLengths);
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE_FAST);
	err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
	err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);

	err = clEnqueueWriteBuffer(command_queue, timeInputBuffer, CL_TRUE, 0, inputLength * sizeof(float), input, 0, NULL, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &timeInputBuffer, &freqInputBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, freqInputBuffer, CL_TRUE, 0, inputLength * sizeof(float), freqInput, 0, NULL, NULL);

	err = clEnqueueWriteBuffer(command_queue, weightsBuffer, CL_TRUE, 0, weightsLength * sizeof(float), weights, 0, NULL, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, freqWeightsBuffer, CL_TRUE, 0, weightsLength * sizeof(float), freqWeights, 0, NULL, NULL);
	
	OpenCLTimeDomain::doFilter(freqInput, inputLength, freqWeights, weightsLength, output);

	err = clEnqueueWriteBuffer(command_queue, freqInputBuffer, CL_TRUE, 0, (inputLength + weightsLength) * sizeof(float), output, 0, NULL, NULL);
	err = clfftEnqueueTransform(planHandle, CLFFT_BACKWARD, 1, &command_queue, 0, NULL, NULL, &freqInputBuffer, &timeInputBuffer, NULL);
	err = clFinish(command_queue);
	err = clEnqueueReadBuffer(command_queue, timeInputBuffer, CL_TRUE, 0, (inputLength + weightsLength) * sizeof(float), output, 0, NULL, NULL);

	clReleaseMemObject(timeInputBuffer);
	clReleaseMemObject(freqInputBuffer);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);

	err = clfftDestroyPlan(&planHandle);
	clfftTeardown();
}
