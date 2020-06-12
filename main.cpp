#include "main.h"

int main(int argc, char** argv) {

#ifdef _DEBUG 

	printf("DEBUG MODE\n");
	std::string filterName = "oclfft";
	std::string inputsPath = "data.dat";
	std::string weightsPath = "weights.dat";
	std::string outputPath = "output.txt";
	std::string expectedOutputPath = "expected_output.txt";

	/* Add another / to toggle comment block
	filterName = "ocl";
	expectedOutputPath = "5GB.basic.dat";
	outputPath = "5GB.ocl.dat";
	//*/
	
#else

	if (argc < 4) {
		usage();
	}

	std::string filterName(argv[1]);
	std::string inputsPath(argv[2]);
	std::string weightsPath(argv[3]);
	std::string outputPath(argc >= 5 ? argv[4] : "");
	std::string expectedOutputPath(argc >= 6 ? argv[5] : "");

#endif

	filter::BaseFilter* filter;

	// Keep this up to date!
	if (filterName == "btd") {
		filter = new filter::BasicTimeDomain();
	} else if (filterName == "ocltd") {
		filter = new filter::OpenCLTimeDomain();
	}
	else if (filterName == "oclctd") {
		filter = new filter::OpenCLChunkedTimeDomain();
	} else if (filterName == "oclfft") {
		filter = new filter::OpenCLFFT();
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

	if (outputPath.length()) {
		printf("Writing output to file\n");
		filter::writeOutputToFile(outputPath, output, outputLength);
	}

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