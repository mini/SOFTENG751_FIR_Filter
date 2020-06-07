#pragma once

#include <cstdint>
#include "base_filter.h"

namespace filter {
	class BasicTimeDomain;
}

class filter::BasicTimeDomain : public filter::BaseFilter {
public:
	void doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile);
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);
};