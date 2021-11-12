/* Written by Todd Doucet. */
#pragma once
#include "netop.h"
#include "algebra.h"

template<int Features, int Hidden>
class Fc_Sig_Bias
{
    struct Parameters_base
    {
        matrix<Hidden, Features> M;
        vec<Hidden> B;
        rowvec<Hidden> V;

        using self = Parameters_base;
        static constexpr auto members = std::make_tuple(&self::M, &self::B, &self::V);
    };

public:
    using Parameters = algebra<Parameters_base>;
    Parameters params, grad;

    struct Activations
    {
        using InputVector = vec<Features>;
        using HiddenVector = vec<Hidden>;
        using OutputVector = vec<1>;

        InputVector input;
        HiddenVector pre_hidden;
        HiddenVector hidden;
        OutputVector pre_out;
        OutputVector out;
    } act;

    using InputVector = typename Activations::InputVector;

    InputVector& input()
    {
        return act.input;
    }

    /* Computes dy/dw, where y is the scalar output of the net
     * from the last forward calculation, and w is any learnable
     * parameter.
     */
    void gradient(Parameters& g)
    {
        grad.clear();

        matrix<1,1> one = { 1 };
        auto r5 = Layer2c.bwd(one);
        auto r4 = Layer2b.bwd(r5);
        auto r3 = Layer2a.bwd(r4);

        auto r2 = Layer1c.bwd(r3);
        auto r1 = Layer1b.bwd(r2);
                  Layer1a.bwd_param(r1);
        g = grad;
    }

protected:
    Linear<Features, Hidden>         Layer1a{act.input, act.pre_hidden, params.M, grad.M};
    Bias<1, Hidden>                  Layer1b{act.pre_hidden, act.hidden, params.B, grad.B};
    Termwise<logistic, Hidden, 1>    Layer1c{act.hidden, act.hidden};

    Linear<Hidden, 1>                Layer2a{act.hidden, act.pre_out, params.V, grad.V};
    Termwise<bipolar_sigmoid, 1, 1>  Layer2b{act.pre_out, act.pre_out};
    Termwise<affine<3,0>, 1, 1>      Layer2c{act.pre_out, act.out};

    float feedForward()
    {
        Layer1a.fwd();
        Layer1b.fwd();
        Layer1c.fwd();

        Layer2a.fwd();
        Layer2b.fwd();
        Layer2c.fwd();

        return float(act.out);
    }
};
