#!/usr/bin/python

import sys
#sys.path.append( "./build/lib.linux-x86_64-2.7" )
import random
import dtk_typhoidindividual as gi
import json

import pdb

def expose( action, prob ):
    #print( "expose: " + action + " with value " + str( prob ) )
    if gi.is_infected() == 0 and random.random() < 0.1:
        imm = gi.get_immunity()
        if imm < 1:
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
gi.create( 10, 'F' )
for tstep in range( 0,1000 ):
    print( "Updating individual, age = {0}, infected = {1}, immunity = {2}.".format( gi.get_age(), gi.is_infected(), gi.get_immunity() ) )
    gi.update()
    serial_man = json.loads( gi.serialize() )
    print( json.dumps( serial_man["individual"], indent=4, sort_keys=True ) )

print( "Done!" )
