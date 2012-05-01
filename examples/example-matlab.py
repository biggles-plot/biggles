#!/usr/bin/env python

import sys
sys.path.insert(1,'..')
sys.ps1 = None

import numpy
from biggles.matlab import *

def press_return():
	print "[press return]"
	sys.stdin.readline()

x = numpy.arange(-10,10);
y = x**2;
e = y/4

subplot(2,3,1)
plot(list(x), list(y), 'ko-')

subplot(2,3,2)
plot(x, y, 'ro')

subplot(2,3,3)
plot(x, y, 'go')

subplot(2,3,4)
plot(x, y, 'b-')

subplot(2,3,5)
plot([0, 1], [0, 1], 'o')

drawnow(width=500, height=200)
press_return()

drawnow()
press_return()
clf()

plot(x, y, '.')
drawnow()
press_return()

clf()
plot(x, y, 'r*-')
hold_on()
plot(x, -y, 'bv-')
errorbar(x, y/2, e, 'ko-')
title('foo')
xlabel('xbar')
ylabel('ybar')
hold_off()
drawnow()

press_return()
