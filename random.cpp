/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */

#include "random.h"
#include "console.h"

/* M = M1 * M1.  It is important that
 * M be less than half of the biggest
 * number representable.  It is also important
 * that B end in [even]21.
 */

#define M ((unsigned long) 2147395600L)
#define M1 ((unsigned long) 46340L)
#define B ((unsigned long) 31415821L)


static unsigned long seed;

static unsigned long mult(unsigned long p, unsigned long q)
{
	unsigned long p1, p0, q1, q0;
	p1 = p/M1; p0 = p%M1;
  	q1 =q/M1; q0 = q%M1;
	return ( ((p0*q1 + p1*q0) % M1) * M1  + p0 * q0) % M;
}

unsigned long l_random()
{
	seed = (mult(seed,B) + 1) % M;
	return seed;
}


/* Randomly return a number between
 * zero and n-1, inclusive.
 */
// I'm not sure what this is doing.  And it might be assuming
// 16-bit ints.

int random(int n)
{
	seed = (mult(seed,B) + 1) % M;
	return (int) ( ((seed/M1) * n) / M1);
}


/* If you don't call this, the seed defaults to 0, which is ok.
 */

void set_seed(unsigned long s)
{
	seed = s;
}

unsigned long current_seed()
{
	return seed;
}


/* Randomly return a number
 * between 1 and 6, inclusive.
 */

int throw_die()
{
	return random(6) + 1;
}

int random_bit()
{
	return l_random() < (M / 2);
}

	// This returns a float in the range [0.0, 1.0).
float random_float()
{
	double total = 0.0;
	for (int i = 0; i < 23; i++){
		total = (random_bit() + total) / 2.0;
	}
	return (float) total;
}

//#include <stdio.h>
#include <iostream>
#include <sys/timeb.h>

void randomize_seed()
{
    struct timeb tb;
    ftime(&tb);
    console << "initializing random number generator to second count: " << (int) tb.time << "\n";
    set_seed(tb.time);
}


float random_float(float f)
{
	return f * (2.0 * (random_float() - 0.5));
}
