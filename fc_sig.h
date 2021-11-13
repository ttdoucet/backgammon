/* Written by Todd Doucet. */
#pragma once
#include "netop.h"
#include "algebra.h"

#include "stopwatch.h"

  /* Fully-connected two-layer network with sigmoidal activations.
   */
template<int Features, int Hidden>
class Fc_Sig
{
    struct Parameters_base
    {
        matrix<Hidden, Features> M;
        rowvec<Hidden> V;

        using self = Parameters_base;
        static constexpr auto members = std::make_tuple(&self::M, &self::V);
    };

public:
    using Parameters = algebra<Parameters_base>;
    Parameters params, grad;

    using InputVector  = vec<Features>;
    using HiddenVector = vec<Hidden>;
    using OutputVector = vec<1>;

    struct Activations
    {
        InputVector input;
        HiddenVector hidden;
        OutputVector pre_out;
        OutputVector out;
    } act;

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
        auto r5 = Op_5.bwd(one);
        auto r4 = Op_4.bwd(r5);
        auto r3 = Op_3.bwd(r4);
        auto r2 = Op_2.bwd(r3);
        Op_1.bwd_param(r2);
        g = grad;
    }

protected:
    Linear<Features, Hidden>                Op_1{act.input, act.hidden, params.M, grad.M};
    Termwise<logistic, HiddenVector>        Op_2{act.hidden, act.hidden};
    Linear<Hidden, 1>                       Op_3{act.hidden, act.pre_out, params.V, grad.V};
    Termwise<bipolar_sigmoid, OutputVector> Op_4{act.pre_out, act.pre_out};
    Termwise<affine<3,0>, OutputVector>     Op_5{act.pre_out, act.out};

    float feedForward()
    {
        Op_1.fwd();
        Op_2.fwd();        
        Op_3.fwd();
        Op_4.fwd();
        Op_5.fwd();

        return float(act.out);
    }
};
