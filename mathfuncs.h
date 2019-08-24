#pragma once

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
