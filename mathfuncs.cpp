#include <emmintrin.h>
#include <xmmintrin.h>

#include "mathfuncs.h"

static const __m128 maxx = _mm_set1_ps(87);
static const __m128 minx = _mm_set1_ps(-87);
static const __m128 one = _mm_set_ss(1);
static const __m128 c = _mm_set_ss(-8388608/0.6931471806);
static const __m128 b = _mm_set_ss(1065353216);

float squash_sse(const float x)
{
    const __m128 y = _mm_max_ss(minx, _mm_min_ss(maxx, _mm_set_ss(x))); // clamp to [-87,87]
    const __m128 z = _mm_add_ss(_mm_mul_ss(y, c), b);
    const __m128i i = _mm_cvtps_epi32(z);
    const float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_add_ss(_mm_load_ps((const float *)&i), one)));

    // assert(std::abs(1/(1+expf(-x)) - r) < 1.48e-2);  // minimum accuracy on floats is 1.48e-2
    return r;
}

template<int N>
inline float dotprod(float *vec1, float *vec2)
{
    float r = 0.0f;
 
    float sum = 0.0f;
    for (int i = 0; i < N; ++i)
        sum += (vec1[i] * vec2[i]);

    return sum;
}

