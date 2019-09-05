#include <emmintrin.h>
#include <xmmintrin.h>
//#include <cmath>

#include "mathfuncs.h"

namespace detail
{
    const __m128 maxx = _mm_set1_ps(87);
    const __m128 minx = _mm_set1_ps(-87);
    const __m128 one = _mm_set_ss(1);
    const __m128 c = _mm_set_ss(-8388608/0.6931471806);
    const __m128 b = _mm_set_ss(1065353216);
}
