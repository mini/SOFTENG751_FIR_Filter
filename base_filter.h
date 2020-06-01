#pragma once
#include <stdint.h>

/*
	When you add a new filter:
		- Include new filter in main.h
		- Add parameter mapping in main.cpp
*/

namespace filter {
	class BaseFilter;
}

class filter::BaseFilter {
public:
	void setup(void) {};
	void cleanup(void) {};
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {};
};