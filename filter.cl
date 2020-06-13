__kernel void filter(const __global float* input, const __global float* weights, ulong inputLength, ulong weightsLength, __global float* output) {
    const size_t i = get_global_id(0);

    float value = 0.0f;
    for (size_t j = 0; j < weightsLength; j++) {
        const size_t index = i - j;
        if (index >= 0 && index < inputLength) {
             value += input[index] * weights[j];
        }
    }
    output[i] = value;
}