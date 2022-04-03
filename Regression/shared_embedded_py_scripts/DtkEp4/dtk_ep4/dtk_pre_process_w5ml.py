#!/usr/bin/python

import yaml
import json
import sys
import copy
import pdb

def application( filename ): 
    # This is fragile but let's try it.
    try:
        ei = json.loads( open( "config.json" ).read() )["parameters"]["Enable_Interventions"]
        if ei == 0:
            return

    except Exception as ex:
        print( "Attempt to get Enable_Interventions from config.json failed." )

    camp_skel = yaml.load( open( filename ).read() ) 
    event_template = """
            {
                "Event_Coordinator_Config": {
                    "Demographic_Coverage": 1.0,
                    "Intervention_Config": {
                        "class": "OutbreakIndividual"
                    },
                    "Target_Demographic": "Everyone",
                    "class": "StandardInterventionDistributionEventCoordinator"
                },
                "Event_Name": "TBD",
                "Nodeset_Config": {
                    "class": "NodeSetAll"
                }
            }

    """
    events = []
    event_counter = 1

    for event_w5 in camp_skel["Events"]:
        new_event = json.loads( copy.deepcopy( event_template ) )
        new_event[ "Event_Name" ] = "Event #" + str( event_counter )
        event_counter += 1

        who = event_w5["Who"]
        whos = who.split( ',' )
        #if len(whos) != 3:
            #print( "'Who' should be comma-separated version of 'X%, A-B, Male'." )
            #sys.exit()
        coverage = float(whos[0].rstrip( '%' ))/100
        new_event["Event_Coordinator_Config"]["Demographic_Coverage"] = coverage
        # TBD: if coverage = 1.0, just omit for defaults?
        ages = whos[1].strip()
        if ages != "AllAges":
            age_lims = ages.split( '-' )
            min_age = float(age_lims[0])
            max_age = float(age_lims[1])
            # TBD: Check for valid values
            new_event["Event_Coordinator_Config"]["Target_Demographic"] = "ExplicitAgeRanges"
            new_event["Event_Coordinator_Config"]["Target_Age_Min"] = min_age
            new_event["Event_Coordinator_Config"]["Target_Age_Max"] = max_age
        sex = whos[2].strip()
        if sex != "BothSexes":
            # TBD: Check for valid values
            new_event["Event_Coordinator_Config"]["Target_Demographic"] = "ExplicitAgeRangesAndGender"
            new_event["Event_Coordinator_Config"]["Target_Gender"] = sex
        if len(whos) >= 4:
            ips = whos[3].strip()
            new_event["Event_Coordinator_Config"]["Property_Restrictions"] = []
            new_event["Event_Coordinator_Config"]["Property_Restrictions"].append( ips )

        where = event_w5["Where"]
        if where != "Everywhere":
            print( "TBD: Need to implement alternatives to NodeSetAll." )

        when = event_w5["When"]
        # Convert 'when' into Start_Day or Start_Year
        whens = [ when ]
        if isinstance( when, str ) and "," in when:
            whens = when.split( ',' )
        start = float(whens[0])
        if( start % 1.0 == 0.0 ):
            # StartTime
            new_event[ "class" ] = "CampaignEvent"
            new_event[ "Start_Day" ] = int(start)
        else: # float
            # StartYear
            new_event[ "class" ] = "CampaignEventByYear"
            new_event[ "Start_Year" ] = start

        other_whens = []
        # now we support two kinds of mutiples: repetitions and 'laundry-list'
        if len(whens) == 2 and '*' in whens[1]:
            reps = whens[1].split("*")
            new_event["Event_Coordinator_Config"]["Timesteps_Between_Repetitions"] = int(reps[0])
            new_event["Event_Coordinator_Config"]["Number_Repetitions"] = int(reps[1])
        else:
            other_whens = whens[1:]


        what = event_w5["What"]
        iv_class = what["class"]
        #new_event["Event_Coordinator_Config"]["Intervention_Config"]["class"] = iv_class
        new_iv = new_event["Event_Coordinator_Config"]["Intervention_Config"]
        new_iv["class"] = iv_class
        # maybe just copy 'payload' kvp's verbatim except for nasty special cases.
        if iv_class == "ScaleLarvalHabitat":
            habitat = what["Habitat"]
            level = float(what["Level"])
            lhm = {}
            lhm[ habitat ] = level 
            new_iv["Larval_Habitat_Multiplier"] = lhm
        else:
            for key in what.keys():
                if key == "Efficacy_Profile":
                    new_iv[ "Waning_Config" ] = {}
                    wanings = what[ "Efficacy_Profile" ].split( ',' )
                    init = float(wanings[1])
                    dur = int(wanings[2])
                    new_iv[ "Waning_Config" ][ "class" ] = "WaningEffect" + wanings[0]
                    new_iv[ "Waning_Config" ][ "Initial_Effect" ] = init
                    if wanings[ 0 ] == "Box":
                        new_iv[ "Waning_Config" ][ "Box_Duration" ] = dur
                else:
                    new_iv[ key ] = what[key]

        #why = event_w5["Why"] # for events

        events.append( new_event )
        if len( other_whens ) > 0:
            for when in other_whens:
                new_event = copy.deepcopy( new_event )
                if "Start_Day" in new_event:
                    new_event["Start_Day"] = float(when)
                elif "Start_Year" in new_event:
                    new_event["Start_Year"] = float(when)
                else:
                    print( "Error. Didn't find Start_Day or Start_Year in new_event." )
                events.append( new_event )

    top_level = {}
    top_level["Use_Defaults"] = 1
    top_level["Events"] = events
    #print( json.dumps( top_level,sort_keys=True,indent=4 ))
    with open( "campaign.json", "w" ) as camp_file:
        #camp_file.write( str( json.dumps( top_level,sort_keys=True,indent=4 ) ) )
        json.dump( top_level, camp_file )

    config_json = json.loads( open( "config.json" ).read() )
    config_json["parameters"]["Campaign_Filename"] = "campaign.json"
    with open( "config.json", "w" ) as config_file:
        json.dump( config_json, config_file )

if __name__ == "__main__":
    application( sys.argv[1] )
