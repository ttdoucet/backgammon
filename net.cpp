/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sstream>

#include "board.h"
#include "net.h"
#include "hits.h"
#include "random.h"

#include "console.h"



/*
 * Encode the board as the network input.
 * 
 * The input can be divided into three types:
 *	1. Functions of our checker configuration. (N_CHECK inputs)
 *	2. Functions of the opponent's checker configuration. (N_CHECK)
 *	3. Functions of the relationship of the two checker positions. 
 * 
 */

void net::compute_input_for(const color_t color, float *ib)
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

void net::compute_contact(const color_t color, float *ib)
{
	int me = netboard.highestChecker(color);
	int him = opponentPoint(netboard.highestChecker(opponentOf(color)));
	int contact = me - him;
	float f = contact < 0.0f ? 0.0f : (contact / 23.0f);

	*ib++ = f;
	*ib++ = (1.0f - f);
}

void net::compute_pip(const color_t color, float *ib)
{
	int pdiff = netboard.pipCount(opponentOf(color)) - netboard.pipCount(color);

	ib[0] = squash_sse( ((float) pdiff) / 27.0f);
	ib[1] = 1.0f - ib[0];
}

void net::compute_hit_danger(const color_t color, float *ib)
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

void net::compute_hit_danger_v3(const color_t color, float *ib)
{
	float h = ((float) num_hits(color, netboard)) / 36.0f;

	ib[0] = h;
	ib[1] = 1.0f - h;
}

void net::compute_v2_inputs(const color_t color, float *ib)
{
	compute_contact(color, ib);
	compute_pip(color, ib + net::METRICS_CONTACT);
	compute_hit_danger(color, ib + net::METRICS_CONTACT + net::METRICS_PIP);
	compute_hit_danger(opponentOf(color), ib + net::METRICS_CONTACT +
					   net::METRICS_PIP + net::METRICS_HIT);
}

int net::crossovers(color_t color)
{
	int m = 4 * netboard.checkersOnBar(color);
	for (int j = 1; j <= 3; j++)
		for (int i = 6*j + 1; i <= 6 * (j+1); i++)
			m += j * netboard.checkersOnPoint(color, i);
	return m;
}

void net::compute_crossovers(const color_t color, float *ib)
{
	float x_me = (float) crossovers(color);
	float x_him = (float) crossovers(opponentOf(color));

	ib[0] = x_me / 60.0f;	// is 60.0 too high?
	ib[1] = x_him / 60.0f;
	
		// A crossover lead of 5 corresponds to a value of
		// squash(1), which is about 75 percent.
	ib[2] = squash_sse( (x_me - x_him) / 5.0f);
	ib[3] = 1.0f - ib[2];
}



void net::compute_v4_inputs(const color_t color, float *ib)
{
	// The first two are the same as net_v2.
	compute_contact(color, ib);
	compute_pip(color, ib + net::METRICS_CONTACT);

	// Here we represent the hit danger and hit attack
	// differently from v2, but the same as v3.

	compute_hit_danger_v3(color, ib + net::METRICS_CONTACT + net::METRICS_PIP);
	compute_hit_danger_v3(opponentOf(color), ib + net::METRICS_CONTACT +
						  net::METRICS_PIP + net::METRICS_HIT_V3);

	// And we add crossover calculations.
	compute_crossovers(color, ib + net::METRICS_CONTACT + net::METRICS_PIP +
					   net::METRICS_HIT_V3 + net::METRICS_HIT_V3);
}

void net::compute_v3_inputs(const color_t color, float *ib)
{
	// The first two are the same as net_v2.
	compute_contact(color, ib);
	compute_pip(color, ib + net::METRICS_CONTACT);

	// Here we represent the hit danger and hit attack differently.

	compute_hit_danger_v3(color, ib + net::METRICS_CONTACT + net::METRICS_PIP);
	compute_hit_danger_v3(opponentOf(color), ib + net::METRICS_CONTACT +
						  net::METRICS_PIP + net::METRICS_HIT_V3);
}

void net::compute_input(const color_t color, float *inbuf)
{
	fatal( "compute_input() on illegal net.");
}

const char *net::checker_names_self[] = {
	"off",
	"1-blot", "1-point", "1-build",
	"2-blot", "2-point", "2-build",
	"3-blot", "3-point", "3-build",
	"4-blot", "4-point", "4-build",
	"5-blot", "5-point", "5-build",
	"6-blot", "6-point", "6-build",
	"7-blot", "7-point", "7-build",
	"8-blot", "8-point", "8-build",
	"9-blot", "9-point", "9-build",
	"10-blot", "10-point", "10-build",
	"11-blot", "11-point", "11-build",
	"12-blot", "12-point", "12-build",
	"13-blot", "13-point", "13-build",
	"14-blot", "14-point", "14-build",
	"15-blot", "15-point", "15-build",
	"16-blot", "16-point", "16-build",
	"17-blot", "17-point", "17-build",
	"18-blot", "18-point", "18-build",
	"19-blot", "19-point", "19-build",
	"20-blot", "20-point", "20-build",
	"21-blot", "21-point", "21-build",
	"22-blot", "22-point", "22-build",
	"23-blot", "23-point", "23-build",
	"24-blot", "24-point", "24-build",
	"bar"
};

const char *net::checker_names_other[] = {
	"opp-off",
	"opp-1-blot", "opp-1-point", "opp-1-build",
	"opp-2-blot", "opp-2-point", "opp-2-build",
	"opp-3-blot", "opp-3-point", "opp-3-build",
	"opp-4-blot", "opp-4-point", "opp-4-build",
	"opp-5-blot", "opp-5-point", "opp-5-build",
	"opp-6-blot", "opp-6-point", "opp-6-build",
	"opp-7-blot", "opp-7-point", "opp-7-build",
	"opp-8-blot", "opp-8-point", "opp-8-build",
	"opp-9-blot", "opp-9-point", "opp-9-build",
	"opp-10-blot", "opp-10-point", "opp-10-build",
	"opp-11-blot", "opp-11-point", "opp-11-build",
	"opp-12-blot", "opp-12-point", "opp-12-build",
	"opp-13-blot", "opp-13-point", "opp-13-build",
	"opp-14-blot", "opp-14-point", "opp-14-build",
	"opp-15-blot", "opp-15-point", "opp-15-build",
	"opp-16-blot", "opp-16-point", "opp-16-build",
	"opp-17-blot", "opp-17-point", "opp-17-build",
	"opp-18-blot", "opp-18-point", "opp-18-build",
	"opp-19-blot", "opp-19-point", "opp-19-build",
	"opp-20-blot", "opp-20-point", "opp-20-build",
	"opp-21-blot", "opp-21-point", "opp-21-build",
	"opp-22-blot", "opp-22-point", "opp-22-build",
	"opp-23-blot", "opp-23-point", "opp-23-build",
	"opp-24-blot", "opp-24-point", "opp-24-build",
	"opp-bar",
};

const char *net_v2::v2_names[] = {
	"Not-Broken", "Broken",
	"Pip-Ahead", "Pip-Behind",

	"Hit-Dang-0", "Hit-Dang-1", "Hit-Dang-2", "Hit-Dang-3", "Hit-Dang-4",
	"Hit-Dang-5", "Hit-Dang-6", "Hit-Dang-7", "Hit-Dang-8",

	"Hit-Attk-0", "Hit-Attk-1", "Hit-Attk-2", "Hit-Attk-3", "Hit-Attk-4",
	"Hit-Attk-5", "Hit-Attk-6", "Hit-Attk-7", "Hit-Attk-8",
};

const char *net_v3::v3_names[] = {
	"Not-Broken", "Broken",
	"Pip-Ahead", "Pip-Behind",

	"Hit-Dang", "!Hit-Dang",
	"Hit-Attk", "!Hit-Attk"
};

const char *net_v4::v4_names[] = {
	"Not-Broken", "Broken",
	"Pip-Ahead", "Pip-Behind",

	"Hit-Dang", "!Hit-Dang",
	"Hit-Attk", "!Hit-Attk",

	"X-me", "X-him",
	"X-Behind",
	"X-Ahead"
};

const char *net::input_name(int n)
{
	fatal("input_name() on illegal net.");
	return "";
}

net::net(int nhidden, int ninputs)
	: n_hidden(nhidden), n_inputs(ninputs)
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

net::net() : n_hidden(0), n_inputs(0), we_learn(0)
{
	games_trained = 0L;
}

net::~net()
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

float net::neuralFeed()
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

/* Apply the current backgammon board to the input,
 * and compute the net's output.
 */
float net::feedBoard(const board &b)
{
	netboard = b;
	compute_input(b.colorOnRoll(), inbuf);
	return neuralFeed();
}


/*
 * For each weight, compute the partial derivate of the output
 * with respect to that weight, and store it in the appropriate
 * entry of the network "g".  This assumes that feedForward()
 * or feed_board() has already been called with the current input.
 */

#if 1
void net::calcGradient(net *g)
{
	int i, j;
	float oprime, hprime;

	oprime = output * (1 - output) ;
	for (i = 0; i < n_hidden; i++){
		g->weights_2[i] = oprime * hidden[i];
	}

	for (i = 0; i < n_hidden; i++){
		hprime = hidden[i] * (1 - hidden[i]) ;
		float product = hprime * oprime * weights_2[i];
		float *p = g->weights_1[i];
		for (j = 0; j < n_inputs; j++)
			p[j] = product * input[j];
	}
}

#else
void net::calcGradient(net *g)
{
	int i, j;
	float oprime, hprime;

	oprime = output * (1 - output) ;
	for (i = 0; i < n_hidden; i++){
		g->weights_2[i] = oprime * hidden[i];
	}

// Why not eliminate the hprime * oprime calculations
// by computing hprime*oprime in the first place?
// Also, would it be advantageous to use float pointers
// with post-increment instead of subscripting by j?
// And I think we can compute n->weights_2[i] once for
// each i-loop (although the compiler might already
// be doing this.  Take a look.	

	for (i = 0; i < n_hidden; i++){
		hprime = hidden[i] * (1 - hidden[i]) ;
		for (j = 0; j < n_inputs; j++)
			g->weights_1[i][j] = hprime * oprime *
					     input[j] * weights_2[i]; 
	}
}
#endif

#define NO_MOVE_YET -99.0f

void net::init_play()
{
	for (int i = 0; i < n_inputs; i++)
		input[i] = 0.0f;
		// This makes future marginal calculations work.
	feedForward();
}

void net::init_learning(float a, float l)
{
	if (we_learn == 0)
		return;
	if (acc_grad == 0){
		acc_grad = new net(n_hidden, n_inputs);
		delta = new net(n_hidden, n_inputs);
		delta->clearNetwork();
		grad = new net(n_hidden, n_inputs);
	}
	acc_grad->clearNetwork();
	alpha = a;
	lambda = l;
	last_observation = NO_MOVE_YET;
	saw_final_signal = 0;
}

class fLearn {
	public:
		const float k;
		fLearn(float f) : k(f) {}
		void operator() (float &d, const float &a) const {
			d += (k * a);
		}
};

void net::learn(float pNew, float pOld)
{
	if (last_observation == NO_MOVE_YET)
		return;

	// for each weight in delta, w
	// delta.w += k * acc_grad.w
		/* times alpha, effectively */
	const float k = delta_equity_to_delta_net(pNew - pOld);	
	applyToNetworks(delta, acc_grad, fLearn(k));
}

void net::observe(const float p)
{
	if (!we_learn)
		return;

	learn(p, last_observation);
	last_observation = p;
	accumGradient();
}

void net::observe_final(const float p)
{
	if (saw_final_signal || !we_learn)
		return;
	saw_final_signal = 1;
	learn(p, last_observation);
}

class fAccum {
	public:
		const float lambda;
		fAccum(float L) : lambda(L) {}
		void operator() (float &ag, float &g) const {
			ag = (lambda * ag) + g;
		}
};

void net::accumGradient()
{
	calcGradient(grad);
		// for each weight in acc_grad, w
		// acc_grad.w = (lambda * acc_grad.w) + grad.w

	applyToNetworks(acc_grad, grad, fAccum(lambda));

}

// this is really the same as fLearn.
class fUpdate {
	public:
		const float alpha;
		fUpdate(float A) : alpha(A) {}
		void operator() (float &n, float &d) const {
			n += alpha * d;
		}
};

float net::incorporate_learning()
{
	applyToNetworks(this, this->delta, fUpdate(alpha));

	float r = feedForward();
	delta->clearNetwork();
	return r;
}

void set_f(float &f)
{
	f = random_float(0.50);
}

void net::randomizeNetwork()
{
    applyFunction(set_f);
}

class fSave {
public:
	FILE *netfp;
	int portable;

	fSave(FILE *fp, int p) : netfp(fp), portable(p){}

	void operator() (float &f) const {
		if (portable){
		  if (fprintf(netfp, "%.12f\n", (double) f) <= 0)
				fatal( "Error writing to network file.");
		} else {	// nonportable format saves all the bits in intel float format
			if (fwrite(&f, sizeof(f), 1, netfp) != 1)
				fatal("Error writing to network file.");
		}
	}
};

void net::dump_network(const char *fn, int portable)
{
		FILE *netfp;

		if ( (netfp = fopen(fn, "wb")) == NULL)
				fatal(std::string("Cannot open file ") + fn + " for writing.");

		fprintf(netfp, "portable format: %d\n", portable);
		fprintf(netfp, "net type: %d\n", n_type);
		fprintf(netfp, "hidden nodes: %d\n", n_hidden);
		fprintf(netfp, "input nodes: %d\n", n_inputs);

		applyFunction(fSave(netfp, portable));

		fprintf(netfp, "Current seed: %ldL\n", current_seed());
		fprintf(netfp, "Games trained: %ldL\n", games_trained);

		fclose(netfp);
}

void net::must_have(int ntype, int inputs, int mustval)
{
	if (inputs != mustval)
			fatal(std::string("net type ") + Str(ntype) + " must have " + Str(mustval) + " inputs, but has " + Str(inputs) + " instead");
}

void net::check_input_value(int ntype, int inputs)
{
	switch(ntype){
		case 1:
			must_have(ntype, inputs, net::inputsForV1);
			break;
		case 2:
			must_have(ntype, inputs, net::inputsForV2);
			break;
		case 3:
			must_have(ntype, inputs, net::inputsForV3);
			break;
		case 4:
			must_have(ntype, inputs, net::inputsForV4);
			break;
		default:
			fatal(std::string("invalid net type: ") + Str(ntype) );
	}
}

class fRead {
	public:
		FILE *netfp;
		int portable;
		fRead(FILE *fp, int p) : netfp(fp), portable(p){}
		void operator() (float &f) const {
			if (portable){
				if (fscanf(netfp, "%f", &f) == 0)
					fatal("Error reading network file.");
			} else {
				if (fread(&f, 1, sizeof(f), netfp) != sizeof(f))
					fatal("Error reading network file.");
			}
		}
};


/* Allocate and initialize a network based
 * on the information in file fn.
 */

net *net::read_network(const char *fn)
{
	using namespace std;
	unsigned long sd;
	int hidden = 40;

	FILE *netfp;
	cout << "Reading network file: " << fn << "\n";
	if ( (netfp = fopen(fn, "rb")) == NULL)
		fatal(std::string("Cannot open network file: ") + fn);

	int portable = 1;
	int ignore = fscanf(netfp, " portable format: %d\n", &portable);
	cout << "portable format: " << (portable ? "yes" : "no") << "\n";


	int ntype = 0;
	// If ntype remains zero, then we have a really old-style net file.
	ignore = fscanf(netfp, " net type: %d\n", &ntype);

	ignore = fscanf(netfp, " hidden nodes: %d\n", &hidden);
	if (hidden > net::max_hidden)
		fatal(std::string("Too many hidden nodes: ") + Str(hidden));

	int inputs = net::inputsForV2;

	char throwAway;
	ignore = fscanf(netfp, " input nodes: %d%c", &inputs, &throwAway);
	if (throwAway == '\r'){
		ignore = fscanf(netfp, "%c", &throwAway);
	}
	if (throwAway != '\n')
		fatal(std::string("Expected newline, got ") + Str(throwAway));

	cout << "inputs=" << inputs << endl;

	if (ntype == 0){	// We have an old-style net file.
		if (inputs == net::inputsForV2)
			ntype = 2;
		else
			ntype = 1;
	}
	
	// Do a consistency check.
	check_input_value(ntype, inputs);


	net *p;
	switch (ntype){
	case 1: p = new net_v1(hidden); break;
	case 2: p = new net_v2(hidden); break;
	case 3: p = new net_v3(hidden); break;
	case 4: p = new net_v4(hidden); break;
	default:
		fatal(std::string("Unknown net type: ") + Str(ntype));
	}
	p->filename = fn;
//      net::applyToNetwork(p, fRead(netfp, portable));
        p->applyFunction(fRead(netfp, portable));

	if (fscanf(netfp, " Current seed: %luL", &sd) != 1)
		fatal("Cannot read seed from network file.");

	p->seed = sd;

	if (fscanf(netfp, " Games trained: %ldL\n", &(p->games_trained)) != 1)
		p->games_trained = 0L;
	fclose(netfp);

	cout << "Finished." << endl;
	p->init_play();
	return p;
}

void net::learns(int l)
{
	we_learn = l;
}

int net::learns()
{
	return we_learn;
}


class clearFtn {
	public:
		void operator() (float &f) const {
		  f = 0.0f;
		}
};

void net::clearNetwork()
{
	applyFunction(clearFtn());
}
