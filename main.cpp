#include "main.h"

int main(int argc, char** argv) {
	// Keep this up to date!
	
	const char* filterName;
	const char* inputsPath;
	uint64_t length;
	const char* weightsPath;
	uint64_t length;
	const char* outputPath;
	
	filter::BaseFilter* chosenFilter;



	
}

void runFilter(filter::BaseFilter* filter, float* inputs, uint64_t length, float* weights, uint64_t weightsLength, float* output) {
	filter->setup();
	START_TIMER;
	filter->doFilter(NULL,NULL,NULL,NULL,NULL);
	STOP_TIMER;
	filter->cleanup();
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n");
	exit(1);
}
