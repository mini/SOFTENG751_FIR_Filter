__kernel void filter_fd(const __global float* input, __constant float* weights, __global float* output) {
    const size_t inputLength = get_global_size(0);

    for (size_t i = 0; i < inputLength; i++) {
        output[i] = input[i] * weights[i];
    }
}