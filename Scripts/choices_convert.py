#!/usr/bin/python

import json
import sys

camp = open( sys.argv[1] ).read()
camp_json = json.loads( camp )

for event in camp_json["Events"]:
    ic = event["Event_Coordinator_Config"]["Intervention_Config"]
    hivrc = None
    if ic["class"] == "HIVRandomChoice":
        hivrc = ic
    elif ic["class"] == "NodeLevelHealthTriggeredIV" and ic["Actual_IndividualIntervention_Config"]["class"] == "HIVRandomChoice":
        hivrc = ic["Actual_IndividualIntervention_Config"]
    if hivrc != None:
        #print( hivrc["Choices"] )
        event_list = []
        prob_list =  []
        for event in hivrc["Choices"]:
            prob = hivrc[ "Choices" ][ event ]
            event_list.append( event )
            prob_list.append( prob )
            #print( event, str( prob ) )
        hivrc[ "Choices_Events" ] = event_list
        hivrc[ "Choices_Probs" ] = prob_list
        hivrc.pop( "Choices" )
        #{u'NoTrigger': 0.33, u'ARTStaging1': 0.67}


with open( sys.argv[1], "w" ) as camp:
    camp.write( json.dumps( camp_json, sort_keys=True, indent=4 ) )
