#!/usr/bin/env python3

# Stuff to do:
#    x Create a new network conveniently.
#    x Figure out where to specify games per round.
#    x Make the train command customizable.
#    x Make sure Sched_const works.
#    x Add ability to execute commands in addition to printing them.
#    - Write Anneal schedule.
#    - Write convenient routines.

import os

class Trainer:
    def __init__(self, kind, basename=None, last=None, traincmd=None, execute=None):
        self.kind = kind
        self.last = last
        self.basename = basename if basename else kind
        self.traincmd = traincmd if traincmd else "train"
        self.execute = execute if execute else False

        self.alpha = 0.0005
        self.lambda_ = 0.85
        self.batchsize = 1
        self.momentum = 0
        self.dual = True
        self.every = 1000

    def preamble(self):
        print("")
        print("# alpha:", self.alpha)
        print("# lambda:", self.lambda_)
        if self.momentum:
            print('# momentum:', self.momentum)
        if self.batchsize > 1:
            print("# batch size:", self.batchsize)

    def train(self, games):
        if self.last is None:
            cmd = f"{self.traincmd} --games 0 -w {self.kind} -o {self.basename}-0.w"
            self.last = 0
        else:
            fromfile = f'{self.basename}-{self.last}.w'
            tofile = f'{self.basename}-{self.last+1}.w'
            cmd  = f'{self.traincmd} --alpha {self.alpha} ' \
                   f'--lambda {self.lambda_} ' \
                   f'--games {games} ' \
                   f'-d -e {self.every} ' \
                   f'-w {fromfile} -o {tofile}'
            if self.batchsize > 1:
                cmd += f' --batch-size {self.batchsize}'
            if self.momentum:
                cmd += f' -p {self.momentum}'
            self.last += 1
        print(cmd)
        if self.execute:
            os.system(cmd)

class Sched_const:
    def train(self, trainer, rounds, games):
        for i in range(rounds):
            trainer.train(games);

class Sched_anneal:
    def __init__(self, rate, over):
        pass
    def train(self, trainer, rounds, games):
        pass

sched_M200 = Sched_anneal(1.0 / 20.0, 200_000_000)
sched_fixed = Sched_const();
t = Trainer(kind='Fc_Sig_H60_I3N',
            basename="experiment",
            last=None, traincmd="./train",
            execute=False)

t.preamble()
sched_fixed.train(t, 1, 0)
sched_fixed.train(t, 25, 1000)

