#include "opencl_chunked_filter.h"

//TODO Remove this, either find optimal or some equation based off of GPU specs
#define CHUNK_SIZE 1024 * 1024 * 1024 / 4 

void filter::OpenCLChunkedTimeDomain::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {

	uint64_t offset = 0;
	uint64_t step = CHUNK_SIZE; 

	for (uint64_t i = 0; i < inputLength; i += step) {
		OpenCLTimeDomain::doFilter(input + offset, std::min(step, inputLength - i), weights, weightsLength, output + offset);
		offset += step;
	}

}
