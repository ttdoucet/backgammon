/* Written by Todd Doucet. */
#pragma once
#include "netop.h"
#include "algebra.h"

template<int Features, int Hidden>
struct Params_Fc_Sig_Bias_base
{
    matrix<Hidden, Features> M;
    rowvec<Hidden> V;
    matrix<1,1> B;  // single bias on the output

    using self = Params_Fc_Sig_Bias_base<Features, Hidden>;
    static constexpr auto members = std::make_tuple(&self::M, &self::V, &self::B);
};

template<int Features, int Hidden>
using Params_Fc_Sig_Bias = algebra<Params_Fc_Sig_Bias_base<Features, Hidden>>;


  // Fully-connected two-layer network with sigmoidal activations and output bias.
template<int Features, int Hidden>
class Fc_Sig_Bias
{
public:
    using Parameters = Params_Fc_Sig_Bias<Features, Hidden>; 
    Parameters params, grad;

    struct Activations
    {
        using InputVector = vec<Features>;
        using HiddenVector = vec<Hidden>;
        using OutputVector = vec<1>;

        InputVector input;
        HiddenVector hidden;
        OutputVector pre_out;
        OutputVector pre_out2;
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
        auto r5  = Op_5.bwd(one);
        auto r4  = Op_4.bwd(r5);
        auto r3b = Op_3b.bwd(r4);
        auto r3  = Op_3.bwd(r3b);
        auto r2  = Op_2.bwd(r3);
        Op_1.bwd_param(r2);
        g = grad;
    }

    void update_model(Parameters const& adj)
    {
        params += adj;
    }

protected:
    Linear<Features, Hidden>      Op_1{act.input, act.hidden, params.M, grad.M};
    Termwise<logistic, Hidden>    Op_2{act.hidden, act.hidden};

    Linear<Hidden, 1>             Op_3{act.hidden, act.pre_out, params.V, grad.V};
    Bias<1, 1>                    Op_3b{act.pre_out, act.pre_out2, params.B, grad.B};
    Termwise<bipolar_sigmoid, 1>  Op_4{act.pre_out2, act.pre_out2};

// try this
// or better yet maybe affine<3.5,1>, but that will not work with
// a template, so do it another way.
    Termwise<affine<4,0>, 1>      Op_5{act.pre_out2, act.out};

    float feedForward()
    {
        Op_1.fwd();
        Op_2.fwd();
        Op_3.fwd();
        Op_3b.fwd();
        Op_4.fwd();
        Op_5.fwd();

        return float(act.out);
    }
};
