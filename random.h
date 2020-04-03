/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <random>
#include <chrono>
#include <cstdint>

class RNG_die
{

public:
    RNG_die(unsigned seed = -1)
        : dice{1, 6},
          seed(0)
    {
        if (seed == -1)
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            seed = now.time_since_epoch().count();
        }
        generator.seed(seed);
    }

    int roll()
    {
        return dice(generator);
    }

private:
    std::uniform_int_distribution<> dice;
    unsigned seed;
    std::default_random_engine generator;
};

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
    std::normal_distribution<float> distribution;
    unsigned seed;
    std::default_random_engine generator;
};
