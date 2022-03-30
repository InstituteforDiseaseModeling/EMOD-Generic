#!/usr/bin/python3.6

import sys
import os
import syslog
import json
import random
import string
import math
import datetime
import time
import pdb

import dtk_nodedemog as nd
import dtk_generic_intrahost as gi
import dtk_vaccine_intervention as iv
import scipy.stats
import numpy 


"""
In this app, we want to demonstrate how fertility and mortality are configured in the DTK code.
We will start with a population of 10k (?) of uniform age. We'll let the user set the fertility 
and morality params, or walk them through the options. We will run for 1 year when they hit 'submit' 
and report back the births and/or deaths.
For births, we'll record the time of each birth and render that in a monthly bar chart on its side.
For deaths, we'll do the same, but render the age (and gender?) in a monthly bar chart.
"""

iv_timestep = 365
human_pop = []
nursery = {} # store newborn counter by timestep (dict of int to int for now)
timestep = 0
foi = 0

def create_person_callback( mcw, age, gender ):
    global human_pop # make sure Python knows to use module-level variable human_pop
    global timestep
    global nursery
    person = {}
    person["mcw"]=mcw
    person["age"]=age/365
    person["sex"]=gender

    #syslog.syslog( syslog.LOG_INFO, "[PY] Creating human with age = {0}, gender = {1}.".format( age, gender ) )
    print( syslog.LOG_INFO, "[PY] Creating human with age = {0}, gender = {1}.".format( age, gender ) )
    new_id = gi.create( ( gender, age, mcw ) ) 
    person["id"]=new_id
    human_pop.append( person )
    syslog.syslog( syslog.LOG_INFO, "Human population now = {0}.".format( len( human_pop ) ) )
    if age == 0:
        month_step = int( timestep/30.0 )
        print( "Made a baby on timestep {0}/month {1}.".format( str( timestep ), str( month_step ) ) )
        if month_step not in nursery:
            nursery[month_step] = ( 0, 0 )
        boys = nursery[month_step][0]
        girls = nursery[month_step][1]
        if gender == 0:
            boys += mcw
        else:
            girls += mcw 
        nursery[month_step] = ( boys, girls )

def conceive_baby_callback( individual_id, duration ):
    print( "{0} just got pregnant".format( individual_id ) )
    gi.initiate_pregnancy( individual_id );


def update_pregnancy_callback( individual_id, dt ):
    print( "update_pregnancy_callback" )
    return gi.update_pregnancy( individual_id, int(dt) );


def mortality_callback( age, sex ):
    #print( "Getting mortality rate for age {0} and sex {1}.".format( age, sex ) )
    mortality_rate = nd.get_mortality_rate( ( age, sex ) )
    return mortality_rate


# INTERESTING TRANSMISSION CODE STARTS HERE

sp_stat_ep_cdf_cache = {}
def expose( action, prob, individual_id ):

    if gi.is_infected( individual_id ):
        return 0 

    global timestep
    if timestep == 0:
        #if gi.get_immunity( individual_id ) == 1.0 and random.random() < 0.1: # let's infect some people at random (outbreaks)
        if random.random() < 0.01: # let's infect some people at random (outbreaks)
            print( "Let's force-infect (outbreak) uninfected, non-immune individual based on random draw." )
            return 1
    else:
        if foi > 0:
            #print( "Exposing individual {0} to contagion {1}.".format( individual_id, foi ) )
            effective_foi = round( foi * gi.get_immunity( individual_id ), 2 )
            prob = 0
            if effective_foi in sp_stat_ep_cdf_cache:
                prob =  sp_stat_ep_cdf_cache[ effective_foi ]
            else:
                prob = scipy.stats.expon.cdf( effective_foi ) # THIS CALL IS REALLY SLOW???!!! :(
                sp_stat_ep_cdf_cache[ effective_foi ] = prob
            if random.random() < prob:
                #print( "Let's infect based on transmission: immunity = {0}.".format( gi.get_immunity( individual_id ) ) )
                return 1
    return 0

def deposit( contagion, individual ):
    #print( "{0} depositing {1} contagion creates total of {2}.".format( individual, contagion, well_mixed_contagion_pool ) )
    global foi
    foi = contagion

# INTERESTING TRANSMISSION CODE ENDS HERE 

def move_people_around():
    
    return

def get_infectiousness( age_of_infection ):
    # implement some simple function that changes infectiousness as a function of infection age.
    # Going with a linear ramp-up and ramp-down thing here for demo
    # you can do whatever functional form you want here. 
    inf = 0
    if age_of_infection < 30:
        inf = 0.1 * (age_of_infection)/30.
    elif age_of_infection >= 30 and age_of_infection < 60:
        inf = 0.1 * (60-age_of_infection)/30.
    #print( "inf = {0}.".format( inf ) )
    return inf;

def run( from_script = False ):
    global human_pop # make sure Python knows to use module-level variable human_pop
    del human_pop[:]
    syslog.syslog( syslog.LOG_INFO, "We cleared out human_pop. Should get populated via populate_from_files and callback..." )

    # set creation callback
    nd.set_callback( create_person_callback )
    nd.populate_from_files() 

    # set vital dynamics callbacks
    gi.set_mortality_callback( mortality_callback )
    gi.my_set_callback( expose )
    gi.set_deposit_callback( deposit )
    nd.set_conceive_baby_callback( conceive_baby_callback )
    nd.set_update_preg_callback( update_pregnancy_callback )

    syslog.syslog( syslog.LOG_INFO, "Update fertility & mortality of population of size {0} for 1 year.".format( len( human_pop ) ) )
    graveyard = []
    for t in range(0,2*365): # for one year
        global timestep
        timestep = t
        #syslog.syslog( syslog.LOG_INFO, "Updating individuals at timestep {0}.".format( t ) )
        #nd.reset_counters()

        num_infected = 0
        for human in human_pop:
            hum_id = human["id"]
            #syslog.syslog( syslog.LOG_INFO, "Updating individual {0} at timestep {1}.".format( hum_id, t ) )
            nd.update_node_stats( ( 1.0, 0, gi.is_possible_mother(hum_id), 0 ) ) # mcw, infectiousness, is_poss_mom, is_infected
            gi.update1( hum_id ) # this should do shedding & vital-dynamics
            if gi.is_infected( hum_id ):
                num_infected += 1
        print( "At end of timestep {0} population = {1}, num_infected = {2}.".format( t, len(human_pop), num_infected ) )

        if t == iv_timestep:
            for human in human_pop:
                hum_id = human["id"]
                vaccine = iv.get_intervention()
                gi.give_intervention( ( hum_id, vaccine ) )

        syslog.syslog( syslog.LOG_INFO, "Updating individuals (exposing) at timestep {0}.".format( t ) )
        for human in human_pop:
            hum_id = human["id"]
            gi.update2( hum_id ) # this should do exposure

            if gi.is_dead( hum_id ):
                syslog.syslog( syslog.LOG_INFO, "Individual {0} died.".format( hum_id ) )
                graveyard.append( human )
                try:
                    human_pop.pop( hum_id )
                except Exception as ex:
                    syslog.syslog( syslog.LOG_INFO, "Exception trying to remove individual from python list: " + str( ex ) )

            # TESTING THIS
            ipm = gi.is_possible_mother( hum_id )
            ip = gi.is_pregnant( hum_id )
            if hum_id == 0:
                pdb.set_trace()
            age = gi.get_age( hum_id )
            #print( "Calling cfp with {0}, {1}, {2}, and {3}.".format( str(ipm), str(ip), str(age), str(hum_id) ) )
            nd.consider_for_pregnancy( ( ipm, ip, hum_id, age, 1.0 ) )

            #serial_man = gi.serialize( hum_id )
            #if hum_id == 1:
                #print( json.dumps( json.loads( serial_man ), indent=4 ) )
                #print( "infectiousness: " + str( json.loads( serial_man )["individual"]["infectiousness"] ) )
        #print( "Updating fertility for this timestep." )
        nd.update_fertility()

        move_people_around()

    if from_script:
        print( "NURSERY\n" + json.dumps( nursery, indent=4 ) )
        print( "GRAVEYARD\n" + json.dumps( graveyard, indent=4  ) )
    return graveyard

if __name__ == "__main__": 
    #test_cart_prod()
    run()
