#pragma once

#include "board.h"

template<typename T>
concept bool EquityEstimator =
    requires(T v, board b)
    {
        { v.equity(b) } -> float;
    };
