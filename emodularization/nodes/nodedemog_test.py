#!/usr/bin/python

import pdb
import sys
import dtk_nodedemog as nd
import dtk_generic_intrahost as gi

individuals = []
def create_person_callback( mcw, age, gender ):
    print( "Created individual with mcw={0}, age={1}, gender={2}.".format( mcw, age, gender ) )
    individuals.append( gi.create( ( gender, age, mcw ) ) )

nd.set_callback( create_person_callback )

def conceive_baby_callback( individual_id, duration ):
    print( "{0} just got pregnant".format( individual_id ) )
    #pdb.set_trace()
    if individual_id not in individuals:
        print( "Yikes! {0} is supposed to get pregnant, but she's not in our population!".format( individual_id ) )
    else:
        #print( str( gi.is_dead( individual_id ) ) );
        gi.initiate_pregnancy( individual_id );

nd.set_conceive_baby_callback( conceive_baby_callback )

def update_pregnancy_callback( individual_id, dt ):
    if individual_id not in individuals:
        print( "Yikes! {0} not in our population!".format( individual_id ) )
    else:
        #print( "{0} updating pregnancy (dt = {1}).".format( individual_id, dt ) )
        return gi.update_pregnancy( individual_id, int(dt) );

nd.set_update_preg_callback( update_pregnancy_callback )

def expose( action, prob ):
    #print( "expose: " + action + " with value " + str( prob ) )
    if False:
        if gi.get_immunity() < 1:
            print( "Let's infect based on random draw." )
            return 1
        else:
            return 0
    else:
        #print( "Let's NOT infect based on random draw." )
        return 0

def mortality_callback( age, sex, dt ):
    mortality_rate = nd.get_mortality_rate( ( age, sex, dt ) )
    #print( "mortality_callback returning " + str( mortality_rate ) )
    return mortality_rate

gi.my_set_callback( expose )
gi.set_mortality_callback( mortality_callback )

try:
    nd.populate_from_files() # Can equivalently pass nd.json or test error handling with something like "nnnnnnodedemog.json"
except Exception as ex:
    print( "Exception: " + str(ex) )
    sys.exit(0)
#print( dtk_nodedemog.get_schema() )

#pdb.set_trace()
for t in range(0,3650):
    nd.update_fertility()
    #print( "There are {0} individuals in the population right now.".format( str( len( individuals ) ) ) )
    for ind in individuals:
        #print( "Updating individual, age = {0}, infected = {1}, immunity = {2}.".format( gi.get_age(), gi.is_infected(), gi.get_immunity() ) )
        #print( "Updating individual " + str( ind ) )
        gi.update( ind )
        # check for dead
        if gi.is_dead( ind ):
            print( "Individual {0} died.".format( ind ) )
            try:
                individuals.pop( ind )
            except Exception as ex:
                print( "Exception trying to remove individual from python list: " + str( ex ) )

        ipm = gi.is_possible_mother( ind )
        ip = gi.is_pregnant( ind )
        age = gi.get_age( ind )
        #print( "Calling cfp with {0}, {1}, {2}, and {3}.".format( ipm, ip, age, ind ) )
        nd.consider_for_pregnancy( ( ipm, ip, ind, age, 1.0 ) ) 
