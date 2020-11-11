/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <cmath>
#include <algorithm>

struct logistic
{
    static float fwd(float x) { return  1 / (1 + exp(-x)); }
    static float bwd(float y) { return y * (1 - y); }
};
    
struct bipolar_sigmoid
{
    static float fwd(float x) { return 2 * logistic::fwd(x) - 1; }
    static float bwd(float y) { return 2 * logistic::bwd( (y+1) / 2 ); }
};
    
struct tanh_sigmoid
{
//  static float fwd(float x) { return 2 * logistic::fwd(2 * x) - 1; }
    static float fwd(float x) { return tanh(x); }
    static float bwd(float y) { return 1 - y * y; }
};

struct reLU
{
    static float fwd(float x) { return std::max(0.0f, x); }
    static float bwd(float y) { return (y < 0) ? 0 : 1; }
};

struct leaky_reLU
{
    static float fwd(float x) { return (x >= 0) ? x : (0.01 * x); }
    static float bwd(float y) { return (y < 0) ? 0.01 : 1; }
};

template<int scale, int offset>
struct affine
{
    static float fwd(float x) { return scale * x + offset; }
    static float bwd(float y) { return scale; }
};

