#!/usr/bin/python

import yaml
import json
import pdb

def application( ignore_path ):
    with open("campaign.niceml", 'r') as stream:
        data_loaded = yaml.load(stream)

    camp_json = json.loads( "{}" )
    camp_json["Use_Defaults"] = 1
    camp_json["Events"] = []
    event = {}

    # boilerplate
    event["class"] = "CampaignEvent"
    event["Nodeset_Config"] = {}
    event["Nodeset_Config"][ "class" ] = "NodeSetAll" # not always, but most of the time.
    event["Event_Coordinator_Config"] = {}
    event["Event_Coordinator_Config"][ "class" ] = "StandardInterventionDistributionEventCoordinator"

    # WHEN
    event["Start_Day"] = data_loaded["when"]["day"]
    # TBD: repetitions

    # WHO
    event["Event_Coordinator_Config"][ "Demographic_Coverage" ] = data_loaded["who"]["fraction"]
    # TBD: age limits

    # WHAT
    intervention = {}
    intervention[ "class" ] = data_loaded["what"]["name"]
    intervention[ "Cost_To_Consumer" ] = data_loaded["what"]["unit_cost"]
    if intervention[ "class" ] == "SimpleVaccine":
        intervention[ "Vaccine_Type" ] = data_loaded["what"]["type"]
    elif intervention[ "class" ] == "Outbreak":
        pass # intervention[ "Vaccine_Type" ] = data_loaded["what"]["type"]

    # HOW LONG
    if intervention[ "class" ] in [ "SimpleVaccine" ]:
        intervention[ "Waning_Config" ] = {}
        intervention[ "Waning_Config" ][ "class" ] = "WaningEffectBox"
        intervention[ "Waning_Config" ][ "Initial_Effect" ] = data_loaded["what"]["efficacy"]
        intervention[ "Waning_Config" ][ "Box_Duration" ] = data_loaded["what"]["duration"]
    event["Event_Coordinator_Config"][ "Intervention_Config" ] = intervention

    camp_json["Events"].append( event )

    with open("campaign.json", 'w') as new_camp:
        new_camp.write( json.dumps( camp_json, sort_keys=True, indent=4 ) )
    return "config.json"
