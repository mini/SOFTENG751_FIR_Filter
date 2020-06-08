#pragma once

#include <algorithm>

#include "opencl_filter.h"
#include "basic_filter.h"

namespace filter {
	class OpenCLChunkedTimeDomain;
}

class filter::OpenCLChunkedTimeDomain : public filter::OpenCLTimeDomain {
public:
	void doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile);
};