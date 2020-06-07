#pragma once

#include <algorithm>

#include "opencl_filter.h"
#include "CL/cl.h"

namespace filter {
	class OpenCLChunkedTimeDomain;
}

class filter::OpenCLChunkedTimeDomain : public filter::OpenCLTimeDomain {
public:
	void doFilter(InputFile* inputFile, InputFile* weightsFile, float* output);
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);
};