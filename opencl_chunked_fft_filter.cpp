#include "opencl_chunked_fft_filter.h"

// This is the limit to how large a clFFT FFT can be
#define CHUNK_SIZE 16777216llu / sizeof(float) 


void filter::OpenCLChunkedFFT::doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile) {
	uint64_t step = CHUNK_SIZE;
	
	float* samples;
	float* weights = weightsFile->read();
	uint64_t overlap = weightsFile->length - 1;
	// FFT size is the smallest power of 2 larger than the chunked input size 
	uint64_t fftSize = pow(2, (ceil(log(std::min(step, inputFile->length) + overlap) / log(2))));
	step = fftSize - overlap;
	uint64_t outputLength = std::min(fftSize, inputFile->length) + overlap;
	float* output = new float[outputLength]();

	clfftPlanHandle forwardPlan;
	clfftPlanHandle backwardPlan;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { fftSize };
	const size_t globalDimension = fftSize;

	// pad weights with zeros before FFT to match fftSize
	float* padded_weights = new float[fftSize]();
	memcpy(padded_weights, weights, weightsFile->length * sizeof(float));

	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

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

	cl_mem weightsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, fftSize * sizeof(float), padded_weights, &err);
	checkError("clCreateBuffer 0");
	cl_mem freqWeightsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, fftSize * sizeof(cl_float2), NULL, &err);
	checkError("clCreateBuffer 0");

	err = clfftEnqueueTransform(forwardPlan, CLFFT_FORWARD, 1, &command_queue, 0, NULL, NULL, &weightsBuffer, &freqWeightsBuffer, NULL);
	err = clFinish(command_queue);

	for (uint64_t offset = 0; offset < inputFile->length; offset += step) {
		uint64_t chunkSize = std::min(fftSize, inputFile->length - offset);
		samples = inputFile->read(chunkSize, offset);
		OpenCLFFT::doFilter(samples, freqWeightsBuffer, output, chunkSize, weightsFile->length, fftSize, forwardPlan, backwardPlan);
		inputFile->free(samples);
		outputFile->write(output, chunkSize + overlap, offset);
	}
	
	weightsFile->free(weights);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);
	err = clfftDestroyPlan(&forwardPlan);
	err = clfftDestroyPlan(&backwardPlan);
	clfftTeardown();

}