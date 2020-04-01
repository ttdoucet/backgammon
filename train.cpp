/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

// To do: save the result of training
// To do: options for alpha and lambda


class cmdopts : public cmdline
{
public:
    int games = 1000;
    bool verbose = false;
    uint64_t user_seed = -1;

    cmdopts()
    {
        setopt('n', "--games", games,         "number of games.");
        setopt('s', "--seed",  user_seed,     "seed for random-number generator.");
        setopt('v', "--verbose", verbose,     "Report the result of each game.");
    }
};

cmdopts opts;

class TrainingGame : public Game
{
public:
    TrainingGame(Player& wh, Player& bl) : Game(wh, bl) {}

protected:
    void reportMove(board bd, moves mv) override
    {
    }
};

static void report(int numGames, double whitePoints)
{
            ostringstream ss;

            ss << "white equity/game = "
               << std::setprecision(3) << whitePoints/numGames
               << " (total "
               << std::setprecision(2) << whitePoints
               << ")\n";

            cout << ss.str();
}

static void trainingSession(int games, Player& whitePlayer, Player& blackPlayer)
{
    TrainingGame game(whitePlayer, blackPlayer);

    int numGames;
    double whitePoints = 0.0;

    for (numGames = 1; numGames <= games ; numGames++)
    {
        double white_eq = game.playGame();
        whitePoints += white_eq;

        if (opts.verbose)
        {
            ostringstream ss;
            ss << std::fixed << "Game: " << numGames << ": "
               << std::setprecision(2) << std::setw(5) << white_eq << "... ";
            cout << ss.str();

            report(numGames, whitePoints);
        }
    }

    if (opts.verbose == false)
    {
        cout << "Games: " << (numGames - 1)  << ": ";
        report(numGames, whitePoints);
    }
}

int main(int argc, char *argv[])
{
    string net_name[2];


    opts.parse(argc, argv);
    if (opts.ExtraArgs.size() == 1)
    {
        net_name[0] = "net.w";
        net_name[1] = "net.w";
    }
    else if (opts.ExtraArgs.size() == 2)
    {
        net_name[0] = opts.ExtraArgs[1];
        net_name[1] = "net.w";
    }
    else if (opts.ExtraArgs.size() == 3)
    {
        net_name[0] = opts.ExtraArgs[1];
        net_name[1] = opts.ExtraArgs[2];
    }

    setupRNG(opts.user_seed);

    cout << "white: " << net_name[0] << endl;
    cout << "black: " << net_name[1] << endl;

    Learner whitePlayer("white", net_name[0]);
    NeuralNetPlayer blackPlayer("black", net_name[1]);

    trainingSession(opts.games, whitePlayer, blackPlayer);

    return 0;
}
