#pragma once

#include <stdint.h>
#include "file_utils.h"

namespace filter {
	class BaseFilter;
}

class filter::BaseFilter {
public:
	virtual void doFilter(InputFile* inputFile, InputFile* weightsFile, float* output) = 0;
	virtual void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) = 0;
};
