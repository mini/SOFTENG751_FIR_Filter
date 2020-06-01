#pragma once

#include <stdio.h>
#include <cstdint>
#include <fstream>
#include <chrono>

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