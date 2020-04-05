#!/usr/bin/env python
import os

games = 1000000
alpha = 0.02
lambda_ = 0.85

def doit(fromfile, tofile):
    cmd  = f'./train --alpha {alpha} --lambda {lambda_} --games {games} -w {fromfile} -o {tofile} -e 1000'
    print(cmd)
    os.system(cmd)

doit('random', 'white-1.w')

for i in range(1, 20):
    doit(f'white-{i}.w', f'white-{i+1}.w')

