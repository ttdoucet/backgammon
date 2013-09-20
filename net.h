#pragma once

#include <emmintrin.h>
#include <xmmintrin.h>

#include <math.h>

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

                virtual float feedBoard(const board &b);

                virtual const char *input_name(int n);

                net(int nhidden, int ninputs);
                net();
                virtual ~net();


	protected:
                float neuralFeed();
                virtual void compute_input(const color_t color, float *inbuf);

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

                template <class Ftn> inline static void applyToNetworks(const net *p1, const net *p2, Ftn f);
                template<class Ftn>  inline void applyFunction(Ftn f);


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
		float feedForward();
		void accumGradient();
		void calcGradient(net *g);
		void clearNetwork();

		void compute_input_for(const color_t color, float *ib);
		void compute_contact(const color_t color, float *ib);
		void compute_pip(const color_t color, float *ib);
		void compute_hit_danger(const color_t color, float *ib);
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
                    return (float) (1 / (1 + exp(-f)) );
                }

} ;



struct net_v1 : public net {
	net_v1(int nhidden);
	void compute_input(const color_t color, float *inbuf);
	const char *input_name(int n);
};

struct net_v2 : public net {
	static const char *v2_names[];
	net_v2(int nhidden);
	void compute_input(const color_t color, float *inbuf);
	const char *input_name(int n);
};

struct net_v3 : public net {
        static const char *v3_names[];
	net_v3(int nhidden);
	void compute_input(const color_t color, float *inbuf);
	const char *input_name(int n);
};

struct net_v4 : public net {
        static const char *v4_names[];
	net_v4(int nhidden);
	void compute_input(const color_t color, float *inbuf);
	const char *input_name(int n);
};


#ifndef MAX_EQUITY
#define MAX_EQUITY 3.0
#endif


#define net_to_equity(n) ((float)( (MAX_EQUITY * 2.0)*(n) - (MAX_EQUITY * 1.0) ))
#define delta_equity_to_delta_net(de) (float)( (de)/ (MAX_EQUITY * 2.0) )



