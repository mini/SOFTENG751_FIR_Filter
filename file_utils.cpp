#include "file_utils.h"

#define TXT_EXT ".txt"
#define EXT_LEN 4

#define CHUNK_SIZE 1073741824llu / sizeof(float) // 1GB of floats


namespace filter {

	// InputFile (base class)

	InputFile* InputFile::get(const std::string& filename) {
		bool isTxt = filename.length() >= EXT_LEN && filename.compare(filename.length() - EXT_LEN, EXT_LEN, TXT_EXT) == 0;
		if (isTxt) {
			return new TextFile(filename);
		} else {
			return new MappedFile(filename);
		}
	}

	float* InputFile::read() {
		return read(length, 0);
	}

	// MappedFile

	MappedFile::MappedFile(const std::string& filename) {
		wchar_t* widePath = new wchar_t[filename.length() + 1];
		mbstowcs(widePath, filename.c_str(), filename.length() + 1);
		fileHandle = CreateFile((LPCWSTR)widePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE) {
			checkWinAPIError("File handle");
		}

		mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, (LPCWSTR)widePath);
		if (!mapHandle) {
			checkWinAPIError("Map handle");
		}

		DWORD hiSize;
		DWORD loSize = GetFileSize(fileHandle, &hiSize);
		if (loSize == INVALID_FILE_SIZE) {
			checkWinAPIError("File Size");
		}
		length = (((uint64_t)hiSize) << (8 * sizeof(DWORD))) + loSize;
		length /= sizeof(float);

		delete[] widePath;
	}

	MappedFile::~MappedFile() {
		CloseHandle(mapHandle);
		CloseHandle(fileHandle);
	}

	float* MappedFile::read(uint64_t nSamples, uint64_t offset) {
		//TODO look into large pages? *nix port? FILE_FLAG_OVERLAPPED?
		DWORD hiOff = (DWORD)((offset & 0xFFFFFFFF00000000llu) >> 32);
		DWORD loOff = (DWORD)offset;
		float* samples = reinterpret_cast<float*>(MapViewOfFile(mapHandle, FILE_MAP_READ, hiOff, loOff, nSamples * sizeof(float)));
		if (!samples) {
			checkWinAPIError("Map view");
		}
		return samples;
	}

	void MappedFile::free(void* ptr) {
		if (!UnmapViewOfFile(ptr)) {
			checkWinAPIError("Free");
		}
	}

	void MappedFile::checkWinAPIError(const char* location) {
		DWORD errorID = GetLastError();
		if (errorID) {
			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
			std::string message(messageBuffer, size);
			LocalFree(messageBuffer);
			printf("%s: %s (%lu)", location, message.c_str(), errorID);
			exit(errorID);
		}
	}

	// TextFile

	TextFile::TextFile(const std::string& filename) {
		file.open(filename);
		if (!file.is_open()) {
			printf("File not found: %s\n", filename.c_str());
			exit(1);
		}

		file.unsetf(std::ios::skipws);
		length = std::count(
			std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),
			'\n'
			);

		file.seekg(-1, std::ios::end);
		char a = 0;
		file.get(a);
		if (a != '\n') {
			length++;
		}

		file.setf(std::ios::skipws);
		file.clear();
		file.seekg(0, std::ios::beg);


		samples = new float[length];
		std::string line;
		for (uint64_t i = 0; i < length; i++) {
			std::getline(file, line);
			samples[i] = (float)atof(line.c_str());
		}

		file.close();
	}

	TextFile::~TextFile() {
		delete[] samples;
	}

	float* TextFile::read(uint64_t nSamples, uint64_t offset) {
		return samples + offset;
	}

	void TextFile::free(void* ptr) {

	}

	// Other util funcs


	double compareToFile(const std::string& outputPath, const std::string& expectedPath) {
		InputFile* output = InputFile::get(outputPath);
		InputFile* expected = InputFile::get(expectedPath);

		uint64_t outputLength = output->length;
		if (outputLength != expected->length) {
			return 0.0f;
		}

		float* outputSamples;
		float* expectedSamples;
		uint64_t chunkSize;
		uint64_t closeEnough = 0;
		for (uint64_t offset = 0; offset < outputLength; offset += CHUNK_SIZE) {
			chunkSize = std::min(CHUNK_SIZE, outputLength - offset);
			outputSamples = output->read(chunkSize, offset);
			expectedSamples = expected->read(chunkSize, offset);

			// Turns out comparing floats across architectures leads down a REALLY DEEP rabbit hole.
			// So we're only going to compare numbers up to 6dp, anything smaller will be equal.
			// And also just returning a percentage match, not a hard boolean

			for (uint64_t i = 0; i < chunkSize; i++) {
				if (fabs(trunc(100000.0 * outputSamples[i]) - trunc(100000.0 * expectedSamples[i])) <= 1.0) {
					closeEnough++;
				}
			}

			output->free(outputSamples);
			expected->free(expectedSamples);
		}

		delete output;
		delete expected;
		return 100 * closeEnough / (double)outputLength;
	}

	// Output file

	OutputFile::OutputFile(const std::string& filename) {
		if (!filename.empty()) {
			isTxt = filename.length() >= EXT_LEN && filename.compare(filename.length() - EXT_LEN, EXT_LEN, TXT_EXT) == 0;
			if (isTxt) {
				file.open(filename, std::ios::out);
			} else {
				file.open(filename, std::ios::out | std::ios::binary);
			}

			if (!file.is_open()) {
				printf("Can't open file for writing\n");
				exit(1);
			}
		}
	}

	void OutputFile::write(float* data, uint64_t length, uint64_t offset) {
		if (!file.is_open()) {
			return;
		}

		if (isTxt) {
			if (offset) {
				file << "go back " << offset << " positions\n";
			}
			char buffer[24];
			for (uint64_t i = 0; i < length; i++) {
				std::snprintf(buffer, sizeof(buffer), "%.*f\n", std::numeric_limits<float>::max_digits10, data[i]);
				file.write(buffer, strlen(buffer));
			}
		} else {
			if (offset) {
				file.seekp(offset * sizeof(float));
			}
			file.write(reinterpret_cast<const char*>(data), sizeof(float) * length);
		}
	}
}