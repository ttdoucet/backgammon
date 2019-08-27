/*
 * Written by Todd Doucet in 2019.
 */

#include <random>
#include <chrono>
using namespace std;

#include "random.h"

//using engine = mt19937;
using engine = default_random_engine;
using distribution = uniform_int_distribution<>;

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

void set_seed(unsigned long s)
{
    generator.seed(s);
}

// This returns a float in the range [0.0, 1.0).
float random_float()
{
    return std::generate_canonical<float, 20>(generator);
}
