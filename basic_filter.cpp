#include "base_filter.h"

namespace filter {
	class BasicTimeDomain;
}

class filter::BasicTimeDomain: public filter::BaseFilter {

	void doFilter(float* inputs, uint64_t length, float* weights, uint64_t weightsLength, float* output) {
		for (uint64_t i = 0; i < length + weightsLength; i++) {
			for (uint64_t j = 0; j < weightsLength; j++) {
				uint64_t index = i - j;
				if (index >= 0 && index < length) {
					output[i] += inputs[index] * weights[j];
				}
			}
		}
	}
};