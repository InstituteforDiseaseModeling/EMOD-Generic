from dtk_ep4 import dtk_pre_process_w5ml
import dtk_ep4.dtk_pre_process_adhocevents as adhoc

def application( config_name ):
    dtk_pre_process_w5ml.application( "campaign.w5ml" )
    return adhoc.application( config_name )
