#include "game.h"

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

