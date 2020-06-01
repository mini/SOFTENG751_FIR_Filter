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
	void doFilter(float* inputs, uint64_t length, float* weights, uint64_t weightsLength, float* output) {};
};