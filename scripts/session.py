#!/usr/bin/env python
import sys
import os

def schedule(n):
    l =  0.85
    a = 0.02 / 36

    if n >= 136:
        a = 0.005 / 36
    if n >= 200:
        a = 0.001 / 36
    if n >= 325:
        a = 0.0001 / 36

    return a, l

games = 1000000
every = 1000

def doit(fromfile, tofile, alpha=0.0, lambda_=1.0):
    cmd  = f'./train --alpha {alpha} --lambda {lambda_} --games {games} -w {fromfile} -o {tofile}  -d -e {every}'
    print(cmd)
    r = os.system(cmd)

    if r & 0xff:
        print("aborted\n");
        sys.exit(1)

    if r:
        print("error encountered, exiting\n");
        sys.exit(1)


#os.system("./train --games 0 -w netv3 -o random.w");
os.system("./train --games 0 -w Fc_Sig_H15_I3 -o random.w");

alpha, lambda_ = schedule(0)
doit('random.w', 'white-1.w', alpha, lambda_)

for i in range(1, 64):
    alpha, lambda_ = schedule(i)
    print("i:", i, "alpha: ", alpha, "lambda:", lambda_)
    doit(f'white-{i}.w', f'white-{i+1}.w', alpha, lambda_)

