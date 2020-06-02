#pragma once

#include<cstdint>
#include<string>
#include<fstream>
#include<vector>

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#undef min

namespace filter {
	class InputFile;
	void writeOutputToFile(std::string filename, float* floats, uint64_t length);
}

class filter::InputFile {
public:
	uint64_t length;
	float* samples;

	InputFile(const std::string& filename);
	~InputFile();
private:
	std::string filename;
	bool fromTxtFile;
	HANDLE fileHandle, mapHandle;

	void readTextFloats();
	void mapInputFile();
	void checkWinAPIError(const char* location);
};