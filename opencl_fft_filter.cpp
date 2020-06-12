#include "opencl_fft_filter.h"
#include "clFFT.h"

void filter::OpenCLFFT::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	uint64_t overlap = weightsLength - 1;
	uint64_t FFT_size = pow(2, (ceil(log(8*weightsLength) / log(2))));
	uint64_t step_size = FFT_size - overlap;
	uint64_t position = 0;
	clfftPlanHandle planHandle;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { FFT_size };
	const size_t globalDimension = FFT_size;
	
	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	cl_mem timeBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	cl_mem freqInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqOutputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weightsLength * sizeof(float), weights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * FFT_size * sizeof(float), NULL, &err);
	checkError("clCreateBuffer 0");

	err = clfftCreateDefaultPlan(&planHandle, context, dim, clLengths);
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
	err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_COMPLEX_INTERLEAVED);
	err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);
	err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);

	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);

	while (position + FFT_size <= inputLength) {
		err = clEnqueueWriteBuffer(command_queue, timeBuffer, CL_TRUE, 0, FFT_size * sizeof(float), input + position, 0, NULL, NULL);

		err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_COMPLEX_INTERLEAVED);
		err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);
		err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &timeBuffer, &freqInputBuffer, NULL);

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &freqInputBuffer);
		checkError("clSetKernelArg 0");
		err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &freqWeightsBuffer);
		checkError("clSetKernelArg 1");
		err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &freqOutputBuffer);
		checkError("clSetKernelArg 3");
		err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalDimension, NULL, 0, NULL, NULL);
		checkError("clEnqueueNDRangeKernel");

		err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_REAL);
		err = clfftBakePlan(planHandle, 1, &command_queue, NULL, NULL);
		err = clfftEnqueueTransform(planHandle, CLFFT_BACKWARD, 1, &command_queue, 0, NULL, NULL, &freqOutputBuffer, &timeBuffer, NULL);

		// offset to buffer is overlap to discard overlapping values. Offset to output is position
		err = clEnqueueReadBuffer(command_queue, freqInputBuffer, CL_TRUE, overlap * sizeof(float), step_size * sizeof(float), output + position, 0, NULL, NULL);
		position += step_size;
	}






	clReleaseMemObject(freqInputBuffer);
	clReleaseMemObject(freqOutputBuffer);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(timeBuffer);
	clReleaseMemObject(freqWeightsBuffer);

	err = clfftDestroyPlan(&planHandle);
	clfftTeardown();
}
