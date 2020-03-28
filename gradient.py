from sympy import *

def s(x):
    return 1 / (1 + exp(-x))

# Model parameters
w1, w2 = var('w1 w2')

# Input
x = var('x')

# Feed-forward calculation.
y1 = w1 * x
y2 = s(y1)
y3 = y2 * w2
out = s(y3)

if simplify(diff(out, w2) - out*(1-out)*y2) == 0 :
    print("w1: yes")

if simplify(diff(out, w1) - out*(1-out)*w2*y2*(1-y2)*x) == 0 :
    print("w2: yes")





