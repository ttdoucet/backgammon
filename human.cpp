#include <string>
#include <iostream>
#include <iomanip>

#include "game.h"
#include "human.h"
#include "math.h"
#include "ttydisp.h"
#include "console.h"

void HumanPlayer::chooseMove(const board& fromBoard, move& choice)
{
    while (1){
        board toBoard = fromBoard;
        getUserBoard(toBoard);
        if (legalToMove(fromBoard, toBoard, choice) )
            return;
        else
            console << "Move not legal.\n";
    }
}

void HumanPlayer::prepareToPlay()
{
    console << "Starting new game.\n";
}

void HumanPlayer::finalEquity(double e)
{
    std::ostringstream buf;
    buf << std::fixed << std::setprecision(3);

    if (e > 0)
        buf << "You won " << e << " points.  Congratulations!\n";
    else if (e < 0)
        buf << "You lost " << fabs(e) << " points.\n";
    else
        buf << "Game over, and it is an exact tie.\n";

    console << buf.str();
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
    if (p > 25 || p < 0){
        console << "Illegal checker position.\n";
        return false;
    }
    return true;
}


void HumanPlayer::parseUserMove(const board& fromBoard, board& toBoard, char *s)
{
    int from, to;
    board bd = fromBoard;
    color_t color = bd.colorOnRoll();

    while(*s == ' ')
        s++;
    while (isdigit(*s)){
        for(from = 0; isdigit(*s); s++)
            from = 10 * from + *s - '0';
        while(*s==' ')
            s++;

        if (!isdigit(*s)){
            return;
        }
        for(to = 0; isdigit(*s); s++){
            to = 10 * to + *s - '0';
        }

        if (!point_ok(from) || !point_ok(to)){
            return;
        }

// will simplify with new moveChecker() semantics...
        int ck;
        if ( (ck = bd.checkersOnPoint( opponentOf(color), opponentPoint(to) )) > 0){
            while (ck--)
                bd.putOnBar(opponentOf(color), opponentPoint(to));
        }
        bd.moveChecker(color, from, to);
        while(*s==' ')
            s++;
    }
    bd.pickupDice();
    toBoard = bd;
}



void HumanPlayer::getUserBoard(board& bd)
{
    display_board(bd, bd.colorOnRoll());
    console << "You roll " << bd.d1() << " and " << bd.d2() << "> ";

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
    move savedMove;
    bool found;

public:
    enumerate(board target) : targetBoard(target), found(false)
        {
        }

    int callBackF(const board &b)
        {
            if (b == targetBoard){
                found = true;
                savedMove = m;
            }
            return 0;
        }
            
    bool foundLegalMove()
        {
            return found;
        }

    move foundMove()
        {
            return savedMove;
        }
};

bool HumanPlayer::legalToMove(board fromBoard, board toBoard, move& mv)
{
    enumerate en(toBoard);
//      fromBoard.play(en);
    plays(fromBoard, en);
    if (en.foundLegalMove() ){
        mv = en.foundMove();
        return true;
    }
    return false;
}
