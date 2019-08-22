#include <stdlib.h>
#include "bearoff.h"

int cmp_bearoff(const void *key, const void *elem)
{
	unsigned long left =  * ((unsigned long *) key);
	unsigned long right = ((struct bear_off *) elem) ->board;

	if (left > right)
		return 1;
	else if (left < right)
		return -1;
	else
		return 0;
}

const int bsize = 54264;
extern struct bear_off bear_off[bsize];

int board_to_index(unsigned long b)
{
	struct bear_off *bp = (struct bear_off *) bsearch(&b, bear_off, bsize,
							sizeof(struct bear_off),
							cmp_bearoff);

	if (bp == NULL){
//		fprintf(stderr, "error in find_board_expectation(), b=%lx\n", b);

		exit(1);
	}
	return bp - bear_off;
}



/* Return the probability that the bearoff position denoted
   by "b" will be cleared in n or fewer rolls.
*/
double fewerEq(unsigned long b, int n)
{
	if (n == 0){
		return (float) (b == 0x0f);
	} else {
		int i = board_to_index(b);
		return bear_off[i].f[n-1];
	}
		
}

/* Return the probability that the bearoff position denoted
   by "b" will be cleared in n or greater rolls.
*/
double greaterEq(unsigned long b, int n)
{
	if (n == 0.0){
		return 1.0;
	}
	return 1.0 - fewerEq(b, n - 1);
}

/* Return the probability that the bearoff position denoted
   by "b" will be cleared in exactly n rolls.
*/
double exact(unsigned long b, int n)
{
	if (n == 0){
		return b == 0x0f;
	}
	return fewerEq(b, n) - fewerEq(b, n - 1);
}


/*
 * This is the main routine exported by the bearoff dll.
 */
double bearoffEquity(unsigned long onRoll, unsigned long notOnRoll)
{
	double p = 0.0;
	for (int i = 0; i <= 15; i++){
		p += (exact(onRoll, i) * greaterEq(notOnRoll, i));
	}
	return (2.0 * p) - 1.0;
}
