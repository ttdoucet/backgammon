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
    callBack& callB;

public:
    LegalPlay(const board &bb, callBack& cb) : b(bb), callB(cb) {}

    int play();
    int checkersToPlay();
    int numMoves();

private:
    bool openPoint(color_t color, int n) const;
    bool duplicate_move(color_t color) const;
    int move_die(int from, int to);
    void move_die(int from, int to, moves& m);
    void move_die(int f, int t, moves& m, int n);
    void unmove_die(moves& m);
    void unmove_die(moves& m, int n);
    void outputMove();
    void doRoll(int r1, int r2, int pt);
    void playNonDouble(int r1, int r2, int pt);
    void playDouble(int r, int n, int pt);
};

inline bool LegalPlay::openPoint(color_t color, int n) const
{
    return n==0 || b.checkersOnPoint(opponentOf(color), opponentPoint(n)) <= 1;
}

inline bool LegalPlay::duplicate_move(color_t color) const
{
    const moves& m = callB.m;

    if (m.count() != 2)
        return false;

    /* moves start at the same place, use only one. */
    if (m[0].from == m[1].from && m[0].to > m[1].to)
        return true;

    assert(m[0].from != m[1].to);

    /* a non-blocked skip move involving no hits,
     * and whose transpose involves no hits.
     */
    if (m[0].to == m[1].from)
    {
        if (!m[0].hit && 
            !b.checkersOnPoint(opponentOf(color),
                               opponentPoint(m[0].from + (m[1].to - m[1].from))
                              )
            &&
            ((m[0].to - m[0].from) > (m[1].to - m[1].from)) )
        {
            return true;
        } 
    }
    return false;
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
inline void LegalPlay::move_die(int from, int to, moves& m)
{
    int hit = move_die(from, to);
    m.push({from, to, hit});
}

inline void LegalPlay::move_die(int f, int t, moves& m, int n)
{
    while (n--)
        move_die(f, t, m);
}

// Undo the last move indicated by the structure.
inline void LegalPlay::unmove_die(moves& m)
{
    auto [from, to, hit] = m.pop();

    b.moveChecker(b.onRoll(), to, from);
    if (hit)
        b.removeFromBar(b.notOnRoll(), opponentPoint(to) );
}

inline void LegalPlay::unmove_die(moves& m, int n)
{
    while (n--)
        unmove_die(m);
}

inline int LegalPlay::play()
{
    int &nmoves = callB.nMoves;
    int &ctp = callB.checkersToPlay;
    nmoves = ctp = 0;
    board save = b;

    assert( b.diceInCup() == false);
    if (b.d1() == b.d2())
    {
        for (int i = 4; i && nmoves == 0 ; i--)
        {
            ctp = i;
            playDouble(b.d1(), i, 25);
        }
    } else {
        ctp = 2;
        playNonDouble(b.d1(), b.d2(), 25);
        if (nmoves == 0)
        {
            ctp = 1;
            playNonDouble(std::max(b.d1(), b.d2()), 0, 25);
        }
        if (nmoves == 0)
        {
            ctp = 1;
            playNonDouble(std::min(b.d1(), b.d2()), 0, 25);
        }
    }

    if (nmoves == 0)
    {
        outputMove();
        nmoves = 0;     // Dancing doesn't count.
        ctp = 0;        // No checkers to play.
    }

    assert(memcmp(&save, &b, sizeof(save)) == 0);
    return nmoves;
}

inline void LegalPlay::outputMove()
{
    callB.nMoves++;

    // Put the other guy on roll.
    int s1 = b.d1(), s2 = b.d2();
    b.pickupDice();

    // Do the callback.
    callB.callBackF(b);

    // Restore the dice and put us back on roll.
    b.setRoller(b.notOnRoll());
    b.setDice(s1, s2);
}

inline void LegalPlay::doRoll(int r1, int r2, int pt)
{
    if (r1 == 0)
        return;

    if (b.checkersOnBar(b.onRoll()) && pt != 25)
        return;

    int hi = b.highestChecker(b.onRoll());
    if (r1 > hi)
        r1 = hi;

    if ( ((pt > r1) || ((pt == r1) && (hi <= 6)) ) && openPoint(b.onRoll(), pt - r1))
    {
        moves& m = callB.m;

        move_die(pt, pt - r1, m);
        playNonDouble(r2, 0, pt);
        unmove_die(m);
    }
}

inline void LegalPlay::playNonDouble(int r1, int r2, int pt)
{
    if (r1 == 0  && !duplicate_move(b.onRoll()))
    {
        outputMove();
        return;
    }

    for (; pt; pt--)
    {
        if (!b.checkersOnPoint(b.onRoll(), pt))
            continue;
        doRoll(r1, r2, pt);
        doRoll(r2, r1, pt);
    }
}

inline void LegalPlay::playDouble(int r, int n, int pt)
{
    int hi, i, checkers;
    moves& m = callB.m;

    if (n == 0)
    {
        outputMove();
        return;
    }

    if (r > (hi = b.highestChecker(b.onRoll())) )
        r = hi;

    for (; pt; pt--)
    {
        const int dest = pt - r;
        int can_move = ((dest > 0) || ((dest == 0) && (hi <= 6))) &&
            openPoint(b.onRoll(), dest);
                        
        if ( (checkers = b.checkersOnPoint(b.onRoll(), pt)) && can_move )
            break;
    }
    if (pt <= 0)
        return;

    if (b.checkersOnBar(b.onRoll()) && pt != 25)
        return;

    if (n < checkers)
        checkers  = n;

    for (i = 0; i <= checkers; i++)
    {
        move_die(pt, pt-r, m, i);
        playDouble(r, n - i, pt - 1);
        unmove_die(m, i);
    }
}

// Exported routines.

void applyMove(board& b,const moves &m)
{
    for (int i = 0;  i < m.count(); i++)
    {
        if (m[i].hit)
            b.putOnBar(b.notOnRoll(), opponentPoint(m[i].to));
        b.moveChecker(b.onRoll(), m[i].from, m[i].to);
    }
    b.pickupDice();
}

int plays(const board& b, callBack& callB)
{
    LegalPlay plays(b, callB);
    return plays.play();
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

std::string moveStr(moves &m)
{
    std::ostringstream s;

    for (int i = 0; i < m.count();)
    {
        s << (s.str().length() ? ", " : " " );
        s << m[i].from << "/" << m[i].to;
        if (m[i].hit) s << "*";
        i++;

        int count = 1;
        for ( ; (i < m.count()) && (m[i].from == m[i-1].from) && (m[i].to == m[i-1].to); i++)
            count++;
        if (count > 1)
            s << "(" << count << ")";

    }
    return s.str();
}

