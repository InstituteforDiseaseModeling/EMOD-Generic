#!/usr/bin/python

from dtk_utils import regression as reg
import dtk_ep4.dtk_pre_process_adhocevents as adhoc

def application( json_config_path ):
    new_name = "config"
    reg.flattenConfig( json_config_path, new_name )
    new_name += ".json"
    return adhoc.application( new_name )
