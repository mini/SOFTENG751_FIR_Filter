#pragma once

#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>


// Keep this up to date!
#include "base_filter.h"
#include "basic_filter.cpp"
#include "opencl_filter.cpp"

#define START_TIMER \
	uintmax_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

#define STOP_TIMER \
	uintmax_t stop = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); \
	printf("Time: %llu\n", stop - start)

void usage(void);
void checkWinAPIError(const char* location);
float* readTextFloats(const char* filename, uint64_t* length);
float* mapInputFile(const char* filename, uint64_t* length);
