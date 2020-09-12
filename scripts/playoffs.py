#!/usr/bin/env python
import sys
import os

opponent = '~/src/backgammon/good.w'
trials = 5000
playoff_cmd = '~/src/backgammon/playoff'

output_file = 'playoffs.csv'

def doit(player, opponent):
    cmd = f'{playoff_cmd} -e 1000 -n {trials} {player} {opponent} 2>>{output_file}'
    print(cmd)
    r = os.system(cmd)

    if r & 0xff:
        print("aborted\n");
        sys.exit(1)

    if r:
        print("error encountered, exiting\n");
        sys.exit(1)


start = 1
stop = 5

os.system(f'echo white,black,trials,equity,sw,sl,gw,gl,bw,bl >>{output_file}')
for i in range(start, stop+1):
    player = f'white-{i}.w'
    doit(player, opponent)

