#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_random

print( "100 random numbers, uniform distribution from 0 to 1.0" )
for i in range(0,100):
    print( str( dtk_random.random() ) )

print( "\n100 random Gaussian numbers." )
for i in range(0,100):
    print( str( dtk_random.gauss() ) )

print( "\n100 random Poisson numbers." )
for i in range(0,100):
    print( str( dtk_random.poisson() ) )

print( "\n100 random Exponential draws." )
for i in range(0,100):
    print( str( dtk_random.expdist() ) )

print( "\n100 random LogLog draws." )
for i in range(0,100):
    print( str( dtk_random.loglogistic() ) )

print( "\n100 random Weibull draws." )
for i in range(0,100):
    print( str( dtk_random.weibull() ) )
