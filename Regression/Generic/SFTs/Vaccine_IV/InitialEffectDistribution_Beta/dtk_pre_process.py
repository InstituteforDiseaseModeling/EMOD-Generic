#!/usr/bin/python

import json
import dtk_post_process
import random

def application( json_config_path):
    alpha = random.randint(0, 50)
    beta = random.randint(0, 50)
    dtk_post_process.alpha = alpha
    dtk_post_process.beta = beta
    campaign_filename = "campaign.json"
    with open(campaign_filename, 'r') as infile:
        campaign = json.load(infile)
    if campaign["Events"][0]["Event_Coordinator_Config"]["Intervention_Config"]["class"] == "Vaccine":
        update_distribution = campaign["Events"][0]["Event_Coordinator_Config"]["Intervention_Config"]
    else:
        update_distribution = campaign["Events"][1]["Event_Coordinator_Config"]["Intervention_Config"]
    update_distribution["Acquire_Config"]["Initial_Effect_Distribution_Alpha"] = alpha
    update_distribution["Acquire_Config"]["Initial_Effect_Distribution_Beta"] = beta
    with open(campaign_filename, 'w') as outfile:
        json.dump(campaign, outfile, indent=4, sort_keys=True)
        pass

    return json_config_path
