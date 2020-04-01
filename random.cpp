/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <random>
#include <chrono>
#include <iostream>

#include "random.h"

//using engine = std::mt19937;
using engine = std::default_random_engine;
using distribution = std::uniform_int_distribution<>;

static engine generator{};
static distribution dice{1,6};

void randomize_seed()
{
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto ts = now.time_since_epoch().count();
    generator.seed(ts);
}

/* Randomly return a number
 * between 1 and 6, inclusive.
 */
int throw_die()
{
    return dice(generator);
}

void set_seed(uint64_t s)
{
    generator.seed(s);
}

// This returns a float in the range [0.0, 1.0).
float random_float()
{
    return std::generate_canonical<float, 20>(generator);
}

void setupRNG(uint64_t user_seed)
{
    if (user_seed != -1)
    {
        std::cout << "RNG using user-specified seed: " << user_seed << "\n";
        set_seed(user_seed);
    }
    else
        randomize_seed();
}
