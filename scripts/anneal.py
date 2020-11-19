#!/usr/bin/env python

train = '~/src/backgammon/train'

games_per_round = 1_000_000

def file_for(basename, number):
    return f'{basename}-{number}.w'

def anneal_schedule(a_start, a_stop, rounds, frm=0, lambda_=0.85, basename='white'):

    games = games_per_round * rounds
    decay = (a_stop / a_start) ** (1 / rounds)

    print("")
    print("# initial alpha:", a_start)
    print("# final alpha:", a_stop);
    print("# lambda:", lambda_)
    print("# games per round:", games_per_round)
    print("# rounds:", rounds)
    print("# decay per round:", decay)
    print("#")

    every = 1000
    alpha = a_start
    for i in range(frm, frm + rounds):

        fromfile = file_for(basename, i)
        tofile = file_for(basename, i+1)

        cmd  = f'{train} --alpha {alpha} ' \
               f'--alpha-end {alpha * decay} ' \
               f'--lambda {lambda_} ' \
               f'--games {games_per_round} ' \
               f'-d -e {every} ' \
               f'-w {fromfile} -o {tofile}'

        print( cmd)
        alpha = alpha * decay


def initialize_net(net_type, basename='white'):
    print("# Initialize a new random network.")
    print("#")
    cmd = f"{train} --games 0 -w {net_type} -o {basename}-0.w"
    print(cmd);


bname = 'h60-a'
a_start = 0.001
a_stop = a_start / 50
rounds = 100

initialize_net('Fc_Sig_H60_I3', basename=bname);
anneal_schedule(a_start, a_stop, rounds, basename=bname)
