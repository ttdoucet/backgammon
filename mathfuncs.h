/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

namespace detail
{
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

    inline double floatify_d(const int64_t&& i)
    {
        return *(double *)&i;
    }

    inline double exp_fast_d(float y)
    {
        constexpr double ln2 = M_LN2;
        constexpr double a = (1LL << (20 + 32)) / ln2;
        constexpr int64_t k = ((1LL<<10) - 1) << (20 + 32);
        constexpr int64_t c = 60801LL << 32; // lowest rms relative error
        constexpr int64_t K = k - c;

        return floatify_d( a * y + K );
    }

    inline float squash(float x)
    {
        return 1 / (1 + expf(-x));
    }

    inline float squash_fast(float x)
    {
        return 1 / (1 + exp_fast(-x));
    }

    inline double squash_fast_d(float x)
    {
        return 1 / (1 + exp_fast_d(-x));
    }

}

using detail::squash;
using detail::squash_fast;
