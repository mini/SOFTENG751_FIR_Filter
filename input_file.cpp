#include<cstdint>
#include<string>
#include<fstream>
#include<vector>

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#undef min


class InputFile {
public:
	uint64_t length;
	float* samples;

	InputFile(const std::string& filename) {
		this->filename = filename;
		fromTxtFile = filename.length() >= 4 && filename.compare(filename.length() - 4, 4, ".txt") == 0;

		if (fromTxtFile) {
			readTextFloats();
		} else {
			mapInputFile();
		}
	}

	~InputFile() {
		if (fromTxtFile) {
			delete[] samples;
		} else {
			//Close file handles
			UnmapViewOfFile(viewHandle);
			CloseHandle(mapHandle);
			CloseHandle(fileHandle);
		}
	}

private:
	std::string filename;
	bool fromTxtFile;
	HANDLE fileHandle, mapHandle;
	LPVOID viewHandle;

	void readTextFloats() {
		std::fstream fs(filename, std::ios::in);
		std::vector<float> floats;
		std::string line;

		if (!fs.is_open()) {
			printf("File not found: %s\n", filename.c_str());
			exit(1);
		}

		while (std::getline(fs, line)) {
			floats.push_back((float)atof(line.c_str()));
		}

		fs.close();
		length = floats.size();

		// TODO Shouldn't copy array.
		// Can't think of a way to return the vector's backend without deallocation problems
		samples = new float[length];
		std::copy(floats.begin(), floats.end(), samples);
	}

	void mapInputFile() {
		//TODO look into large pages? *nix port? FILE_FLAG_OVERLAPPED?
		wchar_t* widePath = new wchar_t[filename.length() + 1];
		mbstowcs(widePath, filename.c_str(), filename.length() + 1);

		fileHandle = CreateFile((LPCWSTR)widePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		checkWinAPIError("File handle");

		mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, (LPCWSTR)widePath);
		checkWinAPIError("Map handle");

		viewHandle = MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, 0);
		samples = reinterpret_cast<float*>(viewHandle);
		checkWinAPIError("Map view");

		DWORD hiSize;
		DWORD loSize = GetFileSize(fileHandle, &hiSize);
		length = (((uint64_t)hiSize) << (8 * sizeof(DWORD))) + loSize;
		length /= sizeof(float);

		delete[] widePath;
	}

	void checkWinAPIError(const char* location) {
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
};