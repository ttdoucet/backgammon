#pragma once

#include <emmintrin.h>
#include <xmmintrin.h>

#include <math.h>
#include "hits.h"

#define delta_equity_to_delta_net(de) (float)( (de)/ (MAX_EQUITY * 2.0f) )

#ifndef MAX_EQUITY
#define MAX_EQUITY 3.0f
#endif


static const __m128 maxx = _mm_set1_ps(87);
static const __m128 minx = _mm_set1_ps(-87);
static const __m128 one = _mm_set_ss(1);
static const __m128 c = _mm_set_ss(-8388608/0.6931471806);
static const __m128 b = _mm_set_ss(1065353216);

class net {
        public:
                static net *read_network(const char *fn);
                void dump_network(const char *fn, int portable = 0);
                void randomizeNetwork();

                void learns(int l);
                int learns();
                void init_play();
                void init_learning(float a = 0.25, float l = 0.70);
                float incorporate_learning();
                void observe(const float p);
                void observe_final(const float p);

		/* Apply the current backgammon board to the input,
		 * and compute the net's output.
		 */
		float feedBoard(const board &b)
		{
		  netboard = b;
		  compute_input(b.colorOnRoll(), inbuf);
		  return neuralFeed();
		}

                virtual const char *input_name(int n);

		net(int nhidden, int ninputs) : n_hidden(nhidden), n_inputs(ninputs)
		{
		  acc_grad = 0;
		  delta = 0;
		  grad = 0;
		  int i;

		  for (i = 0; i < n_hidden; i++)
		    weights_1[i] = new float[n_inputs];
		  inbuf = new float[n_inputs];
		  input = new float[n_inputs];

		  for (i = 0; i < n_inputs; i++)
		    input[i] = 0.0f;

		  hidden = new float[n_hidden];
		  pre_hidden = new float[n_hidden];
		  weights_2 = new float[n_hidden];

		  we_learn = 0;
		  games_trained = 0;
		}

		net() : n_hidden(0), n_inputs(0), we_learn(0)
		{
		  games_trained = 0L;
		}

		virtual ~net()
		{
		  if (n_hidden == 0)
		    return;

		  delete[ ] inbuf;
		  delete[ ] input;

		  delete[ ] hidden;
		  delete[ ] weights_2;

		  delete[ ] pre_hidden;

		  for (int i = 0; i < n_hidden; i++)
		    delete [ ] weights_1[i];
		}



	protected:
		
		static float net_to_equity(float n)
		{
		  return (MAX_EQUITY * 2.0f) * n - MAX_EQUITY;
		}

		float neuralFeed()
		{
		  int i, j;
		  float f, d;

		  // getting close to as fast as the specialized one if I play with the
		  // compiler switches.  -mtune=barcelona under gcc works well when
		  // using full calc.  Similar performance is obtained by using
		  // and sse-specfic dot product, for example the ones provided
		  // by gnu-radio.  But our optimized calc still does better.
		  //#define FULL_CALC
#ifdef FULL_CALC
		  for (i = 0; i < n_inputs ; i++)
		    input[i] = inbuf[i];
		  float r =  feedForward();
		  return r;
#else
		  for (i = 0; i < n_inputs ; i++){
		    if (input[i] == inbuf[i])
		      continue;

		    d = inbuf[i] - input[i] ;
		    input[i] = inbuf[i];

		    for (j = 0; j < n_hidden; j++)
		      pre_hidden[j] += d * weights_1[j][i];

		  }
		  for (j = 0; j < n_hidden; j++)
		    hidden[j] = squash_sse(pre_hidden[j]);

		  f = dot_product(hidden, weights_2, n_hidden);

		  output = squash_sse(f);
		  return net_to_equity(output);
#endif
		}


		virtual void compute_input(const color_t color, float *inbuf)
		{
		  fatal( "compute_input() on illegal net.");
		}



                static const char *checker_names_self[];
                static const char *checker_names_other[];

		enum {
			max_hidden = 80,
			METRICS_HIT  = 9,
			METRICS_CONTACT  = 2,
			METRICS_PIP  = 2,
			N_REL = (METRICS_CONTACT + METRICS_PIP + 2*METRICS_HIT ),
			METRICS_HIT_V3  = 2,
			METRICS_CROSS_V4 = 4,
				// blot, point, builders for each point, plus bar, borne-off.
			N_CHECK = (3*24 + 2),
			inputsForV1 = 2 * N_CHECK,
			inputsForV2 = inputsForV1 + METRICS_CONTACT + METRICS_PIP + 2 * METRICS_HIT,
			inputsForV3 = inputsForV1 + METRICS_CONTACT + METRICS_PIP + 2 * METRICS_HIT_V3,
			inputsForV4 = inputsForV3 + METRICS_CROSS_V4
		};


                static void must_have(int ntype, int inputs, int mustval);
                static void check_input_value(int ntype, int inputs);

		template <class Ftn> static void applyToNetworks(const net *p1, const net *p2, Ftn f)
		{
		  const int n_hidden = p1->n_hidden;
		  const int n_inputs = p1->n_inputs;
		  int i, j;
	
		  for (i = 0; i < n_hidden; i++){
		    float *d = p1->weights_1[i];
		    float *a = p2->weights_1[i];

		    for (j = 0; j < n_inputs; j++)
		      f( d[j], a[j] );
		  }

		  float *d = p1->weights_2;
		  float *a = p2->weights_2;
		  for (i = 0; i < n_hidden; i++)
		    f(d[i], a[i]);
		}

		/*
		 * This takes a routine or function object to apply to each weight
		 * of the network.  The routine or function object is passed a
		 * pointer to the weight.  Depending on what is passed in, we can
		 * randomly initialize the network, read the network from a file,
		 * clear the network, or write the network to a file.
		 */
		template<class Ftn> void applyFunction(Ftn f)
		{
		  int i, j;

		  for (i = 0; i < n_hidden; i++){
		    for (j = 0; j < n_inputs; j++)
		      f( weights_1[i][j] );
		  }
		  for (i = 0; i < n_hidden; i++)
		    f( weights_2[i] );
		}




		float *inbuf;			// n_inputs

		float *input;			// n_inputs
		float *hidden;			// n_hidden
		float *pre_hidden;		// n_hidden
		float *weights_1[max_hidden];	// max n_hidden
		float *weights_2;		// n_hidden

		float output;

		const int n_hidden;
		const int n_inputs;
		int n_type;

		unsigned long seed;
                const char *filename;

			// for learning
		int we_learn;

		net *acc_grad;
		net *delta;
		net *grad;
		float alpha;
		float lambda;


		long games_trained;

		// miscellaneous bookkeeping for learning.
		
		float last_observation;
		int saw_final_signal;


		board netboard;

	protected:
		void learn(float pNew, float pOld);
		void copy_to_transpose();



		static inline float squash_sse(const float x)
		{
#if 0
		  return (float) (1 / (1 + expf(-x)) );
#else
		  const __m128 y = _mm_max_ss(minx, _mm_min_ss(maxx, _mm_set_ss(x))); // clamp to [-87,87]
		  const __m128 z = _mm_add_ss(_mm_mul_ss(y, c), b);
		  const __m128i i = _mm_cvtps_epi32(z);
		  const float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_add_ss(_mm_load_ps((const float *)&i), one)));
		  // assert(std::abs(1/(1+std::exp(-x)) - r) < 1.48e-2);  // minimum accuracy on floats is 1.48e-2
		  return r;
#endif
		}


		/*
		 * Have the network evaluate its input.
		 */
		float feedForward()
		{
		  for (int i = 0; i < n_hidden; i++)
		    pre_hidden[i] = dot_product(input, weights_1[i], n_inputs);

		  for (int i = 0; i < n_hidden; i++)
		    hidden[i] = squash_sse(pre_hidden[i]);

		  output = squash_sse(dot_product(hidden, weights_2, n_hidden));
		  return net_to_equity(output);
		}


		void accumGradient();
		void calcGradient(net *g);
		void clearNetwork();

		/*
		 * Encode the board as the network input.
		 * 
		 * The input can be divided into three types:
		 *	1. Functions of our checker configuration. (N_CHECK inputs)
		 *	2. Functions of the opponent's checker configuration. (N_CHECK)
		 *	3. Functions of the relationship of the two checker positions. 
		 * 
		 */
		void compute_input_for(const color_t color, float *ib)
		{
		  int i, n;

		  *ib++ = (float) netboard.checkersOnPoint(color, 0);		/* borne off */

		  for (i = 1; i <= 24; i++){
		    n = netboard.checkersOnPoint(color, i);
		    *ib++ = (float) (n == 1);			/* blot     */
		    *ib++ = (float) (n >= 2);			/* point    */
		    *ib++ =	(float) ( (n > 2) ? (n - 2) : 0 );	/* builders */
		  }
		  *ib = (float) netboard.checkersOnBar(color);			/* on bar   */
		}

		void compute_contact(const color_t color, float *ib)
		{
		  int me = netboard.highestChecker(color);
		  int him = opponentPoint(netboard.highestChecker(opponentOf(color)));
		  int contact = me - him;
		  float f = contact < 0.0f ? 0.0f : (contact / 23.0f);

		  *ib++ = f;
		  *ib++ = (1.0f - f);
		}

		void compute_pip(const color_t color, float *ib)
		{
		  int pdiff = netboard.pipCount(opponentOf(color)) - netboard.pipCount(color);

		  ib[0] = squash_sse( ((float) pdiff) / 27.0f);
		  ib[1] = 1.0f - ib[0];
		}

		void compute_hit_danger(const color_t color, float *ib)
		{
		  int i, h = num_hits(color, netboard);
		  int whole = (h * net::METRICS_HIT) / 36;
		  int partial = h % (36 / net::METRICS_HIT);

		  for (i = 0; i < whole; i++){
		    *ib++ = 1.0f;
		  }
		  if (partial){
		    *ib++ = partial * (net::METRICS_HIT / 36.0f);
		    i++;
		  }
		  for (; i < net::METRICS_HIT; i++)
		    *ib++ = 0;
		}


		void compute_hit_danger_v3(const color_t color, float *ib);
		void compute_v2_inputs(const color_t color, float *ib);
		int crossovers(color_t color);
		void compute_crossovers(const color_t color, float *ib);
		void compute_v4_inputs(const color_t color, float *ib);
		void compute_v3_inputs(const color_t color, float *ib);
		

                inline static float dot_product(float *vec1, float *vec2, int size)
                {
                    float r = 0.0;
                    while (size--)
                        r += (*vec1++ * *vec2++);
                    return r;
                }

                inline static float squash(const float f)
                {
                    return (float) (1 / (1 + expf(-f)) );
                }

} ;



struct net_v1 : public net {
	net_v1(int nhidden)  : net(nhidden, inputsForV1)
	{
	  console << "allocating net_v1 with " << nhidden << " hidden nodes.\n";
	  n_type = 1;
	}

	void compute_input(const color_t color, float *inbuf)
	{
	  compute_input_for(color, inbuf);
	  compute_input_for(opponentOf(color), inbuf + net::N_CHECK);
	}

	const char *input_name(int n) const
	{
	  if (n < N_CHECK)
	    return checker_names_self[n];
	  n -= N_CHECK;
	  return checker_names_other[n];
	}
};

struct net_v2 : public net {
	static const char *v2_names[];

	net_v2(int nhidden)  : net(nhidden, inputsForV2)
	{
	  console << "allocating net_v2 with " << nhidden << " hidden nodes.\n";
	  n_type = 2;
	}

	const char *input_name(int n) const
	{
	  if (n < N_CHECK)
	    return checker_names_self[n];
	  n -= N_CHECK;
	  if (n < N_CHECK)
	    return checker_names_other[n];
	  n -= N_CHECK;
	  return v2_names[n];
	}

	void compute_input(const color_t color, float *inbuf)
	{
	  compute_input_for(color, inbuf);
	  compute_input_for(opponentOf(color), inbuf + net::N_CHECK);
	  compute_v2_inputs(color, inbuf + (2 * net::N_CHECK) );
	}


};

struct net_v3 : public net {
        static const char *v3_names[];

	net_v3(int nhidden) : net(nhidden, inputsForV3)
        {
	  console << "net_v3(hidden=" << nhidden << ")\n";
	  n_type = 3;
	}

	const char *input_name(int n) const
	{
	  if (n < N_CHECK)
	    return checker_names_self[n];
	  n -= N_CHECK;
	  if (n < N_CHECK)
	    return checker_names_other[n];
	  n -= N_CHECK;
	  return v3_names[n];
	}



	void compute_input(const color_t color, float *inbuf)
	{
	  compute_input_for(color, inbuf);
	  compute_input_for(opponentOf(color), inbuf + net::N_CHECK);
	  compute_v3_inputs(color, inbuf + (2 * net::N_CHECK) );
	}

};

struct net_v4 : public net {
        static const char *v4_names[];

	net_v4(int nhidden)  : net(nhidden, inputsForV4)
	{
	  console << "net_v4(hidden=" << nhidden << ")\n";
	  n_type = 4;
	}


	void compute_input(const color_t color, float *inbuf)
	{
	  compute_input_for(color, inbuf);
	  compute_input_for(opponentOf(color), inbuf + net::N_CHECK);
	  compute_v4_inputs(color, inbuf + (2 * net::N_CHECK) );
	}


	const char *input_name(int n) const
	{
	  if (n < N_CHECK)
	    return checker_names_self[n];
	  n -= N_CHECK;
	  if (n < N_CHECK)
	    return checker_names_other[n];
	  n -= N_CHECK;
	  return v4_names[n];
	}



};







