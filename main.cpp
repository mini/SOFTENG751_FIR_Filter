#include "main.h"

int main(int argc, char** argv) {
	// Keep this up to date!
	
	const char* filterName;
	const char* inputsPath;
	uint64_t intputsLength;
	const char* weightsPath;
	uint64_t weightsLength; // calc later
	const char* outputPath;
	const char* expectedOutputPath;
	
}

void runFilter(filter::BaseFilter* filter, float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	filter->setup();
	START_TIMER;
	filter->doFilter(input, inputLength, weights, weightsLength, output);
	STOP_TIMER;
	filter->cleanup();
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n");
	exit(1);
}
