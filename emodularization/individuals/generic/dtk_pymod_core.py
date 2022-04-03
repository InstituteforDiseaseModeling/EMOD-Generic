import dtk_nodedemog as nd
import dtk_generic_intrahost as gi
import gc

prevalence = []
active_prevalence = []

def setup_vd_callbacks():
    gi.set_mortality_callback( mortality_callback )
    nd.set_conceive_baby_callback( conceive_baby_callback )
    nd.set_update_preg_callback( update_pregnancy_callback )

def conceive_baby_callback( individual_id, duration ):
    print( "{0} just got pregnant".format( individual_id ) )
    gi.initiate_pregnancy( individual_id );

def update_pregnancy_callback( individual_id, dt ):
    return gi.update_pregnancy( individual_id, int(dt) );

def mortality_callback( age, sex ):
    #print( "Getting mortality rate for age {0} and sex {1}.".format( age, sex ) )
    mortality_rate = nd.get_mortality_rate( ( age, sex ) )
    return mortality_rate

def do_shedding_update( human_pop ):
    for human in human_pop:
        hum_id = human["id"]
        nd.update_node_stats( ( 1.0, 0, gi.is_possible_mother(hum_id), 0 ) ) # mcw, infectiousness, is_poss_mom, is_infected
        """
        if gi.is_infected(hum_id):
            if gi.has_latent_infection(hum_id):
                print( "{0} has latent infection.".format( hum_id ) )
            else:
                print( "{0} has active infection (I guess).".format( hum_id ) )
        else:
            print( "{0} has no infection.".format( hum_id ) )
        """
        gi.update1( hum_id ) # this should do shedding & vital-dynamics

def do_vitaldynamics_update( human_pop, graveyard, census_cb = None, death_cb = None ):
    num_infected = 0
    num_active = 0
    new_graveyard = []
    for human in human_pop:
        hum_id = human["id"]
        if census_cb != None:
            census_cb( hum_id )
        gi.update2( hum_id ) # this should do exposure

        if gi.is_dead( hum_id ):
            # somebody died
            #print( "{0} is dead.".format( hum_id ) )
            new_graveyard.append( human )
            if death_cb != None:
                death_cb( hum_id )

        # TESTING THIS
        ipm = gi.is_possible_mother( hum_id )
        ip = gi.is_pregnant( hum_id )
        if hum_id == 0:
            pdb.set_trace()
        age = gi.get_age( hum_id )
        #print( "Calling cfp with {0}, {1}, {2}, and {3}.".format( str(ipm), str(ip), str(age), str(hum_id) ) )
        nd.consider_for_pregnancy( ( ipm, ip, hum_id, age, 1.0 ) )

        if gi.is_infected( hum_id ):
            num_infected += 1
            #if gi.has_active_infection( hum_id ):
            #    num_active += 1
        # serialize seems to be broken when you had an intervention (or at least a SimpleVaccine)
        #serial_man = gi.serialize( hum_id )
        #if hum_id == 1:
            #print( json.dumps( json.loads( serial_man ), indent=4 ) )
            #print( "infectiousness: " + str( json.loads( serial_man )["individual"]["infectiousness"] ) )
    #print( "Updating fertility for this timestep." )
    for corpse in new_graveyard:
        if corpse in human_pop:
            human_pop.pop( human_pop.index( corpse ) )
        else: 
            print( "Exception trying to remove individual from python list: " + str( ex ) )
    #graveyard.extend( new_graveyard )
    nd.update_fertility()
    prevalence.append( num_infected )
    active_prevalence.append( num_active )
    #gc.collect()
