$ demo --trials 19 --alpha 1e-4 --gamma 99.3 -f file.txt -v foo bar baz
Flag values:
  verbose: 1
  file name: file.txt
  trials: 19
  alpha: 0.0001
  gamma: 99.3

ExtraArgs:
  ./demo
  foo
  bar
  baz
$
$ demo -naf 19 1e-4 file.txt --gamma 99.3 -v foo bar baz
Flag values:
  verbose: 1
  file name: file.txt
  trials: 19
  alpha: 0.0001
  gamma: 99.3

ExtraArgs:
  ./demo
  foo
  bar
  baz
$
$ demo --help
Usage: demo [options] [args]
Meaningless set of switches to test code.

Options:
  -v                Extra informational messages.
  -f, --filename    <file> Name of the output file.
  -n, --trials      <num> Number of games to play.
  -a, --alpha       <float> Learning rate.
      --gamma       <float> Credit decay rate.
      --help        Print this message and exit.
$ 
