#!/usr/bin/python3.6

import sys
sys.path.append( "./build/lib.linux-x86_64-3.7" )
import dtk_airborne_intrahost as gi

boy = gi.create( ( 0, 100.0, 1.0 ) )
girl = gi.create( ( 1, 100.0, 1.0 ) )

for t in range(0,10):
    for dude in [ boy, girl ]:
        print( "Test get_age..." )
        print( gi.get_age( dude ) )
        print( "Test is_infected..." )
        print( gi.is_infected( dude ) )
        print( "Test get_immunity..." )
        print( gi.get_immunity( dude ) )
        print( "Test is_possible_mother..." )
        print( gi.is_possible_mother( dude ) )
        print( "Test is_pregnant..." )
        print( gi.is_pregnant( dude ) )
        print( "Test is_dead..." )
        print( gi.is_dead( dude ) )
        if gi.is_infected( dude ):
            print( "Test get_infection_age..." )
            print( gi.get_infection_age( dude ) )
            print( "Test get_infectiousness..." )
            print( gi.get_infectiousness( dude ) )
        print( "Test serialize..." )
        print( gi.serialize( dude ) )
        print( "Test update..." )
        gi.update( dude )

"""
Test that at some point a female becomes a possible mother.
This test is not exhaustive.
"""
while True:
    if gi.is_possible_mother( girl ):
        print( "Girl became possible mother at: " + str( gi.get_age( girl ) ) )
        break
    gi.update( girl )

"""
Test that expose callback works to infect
This test is not exhaustive.
"""
def expose_cb( action, prob, individual_id ):
    print( "Exposing..." )
    #print( "Immunity = " + str( gi.get_immunity( individual_id ) ) )
    if gi.is_infected( individual_id ) == 0:
        return 1
    return 0

gi.my_set_callback( expose_cb )
for t in range(0,10):
    gi.update( boy )
    print( "Is infected? " + str( gi.is_infected( boy ) ) )


"""
Test that deposit callback works to shed appropriately. Note that with expose callback 
set to infect every time this infection will clear but resume.

This test is not exhaustive.
"""
def deposit_cb( contagion, individual ):
    print( "Deposit callback called with contagion qty {0}.".format( contagion ) )

gi.set_deposit_callback( deposit_cb )
for t in range(0,20):
    gi.update( boy )


"""
Test that mortality callback works to infect. Returns probability of dying this timestep
This test is not exhaustive.
"""
def mort_cb( age, sex ):
    print( "Mortality callback called for individual with age {0} and sex {1}.".format( age, sex ) )
    return 1

gi.set_mortality_callback( mort_cb )
for t in range(0,5):
    gi.update( girl )
    print( "Is dead? " + str( gi.is_dead( girl ) ) )

