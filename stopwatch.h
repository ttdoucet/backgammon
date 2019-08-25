#pragma once

#include <chrono>
#include <iostream>

namespace detail_stopwatch {

using namespace std::chrono;

#if 0
class stopwatch
{
public:
    void start()
    {
        started = high_resolution_clock::now();
    }

    void stop()
    {
        time_point stopped = std::chrono::high_resolution_clock::now();
        accum += (stopped - started).count();
        stops++;
    }

    void reset()
    {
        accum = 0;
    }

    long count()
    {
        return stops;
    }

    long elapsed()
    {
        return accum;
    }

private:
    long stops = 0;
    time_point<high_resolution_clock> started;
    long accum = 0;
};

#else

class stopwatch
{
public:
    void start() { }
    void stop()  { }
    void reset() { }
    long count() { return 0; }
    long elapsed() { return 0; }
};

#endif

} // namespace detail_stopwatch

using detail_stopwatch::stopwatch;
