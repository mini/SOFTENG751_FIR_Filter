#include "main.h"

int main(int argc, char** argv) {

#ifdef _DEBUG 

	printf("DEBUG MODE\n");
	std::string filterName = "oclctd";
	std::string inputsPath = "5GB.dat";
	std::string weightsPath = "weights.txt";
	std::string outputPath = "";
	std::string expectedOutputPath = "";

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
	} else if (filterName == "oclctd") {
		filter = new filter::OpenCLChunkedTimeDomain();
	} else if (filterName == "oclfft") {
		filter = new filter::OpenCLFFT();
	} else {
		printf("Filter implementation not found\n");
		exit(1);
	}

	filter::InputFile *inputs = filter::InputFile::get(inputsPath);
	filter::InputFile *weights = filter::InputFile::get(weightsPath);
	filter::OutputFile *output = new filter::OutputFile(outputPath);

	START_TIMER;
	filter->doFilter(inputs, weights, output);
	STOP_TIMER;

	delete filter;
	delete inputs;
	delete weights;
	delete output;

	if (expectedOutputPath.length()) {
		printf("Comparing files: ");
		double percentMatch = filter::compareToFile(outputPath, expectedOutputPath);
		printf("%.3f%%\n", percentMatch);
	}

	benchMark(filterName, stop - start,inputsPath);
	return 0;
}

void usage(void) {
	printf("filter <algorithm> <input file> <weights file> <output file> [expected output file]\n Output and expected file formats must match\n");
	exit(1);
}

void benchMark(std::string filterName, uint64_t time, std::string inputsPath) {
	
#ifdef _DEBUG 	
	printf("Writing to CSV\n");
	std::ofstream myfile("BenchMark.csv", std::ios::app);

	myfile << filterName << ",";
	myfile << time << ",";
	myfile << inputsPath.substr(0, inputsPath.size() - 4)<<std::endl;
	myfile.close();
#endif

}

