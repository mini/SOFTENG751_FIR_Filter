/*
	Generates random waveforms, can be used as a standalone program or a util function
	Basically picks N random points and smoothstep-lerps to it within M samples.

	File format:
		float[] : float array containing samples
*/
#include <stdlib.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <random>

#define DEFAULT_SAMPLES 10
#define AMPLITUDE 1.0f

void generate(const char* filename, uint64_t points, uint64_t samples, bool output);
void usage(void);

int main(int argc, char** argv) {
	if (argc < 3) {
		usage();
	}

	const char* filename = argv[1];
	const uint64_t points = atoll(argv[2]);
	const uint64_t samples = (argc < 4) ? DEFAULT_SAMPLES : atoll(argv[3]);
	const uint64_t fileSize = points * samples * sizeof(float);

	if (!points || !samples) {
		usage();
	}

	printf("Generating %I64dx%I64d -> %s\n", points, samples, filename);
	double size = fileSize;
	int idx = 0;
	const char* units[] = { "B", "kB", "MB", "GB!", "TB!!", "PB!!!" };
	while (size > 1024) {
		size /= 1024;
		idx++;
	}
	printf("File size: %.*f%s\n", idx == 0 ? 0 : 3, size, units[idx]);
	printf("Continue? Y/n ");
	char response;
	std::cin >> response;
	if (toupper(response) != 'Y') {
		exit(1);
	}

	generate(filename, points, samples, true);

	return 0;
}

void usage(void) {
	printf("Usage: generate <filename> <num samples> [samples per points (default 10)]\n");
	exit(1);
}

void generate(const char* filename, uint64_t points, uint64_t samples, bool output) {

	std::ofstream file (filename, std::ios::binary | std::ios::out);
	if (!file.is_open()) {
		printf("Can't write to destination file\n");
		exit(1);
	}

	std::uniform_real_distribution<float> uniform(-AMPLITUDE, AMPLITUDE);
	std::default_random_engine re;

	float prevPoint = 0.0f;
	const double outputStep = points / 200.0f; // steps of 0.5%
	double outputCheck = outputStep;
	float *sampleArr = new float[samples];
	for (uint64_t i = 0; i < points; i++) {
		float nextPoint = uniform(re);
		for (uint64_t j = 0; j < samples; j++) {
			float t = j / (float) samples;
			float smoothT = t * t * (3 - 2 * t);
			sampleArr[j] = prevPoint * (1.0f - smoothT) + nextPoint * smoothT;
		}
		file.write(reinterpret_cast<const char *>(sampleArr), sizeof(float) * samples);
		prevPoint = nextPoint;

		if(output && i >= outputCheck) {
			printf("    \r%I64d%%", i * 100 / points);
			outputCheck += outputStep;
		}
	}
	if(output) printf("   \rDone!");
	
	file.close();
}
