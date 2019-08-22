#include "board.h"
#include "random.h"
#include "game.h"
#include <stdio.h>
#include "ttydisp.h"
#include "console.h"

/*
 * Learning will be done by a specialized Player.
 *
 * Text interaction with the user will be done by a specialized Player.
 *
 * Playoffs will be done by a pair of specialized Players.
 *
 * The dice will be handled by the Game.
 *
 * Once this higher-level stuff is cleaned up and pretty, I hope
 * to have the tightening up trickle down to choose.cpp, which
 * is currently implemented with a bunch of routines modifying static
 * global variables.  It obviously could be better done as an object.
 *
 */


#include <string>
#include <iostream>

class AnnotatedGame : public Game
{
public:
    AnnotatedGame(Player *wh, Player *bl) : Game(wh, bl) {}

protected:
    void reportMove(board bd, move mv)
    {
        std::string s = moveStr(mv);
        console << board::colorname(b.colorOnRoll()) << " rolls " << bd.d1() << " " <<  bd.d2() << " and moves " << s << '\n';
    }
};


Game::Game(Player *wh, Player *bl) :
    whitePlayer(*wh),
    blackPlayer(*bl)
{
    b.clearBoard();
}

double Game::playGame(bool verbose)
{
    whitePlayer.setColor(white);
    blackPlayer.setColor(black);

    whitePlayer.prepareToPlay();
    blackPlayer.prepareToPlay();

// unnecessary assignment?
    Player *playerOnRoll = setupGame();
    move m;

    while( gameOver() == false ){
        playerOnRoll = playerFor(b.colorOnRoll());

        playerOnRoll->chooseMove(b, m);

        extern int display_moves;
        if (display_moves)
            reportMove(b, m);
        applyMove(b, m);
//                extern int display_moves;
//                if (display_moves)
//                    display_board(b, white);

        b.setDice( throw_die(), throw_die() );
    }

    color_t winner = winningColor();
    color_t loser = opponentOf(winner);


    double whiteEquity = (white == winner) ? 1.0 : -1.0;

    bool gammon = (b.checkersOnPoint(loser, 0) == 0);
    bool backgammon = (gammon && b.highestChecker(loser) > 18);

    if (backgammon)
        whiteEquity *= 3;
    else if (gammon)
        whiteEquity *= 2;

    whitePlayer.finalEquity(whiteEquity);
    blackPlayer.finalEquity(-whiteEquity);

    return whiteEquity;
}

void playoffSession(int trials, Player *whitePlayer, Player *blackPlayer)
{
    AnnotatedGame game(whitePlayer, blackPlayer);
    
    int numGames;
    double whitePoints = 0.0;
    for (numGames = 1; numGames <= trials ; numGames++){
        char buf[80];
            
        double white_eq = game.playGame(false);
        whitePoints += white_eq;

        sprintf(buf, "Game %d: %5.2f... ", numGames, white_eq);
        console << buf;
                
        sprintf(buf, "white equity/game = %.3f (total %.2f)\n", whitePoints/numGames, whitePoints);
        console << buf;
    }
}

