/*
** +=======================================================================+
** | (c) 2025 Mario "Neo" Sieg. <mario.sieg.64@gmail.com>                  |
** +=======================================================================+
*/

#define NEUROBIT_IMPLEMENTATION
#include "neurobit.h"

#include <stdio.h>
#include <stdlib.h>

static void test_uint4_quant_roundtrip(void) {
    printf("Testing uint4... quantization\n");
    const float scale = 0.149460852f;
    const int32_t zp = 8;
    const float input[7] = {
        0.851733685f,
        -0.876200974f,
        -0.250579119f,
        0.802029967f,
        0.721749306f,
        -0.72338897f,
        -0.959288836f,
    };
    float dequantized_correct[7] = {
        1.7484988f,
        -1.77296615f,
        -0.549500823f,
        1.54933429f,
        1.46905351f,
        -1.47069323f,
        -1.85605395f,
    };
    for (int i=0; i < 7; ++i) {
        dequantized_correct[i] -= input[i]; /* Because I got the test data from piquant with reduction op add, accidentally to we subtract the input to get the original values */
    }
    const uint8_t correct_quantized[4] = {46, 214, 61, 2};

    uint8_t quantized[4] = {};
    neurobit_quant_float_to_uint4(input, quantized, sizeof(input)/sizeof(*input), scale, zp);
    for (int i=0; i < 4; ++i) {
        if (quantized[i] != correct_quantized[i]) {
            printf("Quantized bits at index %d are not correct\n", i);
            abort();
        }
    }

    float dequantized[7] = {};
    neurobit_dequant_uint4_to_float(quantized, dequantized, sizeof(dequantized)/sizeof(*dequantized), scale, zp);
    for (int i=0; i < 7; ++i) {
        if (fabsf(dequantized[i] - dequantized_correct[i]) > 1e-7) {
            printf("Dequantized floats at index %d are not correct\n", i);
            abort();
        }
    }

    printf("Test passed\n");
}

int main(void) {
    test_uint4_quant_roundtrip();
    return 0;
}
