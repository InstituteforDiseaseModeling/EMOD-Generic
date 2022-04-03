from dtk_ep4 import dtk_pre_process_w5ml
import json

def application( config_name ):
    camp_filename = json.loads( open( config_name ).read() )[ "parameters" ][ "Campaign_Filename" ]
    if camp_filename.endswith( ".w5ml" ):
        dtk_pre_process_w5ml.application( camp_filename )
