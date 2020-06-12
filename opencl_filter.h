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
	OpenCLTimeDomain(const char* kernel_name);
	OpenCLTimeDomain() : OpenCLTimeDomain("filter") {}
	~OpenCLTimeDomain();
	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output);

protected:
	cl_int err;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;

	std::string readFile(std::string filename, cl_int* err);
	void checkError(const char* name);
	static const char* getErrorString(cl_int error);
	cl_kernel kernel;
};