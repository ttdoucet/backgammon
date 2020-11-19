#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os
import sys
import re

def smooth(y):
    from scipy.signal import savgol_filter
    return savgol_filter(y, 7, 1)

def sequence(names):
    def seqno(name):
        v = re.findall(r'\d+', name)
        return int(v[-1])
    return [seqno(name) for name in names]

fig, ax = plt.subplots()

def plot_file(filename):
    bname = os.path.basename(filename)
    name = os.path.splitext(bname)[0]

    playoffs = pd.read_csv(filename)
    playoffs.loc[:, 'equity-smoothed'] = smooth(playoffs.equity.values)
    playoffs.loc[:, 'millions'] = sequence(playoffs['white'].values)

    playoffs.plot(ax=ax, x='millions', y='equity', grid=True, label=name)
    playoffs.plot(ax=ax, x='millions', y='equity-smoothed', grid=True, label=name+' (smoothed)')


for filename in sys.argv[1:]:
    plot_file(filename)

plt.show()
