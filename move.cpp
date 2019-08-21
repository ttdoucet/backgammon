/*
 * Copyright (C) 1993, 1997, 2008, 2013 by Todd Doucet.
 All Rights Reserved.
*/

#include <stdio.h>
#include "move.h"
#include <assert.h>

class LegalPlay
{
    board b;

public:
    LegalPlay(const board &bb) : b(bb) {}

    int play(callBack &callB);

    int checkersToPlay();
    int numMoves();

private:
    int openPoint(color_t color, int n);

    int duplicate_move(color_t color, struct move *mp);
    int move_die(int from, int to);
    void move_die(int from, int to, struct move *mp);
    void move_die(int f, int t, struct move *mp, int n);
    void unmove_die(struct move *mp);
    void unmove_die(struct move *mp, int n);

    int outputMove(callBack &callB);
    int doRoll(int r1, int r2, int pt, callBack &callB);
    int playNonDouble(int r1, int r2, int pt, callBack &callB);
    int playDouble(int r, int n, int pt, callBack &callB);
};

int LegalPlay::openPoint(color_t color, int n)
{
    return n==0 || b.checkersOnPoint(color, n) ||
        b.checkersOnPoint(opponentOf(color), opponentPoint(n)) <= 1 ;
}

int LegalPlay::duplicate_move(color_t color, struct move *mp)
{
    /* moves start at the same place, use only one. */
    if (mp->from[0] == mp->from[1] && mp->to[0] > mp->to[1])
        return 1;
    if (mp->from[0] == mp->to[1])
        fatal("unexpected type of skip move");

    /* a non-blocked skip move involving no hits,
     * and whose transpose involves no hits.
     */
    if (mp->to[0] == mp->from[1]){
        if (!mp->hit[0] && 
            !b.checkersOnPoint(opponentOf(color),
                               opponentPoint(mp->from[0] +
                                             (mp->to[1] - mp->from[1])))
            &&
            ((mp->to[0] - mp->from[0]) >
             (mp->to[1] - mp->from[1])) ){
            return 1;
        } 
    }
    return 0;
}

// Move the indicated checker, and
// return whether the move hits.
int LegalPlay::move_die(int from, int to)
{
    int op = opponentPoint(to);
    int hit = 0;

    if (to && b.checkersOnPoint(b.colorNotOnRoll(), op)){
        hit = 1;
        b.putOnBar(b.colorNotOnRoll(), op);
    }
    b.moveChecker(b.colorOnRoll(), from, to);
    return hit;
}

// Move the indicated piece and record the move
// in the structure.
void LegalPlay::move_die(int from, int to, struct move *mp)
{
    int &m = mp->moves;
    mp->from[m] = from;
    mp->to[m] = to;
    if (move_die(from, to))
        mp->hit[m] = 1;         
    m++;
}

void LegalPlay::move_die(int f, int t, struct move *mp, int n)
{
    while (n--)
        move_die(f, t, mp);
}

// Undo the last move indicated by the structure.
void LegalPlay::unmove_die(struct move *mp)
{
    int &m = mp->moves;
    --m;
    b.moveChecker(b.colorOnRoll(), mp->to[m], mp->from[m]);
    mp->from[m] = 0;

    if (mp->hit[m]){
        b.removeFromBar(b.colorNotOnRoll(),
                        opponentPoint(mp->to[m])
            );
        mp->hit[m] = 0;
    }
    mp->to[m] = 0;
}

void LegalPlay::unmove_die(struct move *mp, int n)
{
    while (n--)
        unmove_die(mp);
}

void applyMove(board& b,const move &m)
{
    for (int i = 0;  i < m.moves; i++){
        if (m.hit[i]){
            b.putOnBar(b.colorNotOnRoll(), opponentPoint(m.to[i]));
        }
        b.moveChecker(b.colorOnRoll(), m.from[i], m.to[i]);
    }
    b.pickupDice();
}

int LegalPlay::play(callBack &callB)
{
    int &nmoves = callB.nMoves;
    int &ctp = callB.checkersToPlay;

// testing
    nmoves = ctp = 0;
        

    callB.m.d1 = b.d1();
    callB.m.d2 = b.d2();

#ifdef CHECK_BOARD
// This check might go away in the production version.
    board save = b;
#endif

    assert( b.diceInCup() == false);
    if (b.d1() == b.d2()){
        for (int i = 4; i && nmoves == 0 ; i--){
            ctp = i;
            playDouble(b.d1(), i, 25, callB);
        }
    } else {
        ctp = 2;
        playNonDouble(b.d1(), b.d2(), 25, callB);
        if (nmoves == 0){
            ctp = 1;
            playNonDouble(std::max(b.d1(), b.d2()), 0, 25, callB);
        }
        if (nmoves == 0){
            ctp = 1;
            playNonDouble(std::min(b.d1(), b.d2()), 0, 25, callB);
        }
    }

    if (nmoves == 0){
        outputMove(callB);
        nmoves = 0;     // Dancing doesn't count.
        ctp = 0;        // No checkers to play.
    }
#ifdef CHECK_BOARD
    assert(memcmp(&save, &b, sizeof(save)) == 0);
#endif
    return nmoves;
}

int LegalPlay::outputMove(callBack &callB)
{
    callB.nMoves++;

    // Put the other guy on roll.
    int s1 = b.d1(), s2 = b.d2();
    b.pickupDice();

    // Do the callback.
    int i = callB.callBackF(b);

    // Restore the dice and put us back on roll.
    b.setRoller(b.colorNotOnRoll());
    b.setDice(s1, s2);
    return i;
}

int LegalPlay::doRoll(int r1, int r2, int pt, callBack &callB)
{
    move *mp = &(callB.m);
    int hi = b.highestChecker(b.colorOnRoll());

    if (b.checkersOnBar(b.colorOnRoll()) && pt != 25)
        return 0;

    if (r1 == 0)
        return 0;
    if (r1 > hi)
        r1 = hi;

    if ( ((pt - r1 > 0) || ((pt - r1 == 0) && (hi <= 6)) ) && openPoint(b.colorOnRoll(), pt - r1)){
        move_die(pt, pt - r1, mp);
        playNonDouble(r2, 0, pt, callB);
        unmove_die(mp);
    }
    return 0;
}

int LegalPlay::playNonDouble(int r1, int r2, int pt, callBack &callB)
{
    int &nm = callB.nMoves;
    move *mp = &(callB.m);

    if (r1 == 0  && !duplicate_move(b.colorOnRoll(), mp)){
        return outputMove(callB);
    }

    for (; pt; pt--){
        if (!b.checkersOnPoint(b.colorOnRoll(), pt))
            continue;
        doRoll(r1, r2, pt, callB);
        doRoll(r2, r1, pt, callB);
    }
    return 0;
}

int LegalPlay::playDouble(int r, int n, int pt, callBack &callB)
{
    int hi, i, checkers;
    move *mp = &(callB.m);

    if (n == 0){
        return outputMove(callB);
    }

    if (r > (hi = b.highestChecker(b.colorOnRoll())) )
        r = hi;

    for (; pt ; pt--){
        int can_move = ((pt - r > 0) || ((pt - r == 0) && (hi <= 6))) &&
            openPoint(b.colorOnRoll(), pt - r);
                        
        if ( (checkers = b.checkersOnPoint(b.colorOnRoll(), pt)) && can_move)
            break;
    }
    if (pt <= 0)
        return 0;
    if (b.checkersOnBar(b.colorOnRoll()) && pt != 25)
        return 0;

    if (n < checkers)
        checkers  = n;

    for (i = 0; i <= checkers; i++){
        move_die(pt, pt-r, mp, i);
        int retval = playDouble(r, n - i, pt - 1, callB);
        unmove_die(mp, i);
        if (retval){
            return 1;
        }
    }
    return 0;
}

// Exported routines.

int plays(const board& b, callBack& callB)
{
    LegalPlay plays(b);
    return plays.play(callB);
}

int numMoves(board& b)
{
    nullCallBack nullCallB;
    plays(b, nullCallB);
    return nullCallB.nMoves;
}

int checkersToPlay(board& b)
{
    nullCallBack nullCallB;
    plays(b, nullCallB);
    return nullCallB.checkersToPlay;
}

void moveStr(std::string& str, struct move &mp)
{
    char move_stringbuf[40];
    int count = 1;
    char countstr[5];
    char buf[40];
    char *s = move_stringbuf;
    *s = 0;
    char *last = s;
    int printed = 0;
    for (int i = 0; i < mp.moves; i++) {
        if (i > 0 && (mp.from[i] == mp.from[i-1]) && (mp.to[i] == mp.to[i-1])){
            sprintf(countstr, "(%d)", ++count);
        } else {
            count = 1;
        }
                
        if (count > 1){
            s = last;
            printed--;
        }

        sprintf(buf, "%s%d/%d%s%s",
                (printed != 0) ? ", " : " ",
                mp.from[i], mp.to[i],
                mp.hit[i - count + 1] ? "*" : "",
                count > 1 ? countstr : ""
            );

        last = s;
        printed++;
        s += sprintf(s, "%s", buf);

    }
    str += move_stringbuf;
}
