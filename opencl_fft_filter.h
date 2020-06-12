#pragma once

#include <algorithm>

#include "opencl_filter.h"
#include "CL/cl.h"

namespace filter {
	class OpenCLFFT;
}

class filter::OpenCLFFT : public filter::OpenCLTimeDomain {
public:
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);
	OpenCLFFT() : OpenCLTimeDomain("filter_fd") {};
};
