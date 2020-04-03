/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <random>
#include <chrono>
#include <cstdint>

int throw_die();
float random_float();

void set_seed(uint64_t s);
void randomize_seed();

void setupRNG(uint64_t user_seed);

class RNG_normal
{
public:
    RNG_normal(double mean = 0, double variance = 1)
        : distribution( mean, sqrt(variance) ),
          seed( std::chrono::system_clock::now().time_since_epoch().count() ),
          generator(seed)
    {
    }

    float random()
    {
        return distribution(generator);
    }

private:
    std::normal_distribution<float>  distribution;
    unsigned seed;
    std::default_random_engine generator;
};
