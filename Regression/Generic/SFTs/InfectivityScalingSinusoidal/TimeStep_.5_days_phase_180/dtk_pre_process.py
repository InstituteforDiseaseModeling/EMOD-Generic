from dtk_ep4 import dtk_pre_process_w5ml
import json
import pdb

def application( config_name ):
    dtk_pre_process_w5ml.application( "campaign.w5ml" )
    #test_json = json.loads( open( "campaign.json" ).read() )
    #with open( "campaign.json", "w" ) as camp:
        #camp.write( json.dumps( test_json ) )
