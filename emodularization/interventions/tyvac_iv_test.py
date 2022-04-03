#!/usr/bin/python

import json
import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_tyvac as tyvac

def report_apply( action, prob ):
    print( "Update probability of " + action + " with value " + str( prob ) )

tyvac.my_set_callback( report_apply )

def get_schema():
    schema = tyvac.get_schema()
    print( schema )

def test_contact_shedding():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Shedding"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_CONTACT"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute() 
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[0]:
        print( "Failed." )
    else:
        print( "Pass." )

def test_contact_dose():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Dose"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_CONTACT"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute()
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[1]:
        print( "Failed." )
    else:
        print( "Pass." )

def test_contact_num():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Exposures"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_CONTACT"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute()
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[2]:
        print( "Failed." )
    else:
        print( "Pass." )

def test_enviro_shedding():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Shedding"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_ENVIRONMENTAL"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute() 
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[3]:
        print( "Failed." )
    else:
        print( "Pass." )

def test_enviro_dose():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Dose"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_ENVIRONMENTAL"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute()
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[4]:
        print( "Failed." )
    else:
        print( "Pass." )

def test_enviro_num():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Exposures"
    tv_json[ "Route" ] = "TRANSMISSIONROUTE_ENVIRONMENTAL"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute()
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - mods[5]:
        print( "Failed." )
    else:
        print( "Pass." )


# batch stuff
def test_batch():
    tyvac.create_batch( 10 )
    tyvac.distribute_batch()
    for i in range( 4000 ):
        tyvac.update_batch( 1 )

#test_batch()
test_contact_shedding()
test_contact_dose()
test_contact_num()
test_enviro_shedding()
test_enviro_dose()
test_enviro_num()
