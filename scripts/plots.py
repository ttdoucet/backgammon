#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os
import click
import sys

from scipy.signal import savgol_filter

def smooth(y):
    yhat = savgol_filter(y, 7, 1)
    return yhat


fig, ax = plt.subplots()

def do_file(filename):
    playoffs = pd.read_csv(filename)
    playoffs.loc[:, 'equity-smoothed'] = smooth(playoffs.equity.values)

    name = os.path.basename(filename)
    playoffs.plot(ax=ax, y='equity', grid=True, label=name)
    playoffs.plot(ax=ax, y='equity-smoothed', grid=True, label=name+' (smoothed)')


for filename in sys.argv[1:]:
    do_file(filename)

plt.show()
