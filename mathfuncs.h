/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <cmath>
#include <algorithm>

/* Sigmoidal activation function, specifically the
 * logistic function, forward and backprop versions.
 */
inline float squash(float x)
{
    return  1 / (1 + exp(-x));
}

inline float squash_bp(float y)
{
    return y * (1 - y);
}

/* Logistic function shifted vertically to be
 * symmetric about zero, forward and backprop.
 */
inline float squash_ctr(float x)
{
    return 2 * squash(x) - 1;
}

inline float squash_ctr_bp(float y)
{
    return 2 * squash_bp(y);
}


/* The tanh activation function is naturally centered
 * about zero.
 */
inline float squash_tanh(float x)
{
    // return 2 * squash(2 * x) - 1;
    return tanh(x);
}

inline float squash_tanh_bp(float y)
{
    return 1 - y * y;
}

/* Rectified Linear Unit activation function,
 * forward and backprop.
 */
inline float relu(float x)
{
    return std::max(0.0f, x);
}

inline float relu_bp(float y)
{
    return (y < 0) ? 0 : 1;
}

/* Leaky rectified Linear Unit activation function,
 * forward and backprop.
 */
inline float leaky_relu(float x)
{
    if (x >= 0)
        return x;
    else
        return 0.01 * x;
}

inline float leaky_relu_bp(float y)
{
    return (y < 0) ? 0.01 : 1;
}

/* For when you want to try linear structures.
 */
inline float identity_act(float x)
{
    return x;
}

inline float identity_act_bp(float y)
{
    return 1;
}
