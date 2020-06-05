#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iterator>
#include <fstream>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 120

#include "base_filter.h"
#include "CL/cl.h"

namespace filter {
	class OpenCLTimeDomain;
}

class filter::OpenCLTimeDomain : public filter::BaseFilter {
public:
	OpenCLTimeDomain();
	~OpenCLTimeDomain();
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);

private:
	cl_int err;
	cl_context context;
	cl_kernel kernel;
	cl_command_queue command_queue;
	cl_program program;

	std::string readFile(const char* filename, cl_int* err);
	void checkError(const char* name);
	static const char* getErrorString(cl_int error);
};