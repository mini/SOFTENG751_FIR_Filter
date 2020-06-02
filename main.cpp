#include "main.h"

int main(int argc, char** argv) {

#ifdef _DEBUG 
	printf("DEBUG MODE\n");
	std::string filterName = "basictd";
	//std::string filterName = "basictd";
	std::string inputsPath = "small.dat";
	std::string weightsPath = "weights.txt";
	std::string outputPath = "output.txt";
#else
	if (argc < 4) {
		usage();
	}

	std::string filterName(argv[1]);
	std::string inputsPath(argv[2]);
	std::string weightsPath(argv[3]);
	std::string outputPath(argv[4]);
#endif

	filter::BaseFilter* filter;

	// Keep this up to date!
	if (filterName == "basictd") {
		filter = new filter::BasicTimeDomain();
	} else if (filterName == "opencltd") {
		filter = new filter::OpenCLTimeDomain();
	} else {
		printf("Filter implementation not found\n");
		exit(1);
	}

	InputFile *inputs = new InputFile(inputsPath);
	InputFile *weights = new InputFile(weightsPath);
	float* output = new float[inputs->length + weights->length]{ 0 };

	START_TIMER;
	filter->doFilter(inputs->samples, inputs->length, weights->samples, weights->length, output);
	STOP_TIMER;

	//TODO write to output file instead
#ifdef _DEBUG
	for (int i = 0; i < std::min(20, (int)(inputs->length + weights->length)); i++) {
		printf("%f %f\n", i < inputs->length ? inputs->samples[i] : 0.0f, output[i]);
	}
#endif

	delete filter;
	delete inputs;
	delete weights;

	//TODO Testing
	//const char* expectedOutputPath;

	return 0;
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n");
	exit(1);
}