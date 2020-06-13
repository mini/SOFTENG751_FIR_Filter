__kernel void filter_fd(const __global float2 *input, const __global float2 *weights, __global float2 *output) {
    const size_t i = get_global_id(0);
    output[i].x = input[i].x * weights[i].x - input[i].y * weights[i].y;
    output[i].y = input[i].x * weights[i].y + input[i].y * weights[i].x;
}