#!/usr/bin/python3.6

import sys
import random
sys.path.append( "./build/lib.linux-x86_64-3.7" )
import dtk_hiv_intrahost as gi
import matplotlib.pyplot as plt

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
            print( "Test get_cd4count..." )
            print( gi.get_cd4( dude ) )
            print( "Test has_hiv..." )
            print( gi.has_hiv( dude ) )
        print( "Test serialize..." )
        print( gi.serialize( dude ) )
        print( "Test update..." )
        gi.update( dude )

"""
Test that at some point a female becomes a possible mother.
This test is not exhauhivve.
"""
while True:
    if gi.is_possible_mother( girl ):
        print( "Girl became possible mother at: " + str( gi.get_age( girl ) ) )
        break
    gi.update( girl )

"""
Test that expose callback works to infect
This test is not exhauhivve.
"""
def expose_cb( action, prob, individual_id ):
    print( "Exposing..." )
    #print( "Immunity = " + str( gi.get_immunity( individual_id ) ) )
    if gi.is_infected( individual_id ) == 0:
        return 1
    return 0

# gi.my_set_callback( expose_cb )

import dtk_artbasic_iv as art
art_people = []
def infect_and_watch_die( person ):
    cd4s = []
    gi.force_infect( person )
    #for t in range(0,365):
    updates = 0
    while gi.is_dead( person ) == False and updates < 10000:
        gi.update( person )
        is_infected = gi.is_infected( person )
        cd4 = gi.get_cd4( person )
        if cd4 < 100 and person not in art_people:
            #retro = art.get_intervention()
            #gi.give_intervention( ( person, retro ) )
            individual_ptr = gi.get_individual_for_iv( person )
            art.distribute( individual_ptr )
            art_people.append( individual_ptr )
        cd4s.append( cd4 )
        is_dead = gi.is_dead( person )
        updates += 1
        print( "Is infected? {0} CD4 {1}, Dead? {2}.".format( is_infected, cd4, is_dead  ) )
    return cd4s

for _ in range(100): # Test 100 separate people
    boy = gi.create( ( 0, random.uniform( 0, 365*100 ), 1.0 ) )
    cd4s = infect_and_watch_die( boy )
    plt.plot( cd4s )

plt.show()

"""
Test that deposit callback works to shed appropriately. Note that with expose callback 
set to infect every time this infection will clear but resume.

This test is not exhauhivve.
"""
def deposit_cb( contagion, individual ):
    print( "Deposit callback called with contagion qty {0}.".format( contagion ) )

gi.set_deposit_callback( deposit_cb )
for t in range(0,20):
    gi.update( boy )


"""
Test that mortality callback works to infect. Returns probability of dying this timestep
This test is not exhauhivve.
"""
def mort_cb( age, sex ):
    print( "Mortality callback called for individual with age {0} and sex {1}.".format( age, sex ) )
    return 1

gi.set_mortality_callback( mort_cb )
for t in range(0,5):
    gi.update( girl )
    print( "Is dead? " + str( gi.is_dead( girl ) ) )

