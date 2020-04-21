/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <cmath>

inline float squash(float x)
{
    return 1 / (1 + expf(-x));
}

