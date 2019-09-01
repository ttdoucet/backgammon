#pragma once

#include <chrono>
#include <iostream>

namespace detail_stopwatch {

using namespace std::chrono;

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

} // namespace detail_stopwatch

using detail_stopwatch::stopwatch;
