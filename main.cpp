#include "main.h"

int main(int argc, char** argv) {

#ifdef _DEBUG 
	printf("DEBUG MODE\n");
	const char* filterName = "basictd";
	const char* inputsPath = "floats.dat";
	const char* weightsPath = "weights.txt";
	const char* outputPath = "output.txt";
#else
	if (argc < 4) {
		usage();
	}

	const char* filterName = argv[1];
	const char* inputsPath = argv[2];
	const char* weightsPath = argv[3];
	const char* outputPath = argv[4];
#endif

	filter::BaseFilter* filter = NULL;
	float* inputs;
	uint64_t inputsLength;
	float* weights;
	uint64_t weightsLength;
	float* output;

	// Keep this up to date!
	if (filterName == "basictd") {
		filter = new filter::BasicTimeDomain();
	} else if (filterName == "opencltd") {
		filter = new filter::OpenCLTimeDomain();
	} else {
		printf("Filter implementation not found\n");
		exit(1);
	}

	//TODO choose parser based on .txt vs .dat/.bin
	inputs = mapInputFile(inputsPath, &inputsLength);
	weights = readTextFloats(weightsPath, &weightsLength);
	output = new float[inputsLength + weightsLength];

	if (filter) {
		START_TIMER;
		filter->doFilter(inputs, inputsLength, weights, weightsLength, output);
		STOP_TIMER;
	}

	//TODO write to output file instead
	for (int i = 0; i < 20; i++) {
		printf("%f %f\n", inputs[i], output[i]);
	}

	//TODO Testing
	//const char* expectedOutputPath;

	delete filter;
	delete[] output;
	return 0;
}

/* For when you have a small text file */
float* readTextFloats(const char* filename, uint64_t* length) {
	std::fstream fs(filename, std::ios::in);
	std::vector<float> floats;
	std::string line;

	if (!fs.is_open()) {
		printf("File not found: %s\n", filename);
		exit(1);
	}

	while (std::getline(fs, line)) {
		floats.push_back((float)atof(line.c_str()));
	}
	fs.close();
	*length = floats.size();
	return &floats[0];
}

/* For when you have gigabytes of data */
float* mapInputFile(const char* filename, uint64_t* length) {
	//TODO look into large pages? *nix port? FILE_FLAG_OVERLAPPED?
	wchar_t widePath[256];
	mbstowcs(widePath, filename, strlen(filename) + 1);

	HANDLE fileHandle = CreateFile((LPCWSTR)widePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	checkWinAPIError("File handle");

	HANDLE mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, (LPCWSTR)widePath);
	checkWinAPIError("Map handle");

	float* data = reinterpret_cast<float*>(MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, 0));
	checkWinAPIError("Map view");

	DWORD hiSize;
	DWORD loSize = GetFileSize(fileHandle, &hiSize);
	*length = (((uint64_t) hiSize) << (8 * sizeof(DWORD))) + loSize;
	*length /= sizeof(float);
	return data;
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n");
	exit(1);
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
