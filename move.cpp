/*
 * Copyright (C) 1993, 1997, 2008, 2013 by Todd Doucet.
 *               All Rights Reserved.
*/

#include <assert.h>
#include <iomanip>
#include "move.h"

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

    int duplicate_move(color_t color, move& m);
    int move_die(int from, int to);
    void move_die(int from, int to, move& m);
    void move_die(int f, int t, move& m, int n);
    void unmove_die(move& m);
    void unmove_die(move& m, int n);

    int outputMove(callBack &callB);
    int doRoll(int r1, int r2, int pt, callBack &callB);
    int playNonDouble(int r1, int r2, int pt, callBack &callB);
    int playDouble(int r, int n, int pt, callBack &callB);
};

inline int LegalPlay::openPoint(color_t color, int n)
{
    return n==0 || b.checkersOnPoint(opponentOf(color), opponentPoint(n)) <= 1 ;
}

inline int LegalPlay::duplicate_move(color_t color, move& m)
{
    /* moves start at the same place, use only one. */
    if (m.from[0] == m.from[1] && m.to[0] > m.to[1])
        return 1;
    if (m.from[0] == m.to[1])
        fatal("unexpected type of skip move");

    /* a non-blocked skip move involving no hits,
     * and whose transpose involves no hits.
     */
    if (m.to[0] == m.from[1])
    {
        if (!m.hit[0] && 
            !b.checkersOnPoint(opponentOf(color),
                               opponentPoint(m.from[0] +
                                             (m.to[1] - m.from[1])))
            &&
            ((m.to[0] - m.from[0]) >
             (m.to[1] - m.from[1])) )
        {
            return 1;
        } 
    }
    return 0;
}

// Move the indicated checker, and
// return whether the move hits.
inline int LegalPlay::move_die(int from, int to)
{
    int op = opponentPoint(to);
    int hit = 0;

    if (to && b.checkersOnPoint(b.notOnRoll(), op))
    {
        hit = 1;
        b.putOnBar(b.notOnRoll(), op);
    }
    b.moveChecker(b.onRoll(), from, to);
    return hit;
}

// Move the indicated piece and record the move
// in the structure.
inline void LegalPlay::move_die(int from, int to, move& m)
{
    int &n = m.moves;
    m.from[n] = from;
    m.to[n] = to;
    if (move_die(from, to))
        m.hit[n] = 1;
    n++;
}

inline void LegalPlay::move_die(int f, int t, move& m, int n)
{
    while (n--)
        move_die(f, t, m);
}

// Undo the last move indicated by the structure.
inline void LegalPlay::unmove_die(move& m)
{
    int &n = m.moves;
    --n;
    b.moveChecker(b.onRoll(), m.to[n], m.from[n]);
    m.from[n] = 0;

    if (m.hit[n])
    {
        b.removeFromBar(b.notOnRoll(),
                        opponentPoint(m.to[n])
                       );
        m.hit[n] = 0;
    }
    m.to[n] = 0;
}

inline void LegalPlay::unmove_die(move& m, int n)
{
    while (n--)
        unmove_die(m);
}

void applyMove(board& b,const move &m)
{
    for (int i = 0;  i < m.count(); i++)
    {
        if (m.hit[i])
            b.putOnBar(b.notOnRoll(), opponentPoint(m.to[i]));
        b.moveChecker(b.onRoll(), m.from[i], m.to[i]);
    }
    b.pickupDice();
}

inline int LegalPlay::play(callBack &callB)
{
    int &nmoves = callB.nMoves;
    int &ctp = callB.checkersToPlay;

// testing
    nmoves = ctp = 0;
        

//    callB.m.d1 = b.d1();
//    callB.m.d2 = b.d2();

    board save = b;

    assert( b.diceInCup() == false);
    if (b.d1() == b.d2())
    {
        for (int i = 4; i && nmoves == 0 ; i--)
        {
            ctp = i;
            playDouble(b.d1(), i, 25, callB);
        }
    } else {
        ctp = 2;
        playNonDouble(b.d1(), b.d2(), 25, callB);
        if (nmoves == 0)
        {
            ctp = 1;
            playNonDouble(std::max(b.d1(), b.d2()), 0, 25, callB);
        }
        if (nmoves == 0)
        {
            ctp = 1;
            playNonDouble(std::min(b.d1(), b.d2()), 0, 25, callB);
        }
    }

    if (nmoves == 0)
    {
        outputMove(callB);
        nmoves = 0;     // Dancing doesn't count.
        ctp = 0;        // No checkers to play.
    }

    assert(memcmp(&save, &b, sizeof(save)) == 0);
    return nmoves;
}

inline int LegalPlay::outputMove(callBack &callB)
{
    callB.nMoves++;

    // Put the other guy on roll.
    int s1 = b.d1(), s2 = b.d2();
    b.pickupDice();

    // Do the callback.
    int i = callB.callBackF(b);

    // Restore the dice and put us back on roll.
    b.setRoller(b.notOnRoll());
    b.setDice(s1, s2);
    return i;
}

inline int LegalPlay::doRoll(int r1, int r2, int pt, callBack &callB)
{
    move& m = callB.m;
    int hi = b.highestChecker(b.onRoll());

    if (b.checkersOnBar(b.onRoll()) && pt != 25)
        return 0;

    if (r1 == 0)
        return 0;
    if (r1 > hi)
        r1 = hi;

    if ( ((pt - r1 > 0) || ((pt - r1 == 0) && (hi <= 6)) ) && openPoint(b.onRoll(), pt - r1))
    {
        move_die(pt, pt - r1, m);
        playNonDouble(r2, 0, pt, callB);
        unmove_die(m);
    }
    return 0;
}

inline int LegalPlay::playNonDouble(int r1, int r2, int pt, callBack& callB)
{
    move& m = callB.m;

    if (r1 == 0  && !duplicate_move(b.onRoll(), m))
        return outputMove(callB);

    for (; pt; pt--)
    {
        if (!b.checkersOnPoint(b.onRoll(), pt))
            continue;
        doRoll(r1, r2, pt, callB);
        doRoll(r2, r1, pt, callB);
    }
    return 0;
}

inline int LegalPlay::playDouble(int r, int n, int pt, callBack &callB)
{
    int hi, i, checkers;
    move& m = callB.m;

    if (n == 0)
        return outputMove(callB);

    if (r > (hi = b.highestChecker(b.onRoll())) )
        r = hi;

    for (; pt ; pt--)
    {
        int can_move = ((pt - r > 0) || ((pt - r == 0) && (hi <= 6))) &&
            openPoint(b.onRoll(), pt - r);
                        
        if ( (checkers = b.checkersOnPoint(b.onRoll(), pt)) && can_move)
            break;
    }
    if (pt <= 0)
        return 0;
    if (b.checkersOnBar(b.onRoll()) && pt != 25)
        return 0;

    if (n < checkers)
        checkers  = n;

    for (i = 0; i <= checkers; i++)
    {
        move_die(pt, pt-r, m, i);
        int retval = playDouble(r, n - i, pt - 1, callB);
        unmove_die(m, i);
        if (retval)
            return 1;
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

std::string moveStr(move &m)
{
    std::ostringstream s;

    for (int i = 0; i < m.count();)
    {
        s << (s.str().length() ? ", " : " " );
        s << m.from[i] << "/" << m.to[i];
        if (m.hit[i]) s << "*";
        i++;

        int count = 1;
        for ( ; (i < m.count()) && (m.from[i] == m.from[i-1]) && (m.to[i] == m.to[i-1]); i++)
            count++;
        if (count > 1)
            s << "(" << count << ")";

    }
    return s.str();
}

