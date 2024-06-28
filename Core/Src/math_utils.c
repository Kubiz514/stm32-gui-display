#include "math_utils.h"

int32_t map_ranges(int32_t input, int32_t input_min, int32_t input_max, int32_t output_min, int32_t output_max) {
    return (input - input_min) * (output_max - output_min) / (input_max - input_min) + output_min;
}
