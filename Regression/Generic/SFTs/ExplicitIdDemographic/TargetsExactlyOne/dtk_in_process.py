#!/usr/bin/python

import json
import sqlite3
import sys

def application( timestep ):
    timestep = int(float(timestep))
    params = None
    print(f"Hello from timestep {timestep}")
    if timestep < 6:
        return ""

    infections = []
    infector_ids = []
    individual_ids = []
    with sqlite3.connect('simulation_events.db') as conn:
        cur = conn.cursor()
        query = "SELECT SIM_TIME, INDIVIDUAL, LABEL, IPS from SIM_EVENTS where EVENT=='NewInfection';"
        cur.execute(query)
        rows = cur.fetchall()
        for row in rows:
            time= row[0]
            individual = row[1]
            infector = row[2]
            ip_info = row[3]
            infection = {"Time": time, "Individual": individual, "Infector": infector, "IP": ip_info}
            if infector > 0 and infector not in infector_ids:
                infector_ids.append(infector)
                pass
            if time == timestep -1:
                individual_ids.append(individual)
                infections.append(infection)
            pass
        pass

    event = {}
    event = json.loads(
        """
        {
            "Start_Day": null,
            "class": "CampaignEvent",
            "Event_Coordinator_Config": {
                "Intervention_Config": {
                    "class": "Vaccine",
                    "Cost_To_Consumer": 1.0,
                    "Dont_Allow_Duplicates": 1,
                    "Vaccine_Type": "TransmissionBlocking",
                    "Waning_Config": {
                        "class": "WaningEffectBox",
                        "Initial_Effect": 1.0,
                        "Box_Duration": 14
                    }
                },
                "Target_Demographic": "ExplicitIDs",
                "ID_List": [],
                "Max_Cases_Per_Node": -1,
                "class": "StandardInterventionDistributionEventCoordinator"
            },
            "Nodeset_Config": {
                "class": "NodeSetAll"
            }
        }
        """
    )

    if len(individual_ids) > 0:
        campaign = {}
        campaign["Use_Defaults"] = 1
        campaign["Events"] = []
        event["Start_Day"] = float(timestep+1)
        event["Event_Coordinator_Config"]["ID_List"] = individual_ids
        campaign["Events"].append(event)
        with open( "generated_campaign.json", "w" ) as camp_file:
            json.dump( campaign, camp_file )
            pass
        return "generated_campaign.json"
    else:
        print("No infector IDs")
        return ""
