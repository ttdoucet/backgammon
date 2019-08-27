#include <string>
#include <iostream>
#include <iomanip>

#include "board.h"
#include "random.h"
#include "game.h"
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

class AnnotatedGame : public Game
{
public:
    AnnotatedGame(Player& wh, Player& bl) : Game(wh, bl) {}

protected:
    void reportMove(board bd, moves mv) override
    {
        std::string s = moveStr(mv);
        console << board::colorname(b.onRoll()) << " rolls "
                << bd.d1() << " " <<  bd.d2()
                << " and moves " << s << '\n';
    }
};


Game::Game(Player& wh, Player& bl) :
    whitePlayer(wh),
    blackPlayer(bl)
{
    b.clearBoard();
}

double Game::playGame(bool verbose)
{
    whitePlayer.setColor(white);
    blackPlayer.setColor(black);

    whitePlayer.prepareToPlay();
    blackPlayer.prepareToPlay();

    setupGame();
    moves m;

    while( gameOver(b) == false )
    {
        playerFor(b.onRoll()).chooseMove(b, m);

        if (verbose)
            reportMove(b, m);

        applyMove(b, m);

//      if (verbose)
//          display_board(b, white);

        b.setDice( throw_die(), throw_die() );
    }

    return score(b, white);
}


// probably does not belong here
void playoffSession(int trials, Player& whitePlayer, Player& blackPlayer, bool verbose)
{
    AnnotatedGame game(whitePlayer, blackPlayer);
    
    int numGames;
    double whitePoints = 0.0;

    for (numGames = 1; numGames <= trials ; numGames++)
    {
        double white_eq = game.playGame(verbose);
        whitePoints += white_eq;

        std::ostringstream ss;

        ss << std::fixed << "Game " << numGames << ": "
           << std::setprecision(2) << std::setw(5) << white_eq << "... ";

        ss << "white equity/game = "
           << std::setprecision(3) << whitePoints/numGames
           << " (total "
           << std::setprecision(2) << whitePoints
           << ")\n";

        console << ss.str();
    }
}

