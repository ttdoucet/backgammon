/*
 * Copyright (C) 1993, 1997, 2008, 2013 by Todd Doucet.
 *                 All Rights Reserved.
 */

#pragma once
#include <cstring>
#include <cassert>

enum color_t { white = 0, black = 1};

inline color_t opponentOf(color_t color)
{
    return (color_t) ! (int) color;
}

inline int opponentPoint(int x)
{
    return (25 - (x));
}

class board
{
public:
    int d1() const { return die_1; }
    int d2() const { return die_2; }
    int diceInCup() const { return d1() < 1; }
    void setDice(int _d1, int _d2){ die_1 = _d1; die_2 = _d2;}

    void pickupDice()
    {
        on_roll = opponentOf(on_roll);
        setDice(0, 0);
    }

    void setRoller(color_t r) { on_roll = r; }

    color_t onRoll() const { return on_roll;}
    color_t notOnRoll() const { return opponentOf(on_roll);}

    inline int checkersOnPoint(color_t color, int point) const
    {
        return thePoints[color][point];
    }

    int moveChecker(color_t color, int from, int to)
    {
        if ( (to == 0) || (to == 25) )
        {
            _moveChecker(color, from, to);
            return 0;
        }
        int op = opponentPoint(to);
        int hit = 0;
        while(checkersOnPoint(opponentOf(color), op))
        {
            putOnBar(notOnRoll(), op);
            hit++;
        }
        _moveChecker(color, from, to);
        return hit;
    }

    void putOnBar(color_t color, int pt)
    {
        moveChecker(color, pt, 25);
    }

    void removeFromBar(color_t color, int pt)
    {
        moveChecker(color, 25, pt);
    }

    int checkersOnBar(color_t color) const 
    {
        return checkersOnPoint(color, 25);
    }

    int highestChecker(color_t color) const
    {
        int i;
        for (i = 25; i >= 0; i--)
            if (checkersOnPoint(color, i) > 0)
                break;
        return i;
    }

    int pipCount(color_t color) const
    {
        return pip[color];
    }

    bool operator==(const board& rhs) const
    {
        return memcmp(this, &rhs, sizeof(board)) == 0;
    }

    void clearBoard()
    {
        for (int i = 0; i <= 25; i++)
        {
            thePoints[0][i] = 0;
            thePoints[1][i] = 0;
        }
        pip[0] = pip[1] = 0;
        setRoller(white);
        setDice(0, 0);

        thePoints[0][0] = 15;
        thePoints[1][0] = 15;
    }

    board() { clearBoard(); }

    static const char *colorname(color_t color)
    {
        assert (color == white || color == black);
        if (color == white)
            return "White";
        else if (color == black)
            return "Black";
        return "";
    }

private:
    signed char thePoints[2][32];
    int pip[2];
    int die_1, die_2;
    color_t on_roll;

    void _moveChecker(color_t color, int from, int to)
    {
        thePoints[color][from]--;
        thePoints[color][to]++;

        pip[color] += (to - from);
    }
};
