#pragma once

#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>
#include <algorithm>

#include "file_utils.h"

// Kep this up to date!
#include "basic_filter.h"
#include "opencl_filter.h"
#include "opencl_chunked_filter.h"
#include "opencl_fft_filter.h"

#define START_TIMER \
	uintmax_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

#define STOP_TIMER \
	uintmax_t stop = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); \
	printf("Time: %llu\n", stop - start)

void benchMark(std::string filterName, uint64_t time,std::string inputsPath);
void usage(void);
