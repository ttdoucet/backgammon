/* Written by Todd Doucet. */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "cmdline.h"

using namespace std;

class PlayoffOptions : public cmdline
{
public:
    int trials = 10000;
    bool display_moves = false;
    int every = 1000;
    uint64_t user_seed = -1;

    // derived
    string white_name;
    string black_name;

    PlayoffOptions()
    {
        setopt('n', "--games", trials,        "number of trials.");
        setopt('s', "--seed",  user_seed,     "seed for random-number generator.");
        setopt('e', "--every", every,         "Report every n games.");
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
        BgNetReader bgr;

        std::unique_ptr<BgNet> whitenet = bgr.read(opts.white_name);
        std::unique_ptr<BgNet> blacknet = bgr.read(opts.black_name);

        NeuralNetPlayer whitePlayer(*whitenet);
        NeuralNetPlayer blackPlayer(*blacknet);

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

        double whitePoints = 0.0;

        int sw = 0, sl = 0;  // Single wins & losses
        int gw = 0, gl = 0;  // Gammons
        int bw = 0, bl = 0;  // Backgammons

        for (int numGame = 1; numGame <= trials ; numGame++)
        {
            auto white_eq = game.playGame();
            whitePoints += white_eq;

            switch (white_eq)
            {
                case 1: sw++; break;
                case 2: gw++; break;
                case 3: bw++; break;

                case -1: sl++; break;
                case -2: gl++; break;
                case -3: bl++; break;
            }

            if (opts.every && !(numGame % opts.every))
            {
                std::ostringstream ss;

                ss << std::fixed << "Game " << numGame << ": "
                   << std::setprecision(2) << std::setw(2) << white_eq << ", ";

                ss << "white equity/game: "
                   << std::setprecision(3) << whitePoints/numGame
                   << ", total: "
                   << std::fixed << std::setprecision(1) << whitePoints
                   << "\n";

                cout << ss.str();
            }
        }

        cerr << opts.white_name;
        cerr << ", " << opts.black_name;
        cerr << ", " << trials;
        cerr << ", " <<  std::setprecision(3) << whitePoints/trials;
        cerr << ", " << sw;
        cerr << ", " << sl;
        cerr << ", " << gw;
        cerr << ", " << gl;
        cerr << ", " << bw;
        cerr << ", " << bl;
        cerr << "\n";
    }
};

int main(int argc, char *argv[])
{
    PlayoffOptions opts;
    opts.parse(argc, argv);
    return PlayoffSession(opts).run();
}
