/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

class cmdopts : public cmdline
{
public:
    int trials = 1000;
    bool display_moves = false;
    int every = 0;
    uint64_t user_seed = -1;

    cmdopts()
    {
        setopt('n', "--games", trials,        "number of trials.");
        setopt('s', "--seed",  user_seed,     "seed for random-number generator.");
        setopt('e', "--every",   every,       "Report every n games.");
        setopt('d',            display_moves, "display moves.");
    }
};

cmdopts opts;

class AnnotatedGame : public Game
{
public:
    AnnotatedGame(Player& wh, Player& bl, uint64_t seed) : Game(wh, bl, seed) {}

protected:
    void reportMove(board bd, moves mv) override
    {
        if (opts.display_moves == false)
            return;

        std::string s = moveStr(mv);
        cout << board::colorname(b.onRoll()) << " rolls "
             << bd.d1() << " " <<  bd.d2()
             << " and moves " << s << '\n';
    }
};

static void playoffSession(int trials, Player& whitePlayer, Player& blackPlayer, uint64_t seed)
{
    AnnotatedGame game(whitePlayer, blackPlayer, seed);

    int numGames;
    double whitePoints = 0.0;

    for (numGames = 1; numGames <= trials ; numGames++)
    {
        double white_eq = game.playGame();
        whitePoints += white_eq;

        if (opts.every && !(numGames % opts.every))
        {
            std::ostringstream ss;

            ss << std::fixed << "Game " << numGames << ": "
               << std::setprecision(2) << std::setw(5) << white_eq << "... ";

            ss << "white equity/game = "
               << std::setprecision(3) << whitePoints/numGames
               << " (total "
               << std::setprecision(2) << whitePoints
               << ")\n";

            cout << ss.str();
        }
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

    cout << "white: " << net_name[0] << endl;
    cout << "black: " << net_name[1] << endl;

    NeuralNetPlayer whitePlayer("white", net_name[0]);
    NeuralNetPlayer blackPlayer("black", net_name[1]);

    playoffSession(opts.trials, whitePlayer, blackPlayer, opts.user_seed);

    return 0;
}
