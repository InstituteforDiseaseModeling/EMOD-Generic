from dtk_ep4 import dtk_pre_process_w5ml
import json
import os

def application( config_name ):
    config_json = json.loads( open( config_name ).read() )
    camp_filename = config_json["parameters"]["Campaign_Filename"]

    #if camp_filename == "campaign.json" and os.path.exists( "campaign.json" ):
    #    return
    #print( "Calling W5ML preprocessor on " + camp_filename  )

    dtk_pre_process_w5ml.application( camp_filename )
