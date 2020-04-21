/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

class PlayoffOptions : public cmdline
{
public:
    int trials = 10000;
    bool display_moves = false;
    int every = 100;
    uint64_t user_seed = -1;

    // derived
    string white_name;
    string black_name;

    PlayoffOptions()
    {
        setopt('n', "--games", trials,        "number of trials.");
        setopt('s', "--seed",  user_seed,     "seed for random-number generator.");
        setopt('e', "--every",   every,       "Report every n games.");
        setopt('d',            display_moves, "display moves.");
    }

    void parse(int argc, char *argv[]) override
    {
        cmdline::parse(argc, argv);
        if (ExtraArgs.size() == 1)
        {
            white_name = "net.w";
            black_name = "net.w";
        }
        else if (ExtraArgs.size() == 2)
        {
            white_name = ExtraArgs[1];
            black_name = "net.w";
        }
        else if (ExtraArgs.size() == 3)
        {
            white_name = ExtraArgs[1];
            black_name = ExtraArgs[2];
        }

        cout << "white: " << white_name << endl;
        cout << "black: " << black_name << endl;
    }
};

class PlayoffSession
{
public:
    PlayoffSession(const PlayoffOptions& options)
        : opts{options}
    {
    }

    int run()
    {
        NeuralNetPlayer whitePlayer(opts.white_name);
        NeuralNetPlayer blackPlayer(opts.black_name);        

        playoffSession(opts.trials, whitePlayer, blackPlayer, opts.user_seed);
        return 0;
    }

private:
    const PlayoffOptions& opts;

    class AnnotatedGame : public Game
    {
    public:
        AnnotatedGame(Player& wh, Player& bl, uint64_t seed, const PlayoffOptions& options)
            : Game(wh, bl, seed),
              opts{options}
            {
            }

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
    private:
        const PlayoffOptions& opts;

    };

    void playoffSession(int trials, Player& whitePlayer, Player& blackPlayer, uint64_t seed)
    {
        AnnotatedGame game(whitePlayer, blackPlayer, seed, opts);

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
                   << std::setprecision(2) << std::setw(5) << white_eq << ", ";

                ss << "white equity/game: "
                   << std::setprecision(3) << whitePoints/numGames
                   << ", total: "
                   << std::fixed << std::setprecision(1) << whitePoints
                   << "\n";

                cout << ss.str();
            }
        }
    }
};

int main(int argc, char *argv[])
{
    PlayoffOptions opts;
    opts.parse(argc, argv);
    return PlayoffSession(opts).run();
}
