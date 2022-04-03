#!/usr/bin/python3.6

import sys
import json
import random
import pdb

from dtk_pymod_core import *
import dtk_nodedemog as nd
import dtk_generic_intrahost as gi
#import dtk_vaccine_intervention as vi
import matplotlib.pyplot as plt


"""
In this app, we want to demonstrate how fertility and mortality are configured in the DTK code.
We will start with a population of 10k (?) of uniform age. We'll let the user set the fertility 
and morality params, or walk them through the options. We will run for 1 year when they hit 'submit' 
and report back the births and/or deaths.
For births, we'll record the time of each birth and render that in a monthly bar chart on its side.
For deaths, we'll do the same, but render the age (and gender?) in a monthly bar chart.
"""

# Initialize module variables.
human_pop = []
nursery = {} # store newborn counter by timestep (dict of int to int for now)
timestep = 0
random.seed( 444 ) # I like the number 4

# constants/parameters
vaccine_disribution_timestep = 200000
outbreak_timestep = 1
outbreak_coverage = 0.02
sim_duration = 365
contagion_bucket = 0

def create_person_callback( mcw, age, gender ):
    """
    This callback is required by dtk_nodedemographic during population initialization.
    It can be named anything as long as it is registered via the appropriate callback.
    It takes 3 parameters: monte-carlo weight, age, and sex.
    """
    # TODO: move some of this to core.
    global human_pop # make sure Python knows to use module-level variable human_pop
    #global timestep
    global nursery
    person = {}
    person["mcw"]=mcw
    person["age"]=age/365
    person["sex"]=gender

    new_id = gi.create( ( gender, age, mcw ) ) 
    person["id"]=new_id
    human_pop.append( person )
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

# INTERESTING TRANSMISSION CODE STARTS HERE

def expose_callback( action, prob, individual_id ):
    """
    This function is the callback for exposure. It is registered with the intrahost module
    and then called for each individual for each timestep. This is where you decide whether 
    an individual should get infected or not. In the limit, you could just return False here 
    and no-one would ever get infected. If you just returned True, everyone would be infected
    always. The expectation is that you write some code that uses the contagion shed and does
    some math. To be heterogeneous, you can use the individual id. The action and prob 
    parameters can be ignored.
    """

    # The following code is just to demo the use of TBHIV-specific getters
    if gi.is_infected( individual_id ): 
        #print( "Individual {0} is apparently already infected.".format( individual_id ) )
        return 0


    global timestep
    #print( "timestep = {0}, outbreak_timestep = {1}.".format( timestep, outbreak_timestep ) )
    if timestep == outbreak_timestep:
        #if gi.get_immunity( individual_id ) == 1.0 and random.random() < 0.1: # let's infect some people at random (outbreaks)
        if random.random() < outbreak_coverage: # let's infect some people at random (outbreaks)
            print( "Let's force-infect (outbreak) uninfected, non-immune individual based on random draw." )
            return 1
    else:
        if individual_id == 0:
            pdb.set_trace()

        global contagion_bucket 

        #print( "Exposing individual {0} to contagion {1}.".format( individual_id, contagion_bucket ) )
        if gi.should_infect( ( individual_id, contagion_bucket ) ):
            return 1

    return 0

def deposit_callback( contagion, individual_id ):
    """
    This function is the callback for shedding. It is registered with the intrahost module
    and then called for each shedding individual for each timestep. This is where you collect
    contagion that you want to subsequently use for transmission. Note that the individual 
    value is only populated with non-zero values if pymod-specific SetGeneticID code is 
    turned on in the DTK. This is only example I can think of of pymod-specific code in DTK.
    """
    #print( "{0} depositing {1} contagion creates total of {2}.".format( individual, contagion, well_mixed_contagion_pool ) )
    #print( "{0} depositing {1} contagion.".format( individual, contagion ) )
    # Figure out what age bucket the individual is in.
    #if individual_id == 0: # we need the DTK code to be built with the GeneticID used for the individual id. If not, this will be 0
        #pdb.set_trace() 
    
    #print( "Shedding {0} into age-clan index {1}.".format( contagion, index ) )
    #age_of_infection = gi.get_infection_age( individual_id )
    #contagion = get_infectiousness( age_of_infection )
    global contagion_bucket 
    contagion_bucket += contagion

# INTERESTING TRANSMISSION CODE ENDS HERE 

def get_infectiousness( age_of_infection ):
    """
    The get_infectiousness function is an optional demo function that lets you create customized infectiousness profiles.
    The input parameter is the age of infection (for an individual) and it returns a floating point value.
    """
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

# 
# INTERVENTION HELPERS
# 

def distribute_interventions( t ):
    """
    Function to isolated distribution of interventions to individuals.
    Interventions are separate python modules. 
    """
    if t == vaccine_disribution_timestep:
        for human in human_pop:
            hum_id = human["id"] 

            # Below is code to give out anti-tb drugs
            #individual_ptr = gi.get_individual( hum_id )
            #print( "Giving anti-tb drug to {0}.".format( hum_id ) ) 
            #tdi.distribute( individual_ptr )

            # Below is code to giveout ART via function that resets ART timers
            give_art( hum_id )

            #Below is code to give out vaccines; this should be updated to use the distribute method
            #print( "Giving simple vaccine to {0}.".format( hum_id ) )
            #vaccine = vi.get_intervention()
            #gi.give_intervention( ( hum_id, vaccine ) )

    # For trivial demo, remove ART after two years.
    elif t == vaccine_disribution_timestep+730:
        for human in human_pop:
            hum_id = human["id"] 
            remove_art( hum_id )


def setup_callbacks():
    """
    The setup_callbacks function tells the PyMod modules which functions (callbacks or delegates)
    to invoke for vital dynamics and transmission.
    """
    # set creation callback
    nd.set_callback( create_person_callback )
    # set vital dynamics callbacks
    gi.my_set_callback( expose_callback )
    gi.set_deposit_callback( deposit_callback )
    setup_vd_callbacks()

def run( from_script = False ):
    """
    This is the main function that actually runs the demo, as one might expect. It does the folllowing:
    - Register callbacks
    - Create human population
    - Foreach timestep:
        - Do shedding loop for each individual
        - Calculate adjusted force of infection
        - Do vital dynamics & exposure update for each individual
        - Distribute interventions
        - "Migration"
    - Plot simple reports summarizing results
    """
    global human_pop # make sure Python knows to use module-level variable human_pop
    del human_pop[:]

    setup_callbacks()
    nd.populate_from_files() 
    if len(human_pop) == 0:
        print( "Failed to create any people in populate_from_files. This isn't going to work!" )
        sys.exit(0)

    graveyard = []
    global timestep
    global contagion_bucket 
    for t in range(0,sim_duration): # for one year
        timestep = t

        do_shedding_update( human_pop )
        contagion_bucket /= len(human_pop)
        do_vitaldynamics_update( human_pop, graveyard ) 
        distribute_interventions( t )

        print( "At end of timestep {0} num_infected = {1}.".format( t, prevalence[-1] ) )

    # Sim done: Report.
    plt.plot( prevalence )
    plt.title( "Infections" )
    plt.xlabel( "time" )
    plt.ylabel( "infected individuals" )
    plt.show()

    if from_script:
        print( "NURSERY\n" + json.dumps( nursery, indent=4 ) )
        print( "GRAVEYARD\n" + json.dumps( graveyard, indent=4  ) )
    return graveyard

if __name__ == "__main__": 
    #test_cart_prod()
    run()
