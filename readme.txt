#
# Backgammon playing and training via a neural network.
#

$ ./playoff --help
Usage: ./playoff [options] [args]

Options:
  -n, --games       number of trials.
  -s, --seed        seed for random-number generator.
  -d                display moves.
      --help        Print this message and exit.
$ 
$ 
$ ./playoff -n 10 net.w good.w
white: net.w
black: good.w
Game 1:  2.00... white equity/game = 2.000 (total 2.00)
Game 2: -2.00... white equity/game = 0.000 (total 0.00)
Game 3: -1.00... white equity/game = -0.333 (total -1.00)
Game 4:  1.00... white equity/game = 0.000 (total 0.00)
Game 5:  1.00... white equity/game = 0.200 (total 1.00)
Game 6: -2.00... white equity/game = -0.167 (total -1.00)
Game 7: -2.00... white equity/game = -0.429 (total -3.00)
Game 8:  1.00... white equity/game = -0.250 (total -2.00)
Game 9:  1.00... white equity/game = -0.111 (total -1.00)
Game 10:  1.00... white equity/game = 0.000 (total 0.00)
$ 
$ time ./playoff -n 1000 net.w good.w >/dev/null

real	0m1.158s
user	0m1.152s
sys	0m0.000s
$
