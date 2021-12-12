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

    def init_net(self):
        cmd = f"{self.traincmd} --games 0 -w {self.kind} -o {self.basename}-0.w"
        print("# Initialize a new random network.")
        print("#")
        print(cmd);
        if self.execute:
            os.system(cmd)
        self.last = 0
        
    def train(self, games, extra=None):
        fromfile = f'{self.basename}-{self.last}.w'
        tofile = f'{self.basename}-{self.last+1}.w'
        cmd  = f'{self.traincmd} --alpha {self.alpha} ' \
               f'--lambda {self.lambda_} ' \
               f'--games {games} '
        if self.dual:
            cmd += '-d '
        if extra is not None:
            cmd += f'{extra} '
        cmd += f'-w {fromfile} -o {tofile}'

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
    def __init__(self, ratio, rounds):
        self.decay = ratio ** (1 / rounds)

    def train(self, trainer, rounds, games):
        if trainer.last is None:
            trainer.init_net();

        print("")
        print("# initial alpha:", trainer.alpha)
        print("# final alpha:", trainer.alpha * (self.decay ** rounds))
        print("# lambda:", trainer.lambda_)
        print("# games per round:", games)
        print("# rounds:", rounds)
        print("# decay per round:", self.decay)
        if trainer.momentum:
            print('# momentum:', trainer.momentum)
        if trainer.batchsize > 1:
            print("# batch size:", trainer.batchsize)
        print("#")

        for i in range(rounds):
            trainer.train(games, f'--alpha-end {trainer.alpha * self.decay}')
            trainer.alpha *= self.decay


sched_M50 =  Sched_anneal(1.0 / 20.0,  50)
sched_M100 = Sched_anneal(1.0 / 20.0, 100)
sched_M200 = Sched_anneal(1.0 / 20.0, 200)
sched_M400 = Sched_anneal(1.0 / 20.0, 400)

#sched_fixed = Sched_const();



