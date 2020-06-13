#include "basic_filter.h"

void filter::BasicTimeDomain::doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile) {
	float* samples = inputFile->read();
	float* weights = weightsFile->read();
	uint64_t outputLength = inputFile->length + weightsFile->length;
	float* output = new float[outputLength]();

	doFilter(samples, inputFile->length, weights, weightsFile->length, output);

	inputFile->free(samples);
	weightsFile->free(weights);
	outputFile->write(output, outputLength);
}

void filter::BasicTimeDomain::doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
	for (uint64_t i = 0; i < inputLength + weightsLength; i++) {
		for (uint64_t j = 0; j < weightsLength; j++) {
			uint64_t index = i - j;
			if (index >= 0 && index < inputLength) {
				output[i] += input[index] * weights[j];
			}
		}
	}
}
