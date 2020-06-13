#pragma once

#include <algorithm>
#include "opencl_filter.h"
#include "opencl_fft_filter.h"
#include "basic_filter.h"

namespace filter {
	class OpenCLChunkedFFT;
}

class filter::OpenCLChunkedFFT : public filter::OpenCLFFT {
public:
	void doFilter(InputFile* inputFile, InputFile* weightsFile, OutputFile* outputFile);
};