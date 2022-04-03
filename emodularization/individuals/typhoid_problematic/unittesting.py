#!/usr/bin/python

import json
import dtk_typhoidindividual as ti
import random
import pdb
import unittest
import ast

def expose( action, prob ):
    print( "expose: " + action + " with value " + str( prob ) ) 
    if ti.is_infected() == 0 and random.random() < TestStringMethods.prob_exposure:
        if ti.get_immunity() == 1:
            print( "Individual is immune. Don't infect." )
            return 0
        return 1
    else:
        print( "Let's NOT infect based on current infection or random draw." )
        return 0

ti.my_set_callback( expose )

def getSerializedIndividualAsJson( man ):
    the_man = json.loads( str( man.serialize() ) ) 
    #print( the_man )
    return the_man

class TestStringMethods(unittest.TestCase):
    prob_exposure  = 0.0
    config_json = json.loads( open( "ti.json" ).read() )

    def __init__(self, *args, **kwargs):
        super(TestStringMethods, self).__init__(*args, **kwargs)

    def setUp(self):
        pass
        #self.mean = 10.0

    @unittest.skip('')
    def test_uninfected_at_start(self):
        print( "\nRunning test to confirm that newly created individuals is not infected.\n" )
        ti.create( 30, 'M' )
        serial_man_json = json.loads( str( ti.serialize() ) )
        num_infections = len(serial_man_json[ "individual" ]["infections"] ) 
        self.assertEqual( num_infections, 0 )

    def test_infected_after_one(self):
        #self.assertTrue('FOO'.isupper())
        #self.assertFalse('Foo'.isupper())
        
        print( "\nRunning test to confirm that newly infected individual IS infected across all ages and both sexes. But newborns have immunity.\n" )
        TestStringMethods.prob_exposure  = 1.0
        for sex in [ 'M', 'F' ]:
            for age in xrange( 125 ):
                ti.create( age, sex )
                immunity = ti.get_immunity()
                ti.update()
                serial_man = getSerializedIndividualAsJson( ti )
                num_infections = len(serial_man[ "individual" ]["infections"] ) 
                state = serial_man[ "individual" ]["state_to_report"]
                if immunity < 1.0:
                    self.assertEqual( num_infections, 1 )
                    self.assertEqual( state, "PRE" )
                else:
                    self.assertEqual( num_infections, 0 )
                    self.assertEqual( state, "SUS" )

    @unittest.skip('')
    def test_prepatent(self):
        """
        Requirements: 
        1) Stay in prepatent until timer expires.
        2) Leave prepat after timer expires.
        3) Shed at appropriate rate for prepatent.
        4) Transition to Acute or Sub afterwards.
        """
        print( "\nRunning test to confirm that prepatent stage operates correctly.\n" )
        ti.create( 30, 'M' )
        TestStringMethods.prob_exposure  = 0.0
        # Just get things into a non-initial state (but no infection)...
        for t in xrange( 365 ):
            ti.update()
        state = json.loads( str( ti.serialize() ) )[ "individual" ]["state_to_report"]
        self.assertEqual( state, "SUS" )

        # Now infect them
        TestStringMethods.prob_exposure  = 1.0
        ti.update()
        TestStringMethods.prob_exposure  = 0.0

        serial_man_json = json.loads( str( ti.serialize() ) )
        #print( json.dumps( serial_man_json, indent=4, sort_keys=True ) )
        pp_inf_theory = TestStringMethods.config_json[ "Typhoid_Acute_Infectiousness" ] * TestStringMethods.config_json[ "Typhoid_Prepatent_Relative_Infectiousness" ]
        pp_dur = prepat_init_duration = serial_man_json[ "individual" ][ "infections" ][0][ "prepatent_duration" ]
        for step in xrange( pp_dur ):
            ti.update()
            serial_man = json.loads( str( ti.serialize() ) )
            state = serial_man[ "individual" ]["state_to_report"]
            self.assertEqual( state, "PRE" )
            inf   = serial_man[ "individual" ]["infectiousness"]
            self.assertEqual( inf, pp_inf_theory )
        ti.update()
        serial_man = json.loads( str( ti.serialize() ) )
        state = serial_man[ "individual" ]["state_to_report"]
        self.assertNotEqual( state, "PRE" )


if __name__ == '__main__':
    unittest.main()

