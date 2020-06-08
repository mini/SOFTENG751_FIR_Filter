#include "opencl_chunked_filter.h"

//TODO Remove this, either find optimal or some equation based off of GPU specs
#define CHUNK_SIZE 1073741824llu / sizeof(float) // 1GB of floats


void filter::OpenCLChunkedTimeDomain::doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile) {
	uint64_t step = CHUNK_SIZE;

	float* samples;
	float* weights = weightsFile->read();

	uint64_t outputLength = std::min(step, inputFile->length) + weightsFile->length;
	float* output = new float[outputLength];

	for (uint64_t offset = 0; offset < inputFile->length; offset += step) {
		uint64_t chunkSize = std::min(step, inputFile->length - offset);

		samples = inputFile->read(chunkSize, offset);
		OpenCLTimeDomain::doFilter(samples, chunkSize, weights, weightsFile->length, output);
		inputFile->free(samples);
		
		outputFile->write(output, chunkSize + weightsFile->length, offset);
	}

	weightsFile->free(weights);
}