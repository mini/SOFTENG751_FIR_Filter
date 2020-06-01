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
	}

	//TODO choose parser based on .txt or other (.dat/.bin)
	inputs = mapInputFile(inputsPath, &inputsLength);
	weights = readTextFloats(weightsPath, &weightsLength);
	output = new float[inputsLength + weightsLength];


	if (filter) {
		START_TIMER;
		filter->doFilter(inputs, inputsLength, weights, weightsLength, output);
		STOP_TIMER;
	}

	for (int i = 0; i < 20; i++) { // Temp
		printf("%f %f\n", inputs[i], output[i]);
	}

	//TODO write to output file

	//TODO Testing
	//const char* expectedOutputPath;

	delete filter;
	delete[] output;
}

float* readTextFloats(const char* filename, uint64_t* length) {
	std::fstream fs(filename, std::ios::in);
	std::vector<float> floats;
	std::string line;

	while (std::getline(fs, line)) {
		floats.push_back((float)atof(line.c_str()));
	}
	fs.close();

	*length = floats.size();
	return &floats[0];
}

float* mapInputFile(const char* filename, uint64_t* length) { //TODO look into large pages? *nix port
	HANDLE fileHandle = CreateFile((LPCWSTR)filename, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		printf("File not found\n");
		exit(1);
	}
	HANDLE mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, (LPCWSTR)filename);
	if (mapHandle == INVALID_HANDLE_VALUE) {
		printf("File map error\n");
		exit(1);
	}
	DWORD offsetHigh;
	DWORD offsetLow = GetFileSize(mapHandle, &offsetHigh);
	InputFile* data = (InputFile *) MapViewOfFile(mapHandle, FILE_MAP_READ, offsetHigh, offsetLow, 0);
	*length = data->totalSamples;
	return data->samples;
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n");
	exit(1);
}
