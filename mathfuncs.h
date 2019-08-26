#pragma once
#include <emmintrin.h>
#include <xmmintrin.h>

float squash_sse(const float x);

template<int N>
inline float dotprod(float *vec1, float *vec2)
{
    float r = 0.0f;

    float sum = 0.0f;
    for (int i = 0; i < N; ++i)
        sum += (vec1[i] * vec2[i]);

    return sum;
}

namespace _squash_sse
{
    extern const __m128 maxx;
    extern const __m128 minx;
    extern const __m128 one;
    extern const __m128 c;
    extern const __m128 b;
}

inline float squash_sse(const float x)
{
//  return 1/(1+expf(-x));

    using namespace _squash_sse;

    const __m128 y = _mm_max_ss(minx, _mm_min_ss(maxx, _mm_set_ss(x))); // clamp to [-87,87]
    const __m128 z = _mm_add_ss(_mm_mul_ss(y, c), b);
    const __m128i i = _mm_cvtps_epi32(z);
    const float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_add_ss(_mm_load_ps((const float *)&i), one)));

    // assert(std::abs(1/(1+expf(-x)) - r) < 1.48e-2);  // minimum accuracy on floats is 1.48e-2
    return r;
}
