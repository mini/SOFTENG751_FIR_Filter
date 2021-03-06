#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#undef min

namespace filter {
	class InputFile;
	class BinaryFile;
	class TextFile;

	class OutputFile;

	double compareToFile(const std::string& outputPath, const std::string& expectedPath);
}

class filter::InputFile {
public:
	uint64_t length;

	static InputFile* get(const std::string& filename);

	float* read();
	virtual float* read(uint64_t offset, uint64_t samples) = 0;
	virtual void free(void* ptr) = 0;
};

class filter::BinaryFile : public filter::InputFile {
public:
	BinaryFile(const std::string& filename);
	~BinaryFile();

	float* read(uint64_t offset, uint64_t samples);
	void free(void* ptr);

private:
	std::ifstream file;

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

class filter::OutputFile {
public:
	OutputFile(const std::string& filename);

	void write(float* data, uint64_t length, uint64_t offset = 0);

private:
	bool isTxt = false;
	std::ofstream file;
};