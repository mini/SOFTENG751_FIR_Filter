#include "file_utils.h"

#define TXT_EXT ".txt"
#define EXT_LEN 4

filter::InputFile::InputFile(const std::string& filename) {
	this->filename = filename;
	fromTxtFile = filename.length() >= EXT_LEN && filename.compare(filename.length() - EXT_LEN, EXT_LEN, TXT_EXT) == 0;

	if (fromTxtFile) {
		readTextFloats();
	} else {
		mapInputFile();
	}
}

filter::InputFile::~InputFile() {
	if (fromTxtFile) {
		delete[] samples;
	} else {
		UnmapViewOfFile(samples);
		CloseHandle(mapHandle);
		CloseHandle(fileHandle);
	}
}

void filter::InputFile::readTextFloats() {
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

void filter::InputFile::mapInputFile() {
	//TODO look into large pages? *nix port? FILE_FLAG_OVERLAPPED?
	wchar_t* widePath = new wchar_t[filename.length() + 1];
	mbstowcs(widePath, filename.c_str(), filename.length() + 1);

	fileHandle = CreateFile((LPCWSTR)widePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	checkWinAPIError("File handle");

	mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, (LPCWSTR)widePath);
	checkWinAPIError("Map handle");

	samples = reinterpret_cast<float*>(MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, 0));
	checkWinAPIError("Map view");

	DWORD hiSize;
	DWORD loSize = GetFileSize(fileHandle, &hiSize);
	length = (((uint64_t)hiSize) << (8 * sizeof(DWORD))) + loSize;
	length /= sizeof(float);

	delete[] widePath;
}

void filter::InputFile::checkWinAPIError(const char* location) {
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

void filter::writeOutputToFile(std::string filename, float* floats, uint64_t length) {
	int toTxtFile = filename.length() >= EXT_LEN && filename.compare(filename.length() - EXT_LEN, EXT_LEN, TXT_EXT) == 0 ? ~0 : 0;
	std::ofstream file(filename, std::ios::out | (std::ios::binary & ~toTxtFile));

	if (!file.is_open()) {
		printf("Can't open file for writing\n");
		exit(1);
	}

	if (toTxtFile) {
		char buffer[24];
		for (uint64_t i = 0; i < length; i++) {
			std::snprintf(buffer, sizeof(buffer), "%f\n", floats[i]);
			file.write(buffer, strlen(buffer));
		}
	} else {
		file.write(reinterpret_cast<const char*>(floats), sizeof(float) * length);
	}
	file.close();
}