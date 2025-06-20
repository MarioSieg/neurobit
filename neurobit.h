/*
** +=======================================================================+
** | (c) 2025 Mario "Neo" Sieg. <mario.sieg.64@gmail.com>                  |
** +=======================================================================+
**
** Single header library. In ONE C/C++ file (compilation unit) add:
**
**      #define NEUROBIT_IMPLEMENTATION
**      #include <neurobit.h>
**
*/

#ifndef NEUROBIT_H
#define NEUROBIT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Quantise a float32 array to packed unsigned-4-bit (uint4) values.
 * Two uint4 values are stored in each uint8 of out.
 * The caller must allocate (len + 1) / 2 bytes for out.
 */
extern void neurobit_quant_float_to_uint4(const float* in, uint8_t* out, size_t len, float scale, int32_t zero_point);

/**
 * De-quantise a packed uint4 array back to float32.
 * The input buffer must contain (len + 1) / 2 bytes.
 */
extern void neurobit_dequant_uint4_to_float(const uint8_t* in, float* out, size_t len, float scale, int32_t zero_point);

/**
 * Quantise a float32 array to packed unsigned-2-bit (uint2) values.
 * Two uint2 values are stored in each uint8 of out.
 * The caller must allocate (len + 3) / 4 bytes for out.
 */
extern void neurobit_quant_float_to_uint2(const float* in, uint8_t* out, size_t len, float scale, int32_t zero_point);

/**
 * De-quantise a packed uint2 array back to float32.
 * The input buffer must contain (len + 3) / 4 bytes.
 */
extern void neurobit_dequant_uint2_to_float(const uint8_t* in, float* out, size_t len, float scale, int32_t zero_point);

#ifdef __cplusplus
}
#endif

#ifdef NEUROBIT_IMPLEMENTATION

#include <math.h>
#include <assert.h>

static inline uint8_t sat_u4(int32_t x) { return x < 0 ? 0 : x > 15 ? 15 : x; }
static inline uint8_t sat_u2(int32_t x) { return x < 0 ? 0 : x > 3 ? 3 : x; }

void neurobit_quant_float_to_uint4(const float* in, uint8_t* out, size_t len, float scale, int32_t zero_point) {
    assert(in && out && scale > 0.f);
    float inv_scale = 1.0f / scale;
    size_t i;
    for (i=0; i+1 < len; i += 2) {
        uint8_t a = sat_u4(zero_point + (int32_t)roundf(in[i]*inv_scale));
        uint8_t b = sat_u4(zero_point + (int32_t)roundf(in[i+1]*inv_scale));
        out[i>>1] = a | b<<4;
    }
    if (len & 1) /* Odd tail */
        out[i>>1] = sat_u4(zero_point + (int32_t)roundf(in[len-1]*inv_scale));
}

void neurobit_dequant_uint4_to_float(const uint8_t* in, float* out, size_t len, float scale, int32_t zero_point) {
    assert(in && out && scale > 0.f);
    size_t i, j;
    for (i=0, j=0; i+1 < len; i += 2, ++j) {
        uint8_t p = in[j];
        uint8_t a = p & 15, b = p >> 4;
        out[i] = (float)((int32_t)a - zero_point)*scale;
        out[i+1] = (float)((int32_t)b - zero_point)*scale;
    }
    if (len & 1) /* Odd tail */
        out[len-1] = (float)((in[i>>1] & 15) - zero_point)*scale;
}

void neurobit_quant_float_to_uint2(const float* in, uint8_t* out, size_t len, float scale, int32_t zero_point) {
    assert(in && out && scale > 0.f);
    float inv_scale = 1.0f / scale;
    size_t i;
    for (i=0; i+3 < len; i += 4) {
        uint8_t a = sat_u2(zero_point + (int32_t)roundf(in[i]*inv_scale));
        uint8_t b = sat_u2(zero_point + (int32_t)roundf(in[i+1]*inv_scale));
        uint8_t c = sat_u2(zero_point + (int32_t)roundf(in[i+2]*inv_scale));
        uint8_t d = sat_u2(zero_point + (int32_t)roundf(in[i+3]*inv_scale));
        out[i>>2] = (uint8_t)(a | (b<<2) | (c<<4) | (d<<6));
    }
    if (len & 3) { /* Handle 1-, 2- or 3-value tail */
        uint8_t p = 0;
        switch (len & 3) {
            case 3: p |= sat_u2(zero_point + (int32_t)roundf(in[i+2]*inv_scale)) << 4;
            case 2: p |= sat_u2(zero_point + (int32_t)roundf(in[i+1]*inv_scale)) << 2;
            case 1: p |= sat_u2(zero_point + (int32_t)roundf(in[i]*inv_scale));
        }
        out[i>>2] = p;
    }
}

void neurobit_dequant_uint2_to_float(const uint8_t* in, float* out, size_t len, float scale, int32_t zero_point) {
    assert(in && out && scale > 0.f);
    size_t i, j;
    for (i=0, j=0; i+3 < len; i += 4, ++j) {
        uint8_t p = in[j];
        out[i] = (float)((p & 3) - zero_point)*scale;
        out[i+1] = (float)((p>>2 & 3) - zero_point)*scale;
        out[i+2] = (float)((p>>4 & 3) - zero_point)*scale;
        out[i+3] = (float)((p>>6 & 3) - zero_point)*scale;
    }
    if (len & 3) { /* Handle 1-, 2- or 3-value tail */
        uint8_t p = in[i>>2];
        if (len & 1) out[i] = (float)((p & 3) - zero_point)*scale;
        if (len & 2) out[i+1] = (float)((p>>2 & 3) - zero_point)*scale;
        if (len & 3) out[i+((len & 3) == 3 ? 2 : 0)] = (float)((p>>4 & 3) - zero_point)*scale;
    }
}

#endif

#endif
