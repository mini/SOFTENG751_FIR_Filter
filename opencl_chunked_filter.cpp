#include "opencl_chunked_filter.h"

//TODO Remove this, either find optimal or some equation based off of GPU specs
#define CHUNK_SIZE 1024 * 1024 * 1024 / 4 


void filter::OpenCLChunkedTimeDomain::doFilter(InputFile* inputFile, InputFile* weightsFile, float* output) {
	uint64_t offset = 0;
	uint64_t step = CHUNK_SIZE;


	float* samples;
	float* weights = weightsFile->read();
	for (uint64_t i = 0; i < inputFile->length; i += step) {
		uint64_t chunkSize = std::min(step, inputFile->length - i);
		samples = inputFile->read(chunkSize, i);
		OpenCLTimeDomain::doFilter(samples, chunkSize, weights, weightsFile->length, output + offset);
		offset += step;
	}
}


void filter::OpenCLChunkedTimeDomain::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
}