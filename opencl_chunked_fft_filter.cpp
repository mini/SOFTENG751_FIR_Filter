#include "opencl_chunked_fft_filter.h"

//TODO Remove this, either find optimal or some equation based off of GPU specs
#define CHUNK_SIZE 4000llu / sizeof(float) // 1GB of floats


void filter::OpenCLChunkedFFT::doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile) {
	uint64_t step = CHUNK_SIZE;

	float* samples = inputFile->read();
	float* weights = weightsFile->read();


	uint64_t overlap = weightsFile->length - 1;
	uint64_t fftSize = pow(2, (ceil(log(std::min(step, inputFile->length) + overlap) / log(2))));
	step = fftSize - overlap;
	uint64_t outputLength = std::min(fftSize, inputFile->length) + overlap;
	float* output = new float[outputLength]();

	clfftPlanHandle forwardPlan;
	clfftPlanHandle backwardPlan;
	clfftDim dim = CLFFT_1D;
	size_t clLengths[1] = { fftSize };
	const size_t globalDimension = fftSize;

	float* padded_weights = (float*)calloc(fftSize, sizeof(float));
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

		OpenCLFFT::doFilter(samples + offset, freqWeightsBuffer, output, chunkSize, weightsFile->length, fftSize, forwardPlan, backwardPlan);

		outputFile->write(output, chunkSize + overlap, offset);
	}
	
	weightsFile->free(weights);
	inputFile->free(samples);
	clReleaseMemObject(weightsBuffer);
	clReleaseMemObject(freqWeightsBuffer);
	err = clfftDestroyPlan(&forwardPlan);
	err = clfftDestroyPlan(&backwardPlan);
	clfftTeardown();

}