#pragma once

#include <algorithm>
#include "clFFT.h"
#include "opencl_filter.h"
#include "CL/cl.h"

namespace filter {
	class OpenCLFFT;
}

class filter::OpenCLFFT : public filter::OpenCLTimeDomain {
public:
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);
	void doFilter(float* input, cl_mem weightsBuffer, float* output, uint64_t inputLength, uint64_t weightsLength, uint64_t FFT_size, clfftPlanHandle forwardPlan, clfftPlanHandle backwardsPlan);
	OpenCLFFT() : OpenCLTimeDomain("filter_fd") {};
};
