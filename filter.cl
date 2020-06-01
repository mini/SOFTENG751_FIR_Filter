__kernel void filter(const __global float* input, __constant float* weights, ulong weightsLength, __global float* output) {
    const size_t i = get_global_id(0);
    const size_t inputLength = get_global_size(0);

    for (size_t j = 0; j < weightsLength; j++) {
        const size_t index = i - j;
        if (i >= j && index < inputLength) {
            output[i] += input[index] * weights[j];
        }
    }
}