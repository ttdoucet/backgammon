#include <iostream>
using namespace std;

// #include <cmath>

#include "matrix.h"

constexpr int N_HIDDEN = 32;
constexpr int N_INPUTS = 128;

typedef matrix<N_HIDDEN, N_INPUTS> W1;
typedef matrix<N_INPUTS, 1> input_vector;
typedef matrix<N_HIDDEN, 1> hidden_vector;

W1 M;
input_vector x;
hidden_vector h;


int main(int argc, char *argv[])
{
    int cnt = 10'000'000;

    for (int i = 0; i < cnt; i++)
        h =  M * x;

    cout << h.magnitude() << "\n";

}
