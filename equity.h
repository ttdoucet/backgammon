#pragma once

#include <string>
#include "board.h"

template<typename T>
concept bool EquityEstimator =
    requires(T v, board b, std::string filename)
    {
        { v.equity(b) } -> float;

        // I think these will become automatically generated.
//        { readFile(v, filename) } -> bool;
//        { writeFile(v, filename) } -> bool;
    };

template<typename T>
concept bool TrainableEquityEstimator =
    requires(T v, board b, std::string filename)
    {
        requires EquityEstimator<T>;

        typename T::Parameters;
        {v.params *= 2.2} -> typename T::Parameters;
        {v.params *  2.2} -> typename T::Parameters;
        {v.params += v.params} -> typename T::Parameters;
    };

