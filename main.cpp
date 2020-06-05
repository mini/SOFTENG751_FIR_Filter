#include "main.h"

int main(int argc, char** argv) {

#ifdef _DEBUG 

	printf("DEBUG MODE\n");
	std::string filterName = "basictd";
	filterName = "opencltd";
	std::string inputsPath = "small.dat";
	std::string weightsPath = "weights.txt";
	std::string outputPath = "output.dat";
	std::string expectedOutputPath = "small.basic.dat";
	//expectedOutputPath = "";
	
#else

	if (argc < 5) {
		usage();
	}

	std::string filterName(argv[1]);
	std::string inputsPath(argv[2]);
	std::string weightsPath(argv[3]);
	std::string outputPath(argv[4]);
	std::string expectedOutputPath(argc >= 6 ? argv[5] : "");

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

	filter::InputFile *inputs = new filter::InputFile(inputsPath);
	filter::InputFile *weights = new filter::InputFile(weightsPath);
	uint64_t outputLength = inputs->length + weights->length;
	float* output = new float[outputLength]{ 0 };

	START_TIMER;
	filter->doFilter(inputs->samples, inputs->length, weights->samples, weights->length, output);
	STOP_TIMER;

#ifdef _DEBUG
	for (int i = 0; i < std::min(20, (int) outputLength); i++) {
		printf("%f %f\n", i < inputs->length ? inputs->samples[i] : 0.0, output[i]);
	}
#endif

	delete filter;
	delete inputs;
	delete weights;

	printf("Writing output to file\n");
	filter::writeOutputToFile(outputPath, output, outputLength);

	if (expectedOutputPath.length()) {
		printf("Comparing files: ");
		double percentMatch = filter::compareToFile(output, outputLength, expectedOutputPath);
		printf("%.3f%%\n", percentMatch);
	}

	return 0;
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n Output and expected file formats must match\n");
	exit(1);
}