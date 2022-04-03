#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import random
import dtk_genericindividual as gi

import pdb

def expose( action, prob ):
    #print( "expose: " + action + " with value " + str( prob ) )
    if random.random() < 0.1:
        if gi.get_immunity() < 1:
            print( "Let's infect based on random draw." )
            return 1
        else:
            return 0
    else:
        print( "Let's NOT infect based on random draw." )
        return 0

gi.my_set_callback( expose )
def get_schema():
    schema = gi.get_schema()
    print( schema )

#print( "getting schema." )
#get_schema()

print( "creating individual." )
gi.create()
for tstep in range( 0,100 ):
    print( "Updating individual, age = {0}, infected = {1}, immunity = {2}.".format( gi.get_age(), gi.is_infected(), gi.get_immunity() ) )
    gi.update( 1 )

print( "Done!" )
