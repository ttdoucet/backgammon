/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once
#include <cstring>
#include <cstdint>
#include <cassert>

enum color_t { white = 0, black = 1};

inline color_t opponentOf(color_t color)
{
    return (color_t) ! (int) color;
}

inline int opponentPoint(int x)
{
    return 25 - x;
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

    inline int myCheckersOnMyPoint(int point) const
    {
        return thePoints[ onRoll() ][point];
    }

    inline int myCheckersOnHisPoint(int point) const
    {
        return thePoints[ onRoll() ][ opponentPoint(point) ];
    }

    inline int hisCheckersOnMyPoint(int point) const
    {
        return thePoints[ notOnRoll() ][ opponentPoint(point) ];
    }

    inline int hisCheckersOnHisPoint(int point) const
    {
        return thePoints[ notOnRoll() ][ point ];
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

    int moveMyChecker(int from, int to)
    {
        return moveChecker(onRoll(), from, to);
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
    int8_t thePoints[2][26];
    int32_t pip[2];
    int32_t die_1, die_2;
    color_t on_roll;

    void _moveChecker(color_t color, int from, int to)
    {
        thePoints[color][from]--;
        thePoints[color][to]++;

        pip[color] += (to - from);
    }
};
