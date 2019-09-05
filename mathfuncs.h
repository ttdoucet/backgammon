#pragma once
#include <emmintrin.h>
#include <xmmintrin.h>
#include <cmath>

template<int N, typename T1, typename T2>
inline float dotprod(T1 vec1, T2 vec2)
{
    float sum = 0;
    for (int i = 0; i < N; ++i)
        sum += (vec1[i] * vec2[i]);

    return sum;
}

namespace detail
{
    extern const __m128 maxx;
    extern const __m128 minx;
    extern const __m128 one;
    extern const __m128 c;
    extern const __m128 b;

    inline float squash_sse(const float x)
    {
        const __m128 y = _mm_max_ss(minx, _mm_min_ss(maxx, _mm_set_ss(x))); // clamp to [-87,87]
        const __m128 z = _mm_add_ss(_mm_mul_ss(y, c), b);
        const __m128i i = _mm_cvtps_epi32(z);
        const float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_add_ss(_mm_load_ps((const float *)&i), one)));

        // assert(std::abs(1/(1+expf(-x)) - r) < 1.48e-2);  // minimum accuracy on floats is 1.48e-2
        return r;
    }

    /* These fast approximations of exp() are based on Nicol N. Schraudolph,
     * "A fast, compact approximation of the exponential function",
     *  Neural Computation 11, 853-862 (1999).  
     */

    inline float floatify(const int&& i)
    {
        return *(float *)&i;
    }

    inline float exp_fast(float y)
    {
        constexpr float ln2 = M_LN2;
        constexpr float a = (1 << 23) / ln2;
        constexpr int k = ((1<<7) - 1) << 23;
        constexpr int c = 486412; // lowest rms relative error
        constexpr int K = k - c;

        return floatify( a * y + K );
    }

    inline double floatify_d(const long&& i)
    {
        return *(double *)&i;
    }

    inline double exp_fast_d(float y)
    {
        constexpr double ln2 = M_LN2;
        constexpr double a = (1L << (20 + 32)) / ln2;
        constexpr long k = ((1L<<10) - 1) << (20 + 32);
        constexpr long c = 60801L << 32; // lowest rms relative error
        constexpr long K = k - c;

        return floatify_d( a * y + K );
    }

    inline float squash(float x)
    {
        return 1 / (1 + exp_fast(-x));
    }


    inline float squash_full(float x)
    {
        return 1 / (1 + expf(-x));
    }

    inline double squash_fast_d(float x)
    {
        return 1 / (1 + exp_fast_d(-x));
    }

}

using detail::squash;
using detail::squash_full;
