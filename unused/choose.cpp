/*
	This is the backgammon-aware interface to the neural network.

    Actually, this is pretty lame.  This is screaming out to be an object.

    This file is no longer used.  Some of the code in here might be
    salvaged as part of some particular Player.

*/

#include <stdlib.h>
#include <memory.h>

#include "board.h"
#include "hits.h"
#include "net.h"
#include "choose.h"


/*
 *  select_move() section.  This is not currently used but should not
 *  yet be discarded.  It seems to be an alternative version of the kind
 *  of thing currently done in choose.cpp.  In a state of flux.
 */

// Get rid of all this imprecise talk about "ply".
// This builds in littleE() and bearoffEquity().

float select_move(board &b, int ply, move &m);
float look_ahead(board &b, int ply);


const int cand_limit = 4;

float select_move(board &b, int ply, move &m)
{
	mEntry cand[cand_limit];
	int j = getTopCandidates(b, cand,
				 isBearingOff(b) ? bearoffEquity : littleE,
				 cand_limit
                );

	if (ply == 0){
		m = cand[0].m;
		return cand[0].eqZeroPly;
	} else {
		double eq, best_eq = -10.0;
		move best_move;
		for (int i = 0; i < j; i++){
			move &mv = cand[i].m;
			b.applyMove(mv);
			eq = -look_ahead(b, ply - 1);
			b.reverseMove(mv);
			if (eq > best_eq){
				best_eq = eq;
				best_move = mv;
			}
		}
		m = best_move;
		return best_eq;
	}
}

float look_ahead(board &b, int ply)
{
	move m;
	tAssert(b.d1() == -1);
	tAssert(b.colorOnRoll() != nobody);

	double eq = 0.0;
	for (int i = 1; i <= 6; i++){
		for (int j = 1; j <= i; j++){
			b.setDice(i, j);
			double f = select_move(b, ply, m);
			if (i == j)
				eq += f;
			else
				eq += (2 * f);
		}

	}
	b.setDice(-1, -1);
	eq /= 36.0;
	return eq;
}





/*
 * End of select_move() section.
 */



// Really dopey.  Possibly motivated by performance during training.
static net *neural;

void setNetwork(net *n)
{
	tAssert(n != 0);
	neural = n;
}

/******************************************/

static int gen_comp(const void *l, const void *r, float mEntry::*equity)
{
	const float  left = ((mEntry *) l)->*equity;
	const float right = ((mEntry *) r)->*equity;

	if (left > right)
		return -1;
	else if (left < right)
		return 1;
	else
		return 0;
}

	// Sort comparison function for sort by 1-ply equity.
static int cmp_entry_1(const void *l, const void *r)
{
	return gen_comp(l, r, &mEntry::eqZeroPly);
}

	// Sort comparison function for sort by 2-ply equity.
static int cmp_entry_2(const void *l, const void *r)
{
	return gen_comp(l, r, &mEntry::eqOnePly);
}


void candidateMoves::sortZeroPly() {
	  qsort(mEnt, nitems, sizeof(mEntry), cmp_entry_1);
	  for (int i = 0; i < nitems; i++){
		  getEntry(i)->zeroPlyRank = i + 1;
	  }
}

void candidateMoves::sortOnePly() {
	  qsort(mEnt, nitems, sizeof(mEntry), cmp_entry_2);
	  for (int i = 0; i < nitems; i++){
		  getEntry(i)->onePlyRank = i + 1;
	  }
}


int getTopCandidates(board &b, mEntry *m, equityEstimator e, int limit)
{
	static candidateMoves legal;
	legal.setEvaluator(e);
	b.play(legal);
	legal.sortZeroPly();
	int nmoves = legal.size();
	int s = (nmoves > limit) ? limit : nmoves; 
	for(int j = 0; j < s; j++){
		m[j] = *(legal.getEntry(j));
	}
	return s;
}




/**********************************************/

class selectM : public callBack {
		float bestEquity;
		move bestMove;
		equityEstimator *equityEstimate;
	public:
		int callBackF(board &b){
			float e = -equityEstimate(b);
			if (e > bestEquity){
				bestEquity = e;
				bestMove = m;	// the current move.
			}
			return 0;
		}
		float equity(){
			return bestEquity;
		}
		move &moveChoice(){
			return bestMove;
		}
		selectM(board &b, equityEstimator *eqE){
			bestEquity = -10.0f;
			equityEstimate = eqE;
		}
};


	// This selects the best move for the side on roll, given
	// the equity estimator, which is used to evaluate the legal
	// positions.
float selectMove(board &fromBoard, move &m, equityEstimator equity)
{
#ifdef CHECK_BOARD
	board save = fromBoard;
#endif
	selectM select(fromBoard, equity);
	fromBoard.play(select);
	m = select.moveChoice();

#ifdef CHECK_BOARD
	tAssert(memcmp(&save, &fromBoard, sizeof(save)) == 0);
#endif
	return select.equity();
}


// maybe this should be a private class?
class lookAheadItem {
	public:
		int r1, r2;
		float eq;
		lookAheadItem() : r1(0), r2(0), eq(0.0f){
		}

};


class cLookAhead {
	public:
		lookAheadItem lookItem[21];
		cLookAhead() {
			int i, j, k;
			for (k = 0, i = 1; i <= 6; i++){
				for (j = 1; j <= i; j++){
					lookAheadItem &lh = lookItem[k++];
					lh.r1 = i;
					lh.r2 = j;
				}
			}
		}
};



float lookAhead(board &board, equityEstimator equity)
{
	int i;

	tAssert(board.d1() == -1);
	tAssert(board.colorOnRoll() != nobody);
	float directEquity = equity(board);

	cLookAhead lh;
	float total = 0.0f;
	move m;

	for (i = 0; i < 21; i++){
		float thisEquity;
			// Set up each possible dice roll.
		int &r1 = lh.lookItem[i].r1;
		int &r2 = lh.lookItem[i].r2;
		
		board.setDice(r1, r2);
			// We play the dice.
		thisEquity = selectMove(board, m, equity);
			// Weight the result appropriately.
		if (r1 == r2){
			total += thisEquity;
		} else {
			total += 2 * thisEquity;
		}
	}

	/*
		Presently, we do not take into account the
		directEquity, but maybe we should.
	*/

	board.setDice(-1, -1);
	return total / 36.0f;
}


int isBearingOff(board &nb)
{
	return	nb.checkersOnPoint(white, 0) &&
			nb.checkersOnPoint(black, 0) &&
			(nb.highestChecker(white) <= 6) &&
		    (nb.highestChecker(black) <= 6);
}

// This is duplicated exactly in beargen.cpp, which is part of
// a separate progam that generates the bearoff database.  Might
// it be reasonable to make this a method of the board class?


	// Encode the bearoff board as a 32-bit number (currently,
	// we use 24 bits, although 4 are redundant.
unsigned long board_to_32(board &b, color_t c)
{
	unsigned long d6 = b.checkersOnPoint(c, 6) << (4 * 6);
	unsigned long d5 = b.checkersOnPoint(c, 5) << (4 * 5);
	unsigned long d4 = b.checkersOnPoint(c, 4) << (4 * 4);
	unsigned long d3 = b.checkersOnPoint(c, 3) << (4 * 3);
	unsigned long d2 = b.checkersOnPoint(c, 2) << (4 * 2);
	unsigned long d1 = b.checkersOnPoint(c, 1) << (4 * 1);
	unsigned long d0 = b.checkersOnPoint(c, 0);
	return d0 | d1 | d2 | d3 | d4 | d5 | d6;
}



	// in beardll.dll
double  bearoffEquity(unsigned long, unsigned long);



float bearoffEquity(board &b)
{
	tAssert(isBearingOff(b));
	unsigned long onRoll = board_to_32(b, b.colorOnRoll());
	unsigned long notOnRoll = board_to_32(b, b.colorNotOnRoll());
	return (float) bearoffEquity(onRoll, notOnRoll);
}


	// winner won the game.  Return the equity counting gammons & backgammons.
static int gammon_check(board &nb, color_t winner)
{
	color_t loser = opponentOf(winner);
	int equity = 1;
	if (nb.checkersOnPoint(loser, 0) == 0){
		equity++;	// gammon
		if (nb.highestChecker(loser) > 18)
			equity++;	// backgammon
	}
	return equity;
}

static int win_check(board &nb, color_t side)
{
	if (nb.checkersOnPoint(side, 0) == 15)
		return gammon_check(nb, side);
	else
		return 0;
}

// get rid of this, moving to Game.  choose.cpp also uses it right now.
	// Returns nonzero if the game is over, in which case the
	// returned value is the equity of the player "on roll".
int gameOver(board &nb)
{
	int e;
	if (e = win_check(nb, nb.colorOnRoll()))
			return e;
	else if (e = win_check(nb, nb.colorNotOnRoll()))
		return -e;
	else
		return 0;
}


float littleE(board &nb)
{
	tAssert(nb.d1() == -1);
	tAssert(neural != 0);

	int e = gameOver(nb);
	if (e){
		return (float) e;
	}
	return  neural->feedBoard(nb);

}


/*
	This 1-ply equity estimator is appropriate when its result
	will not be compared with other positions.  It uses either
	littleE() or bearoffEquity(), as appropriate.

*/

float observeE(board &nb)
{
	if (isBearingOff(nb)){
		return bearoffEquity(nb);
	} else {
		return littleE(nb);
	}
}


float bigE(board &board)
{
	return lookAhead(board, observeE);
}


float neural_2ply(board &board)
{
	return lookAhead(board, littleE);
}




float quickPlay(board &board, move &m)
{
	return selectMove(board, m, littleE);
}

void slowPlay(board &board, move &m)
{
	selectMove(board, m, bigE);
}




/*******************************************************************/
/*******************************************************************/

/*
  New Hit Stuff.  This is too slow to use, but it is correct, and
  useful for evaluating and debugging the heuristic hit calculations.
*/


class queryHit : public callBack {
	board &bb;
	int startOnBar;
	int foundHit;

	public:
		int tries;
		int callBackF(board &b){
			tries++;
			if (bb.checkersOnBar(bb.colorOnRoll()) > startOnBar){
				foundHit = 1;
					// returns 1 to short circuit.
					// Shortcircuiting functionality is discontinued.
				return 1;

			}
			return 0;
		}
		int doesHit(){
			return foundHit;
		}
		queryHit(board &b) : bb(b) {
			startOnBar = b.checkersOnBar(b.colorNotOnRoll());
			foundHit = 0;
			tries = 0;
		}
};


int doesHit(board &b, int &tries)
{
	tAssert(b.d1() != -1 && b.d2() != -1);

	queryHit qH(b);
	b.play(qH);
	tries = qH.tries;
//	dprintf("doesHit() returns %d after %d tries\n", qH.doesHit(), tries);
	return qH.doesHit();
}


int hitLookAhead(board &board, int &tries)
{
	tAssert(board.d1() == -1);
	tAssert(board.colorOnRoll() != nobody);

	cLookAhead lh;
	int totalHits = 0;

	int _tries;

	for (int i = 0; i < 21; i++){
			// Set up each possible dice roll.
		int &r1 = lh.lookItem[i].r1;
		int &r2 = lh.lookItem[i].r2;
		
		board.setDice(r1, r2);
			// We play the dice.
		int thisHits = doesHit(board, _tries);
		tries += _tries;

			// Weight the result appropriately.
		if (r1 != r2){
			totalHits += 2 * thisHits;
		} else {
			totalHits += thisHits;
		}
	}
	board.setDice(-1, -1);
	return totalHits;
}

int noBlots(board &fromBoard, color_t victim)
{
	for (int i = 1; i <= 24; i++){
		if (fromBoard.checkersOnPoint(victim, i) == 1)
			return 0;
	}
	return 1;
}

int noContact(board &b)
{
	return (b.highestChecker(white) + b.highestChecker(black)) < 25;
}

// For compatibility with num_hits(), this takes a color that
// says who is on roll  The num_hits() code ignores this info in
// the board structure.


int numHits(board &fromBoard, color_t victim)
{
#ifdef CHECK_BOARD
	board save = fromBoard;
#endif
	if (noContact(fromBoard) || noBlots(fromBoard, victim))
		return 0;


	color_t origRoller = fromBoard.colorOnRoll();
	fromBoard.setRoller(opponentOf(victim));


	int tries = 0;
	int i = hitLookAhead(fromBoard, tries);

	fromBoard.setRoller(origRoller);
#ifdef CHECK_BOARD
	tAssert(memcmp(&save, &fromBoard, sizeof(save)) == 0);
#endif
	return i;
}

