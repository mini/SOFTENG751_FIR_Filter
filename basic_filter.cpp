#include "base_filter.h"

namespace filter {
	class BasicTimeDomain;
}

class filter::BasicTimeDomain: public filter::BaseFilter {

	void doFilter(float* input, uint64_t inputLength, float* weights, uint64_t weightsLength, float* output) {
		for (uint64_t i = 0; i < inputLength + weightsLength; i++) {
			for (uint64_t j = 0; j < weightsLength; j++) {
				uint64_t index = i - j;
				if (index >= 0 && index < inputLength) {
					output[i] += input[index] * weights[j];
				}
			}
		}
	}
};