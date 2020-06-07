#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#undef min

namespace filter {
	class InputFile;
	class MappedFile;
	class TextFile;
	void writeOutputToFile(std::string filename, float* floats, uint64_t length);
	double compareToFile(float* output, uint64_t length, std::string filename);
}

class filter::InputFile {
public:
	uint64_t length;

	static InputFile* get(const std::string& filename);

	float* read();
	virtual float* read(uint64_t offset, uint64_t samples) = 0;
	virtual void free(void* ptr) = 0;
};

class filter::MappedFile : public filter::InputFile {
public:
	MappedFile(const std::string& filename);
	~MappedFile();

	float* read(uint64_t offset, uint64_t samples);
	void free(void* ptr);

private:
	HANDLE fileHandle, mapHandle;

	void checkWinAPIError(const char* location);
};

class filter::TextFile : public filter::InputFile {
public:
	TextFile(const std::string& filename);
	~TextFile();

	float* read(uint64_t offset, uint64_t samples);
	void free(void* ptr);

private:
	std::ifstream file;
	float* samples;
};