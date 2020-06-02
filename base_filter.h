#pragma once

#include <stdint.h>

namespace filter {
	class BaseFilter;
}

class filter::BaseFilter {
public:
	virtual void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) = 0;
};
