/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <string>
#include <iostream>
#include <iomanip>

#include "game.h"
#include "human.h"
#include "math.h"
#include "ttydisp.h"

using std::cout;

void HumanPlayer::chooseMove(const board& fromBoard, moves& choice)
{
    while (1)
    {
        board toBoard = fromBoard;
        getUserBoard(toBoard);
        if (legalToMove(fromBoard, toBoard, choice) )
            return;
        else
            cout << "Move not legal.\n";
    }
}

void HumanPlayer::prepareToPlay()
{
    cout << "Starting new game.\n";
}

void HumanPlayer::finalEquity(float e)
{
    std::ostringstream buf;
    buf << std::fixed << std::setprecision(3);

    if (e > 0)
        buf << "You won " << e << " points.  Congratulations!\n";
    else if (e < 0)
        buf << "You lost " << fabs(e) << " points.\n";
    else
        buf << "Game over, and it is an exact tie.\n";

    cout << buf.str();
}


/* 
 * Parsing the user command.
 */
static bool isdigit(char ch)
{
    return (ch >= '0' && ch <= '9');
}

static bool point_ok(int p)
{
    if (p > 25 || p < 0)
    {
        cout << "Illegal checker position.\n";
        return false;
    }
    return true;
}


void HumanPlayer::parseUserMove(const board& fromBoard, board& toBoard, char *s)
{
    int from, to;
    board bd = fromBoard;
    color_t color = bd.onRoll();

    while(*s == ' ')
        s++;
    while (isdigit(*s))
    {
        for(from = 0; isdigit(*s); s++)
            from = 10 * from + *s - '0';
        while(*s==' ')
            s++;

        if (!isdigit(*s))
            return;

        for(to = 0; isdigit(*s); s++)
            to = 10 * to + *s - '0';

        if (!point_ok(from) || !point_ok(to))
            return;

        bd.moveChecker(color, from, to);
        while(*s==' ')
            s++;
    }
    bd.pickupDice();
    toBoard = bd;
}

void HumanPlayer::getUserBoard(board& bd)
{
    display_board(bd, bd.onRoll());
    cout << "You roll " << bd.d1() << " and " << bd.d2() << "> ";

    char userString[80];
    char  *ignore = fgets(userString, sizeof(userString), stdin);

    board newBoard;
    parseUserMove(bd, newBoard, userString);
    bd = newBoard;
}

class enumerate : public callBack
{
protected:
    board targetBoard;
    moves savedMove;
    bool found;

public:
    enumerate(board target) : targetBoard(target), found(false) { }
    bool foundLegalMove() { return found; }
    moves foundMove() { return savedMove; }

    int callBackF(const board &b) override
    {
        if (b == targetBoard)
        {
            found = true;
            savedMove = m;
        }
        return 0;
    }
};

bool HumanPlayer::legalToMove(board fromBoard, board toBoard, moves& mv)
{
    enumerate en(toBoard);
    plays(fromBoard, en);
    if (en.foundLegalMove())
    {
        mv = en.foundMove();
        return true;
    }
    return false;
}
