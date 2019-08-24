/*
 * Copyright (C) 1993, 2008 by Todd Doucet.  All Rights Reserved.
 */

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "console.h"

/*
 * Command-line stuff
 */
void usage()
{
    console << "usage: train  [flags] [playerOne] [playerTwo]\n";
    console << "\t-d display moves.\n";
    console << "\t-aAlpha\n";
    console << "\t-lLambda\n";
    console << "\t-nTrials.\n";
    console << "\t-Sseedval\n";
    fatal("");
}

int numPlayers;
const char *player_name[2];

float alpha = 0.02f;  // not hooked up
float lambda = 0.85f; // not hooked up
int trials = 500;
int isTraining;  // not hooked up
int display_moves;  // hack

unsigned long user_seed;
bool explicitSeed = false;

void cmdline(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'n':
                trials = std::stoi(argv[i] + 2);
                break;
            case 'a':
                alpha = std::stof(argv[i] + 2);
                console << "alpha: " << alpha << "\n";
                break;

            case 'l':
                lambda = std::stof(argv[i] + 2);
                console << "lambda: " << lambda << "\n";
                break;

            case 'S':
                user_seed = std::stoi(argv[i] + 2);
                explicitSeed = true;
                break;

            case 'd':
                display_moves++;
                break;

            default:
            case '?':
                usage();
                break;
            }
        }
        else
        {
            if (numPlayers == 2)
                usage();
            player_name[numPlayers] = argv[i];
            numPlayers++;
        }
    }

    if (numPlayers == 0)
    {
        player_name[numPlayers++] = "net.w";
        isTraining = 1;
    }
    else if (numPlayers == 1)
        isTraining = 1;
    else if (numPlayers == 2)
        isTraining = 0;
    else
        usage();
}

/*
 *  Main routine. 
 */


void setupRNG()
{
    if (explicitSeed == true)
    {
        console << "RNG using user-specified seed: " << user_seed << "\n";
        set_seed(user_seed);
    }
    else
        randomize_seed();
}

#include <chrono>
using namespace std::chrono;

int main(int argc, char *argv[])
{
    cmdline(argc, argv);
    setupRNG();

    NeuralNetPlayer *whitePlayer = new NeuralNetPlayer("white", player_name[0]);
//  Player *whitePlayer = new HumanPlayer("human");

    Player *blackPlayer = new NeuralNetPlayer("black", player_name[1]);
//  Player *blackPlayer = new NeuralNetLearner("black", player_name[1]);

    auto start = high_resolution_clock::now();

    playoffSession(trials, whitePlayer, blackPlayer);

    auto stop = high_resolution_clock::now();

    std::cerr << duration_cast<nanoseconds> (stop - start).count() << " nsec\n";
    std::cerr << duration_cast<microseconds> (stop - start).count() << " usec\n";


    std::cerr << "period: " << high_resolution_clock::period::num
              << " / "
              << high_resolution_clock::period::den
              << "\n";

    return 0;
}
