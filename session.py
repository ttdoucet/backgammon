#!/usr/bin/env python
import sys
import os

games = 1000000
every = 1000
alpha = 0.02
lambda_ = 0.85

def doit(fromfile, tofile):
    cmd  = f'./train --alpha {alpha} --lambda {lambda_} --games {games} -w {fromfile} -o {tofile} -e {every}'
    print(cmd)
    r = os.system(cmd)

    if r & 0xff:
        print("aborted\n");
        sys.exit(1)

    if r:
        print("error encountered, exiting\n");
        sys.exit(1)


os.system("./train --games 0 -w random -o random.w");
doit('random.w', 'white-1.w')

for i in range(1, 54):
    doit(f'white-{i}.w', f'white-{i+1}.w')

