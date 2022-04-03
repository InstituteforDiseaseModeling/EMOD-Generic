#!/usr/bin/python
<<<<<<< HEAD

#import regression_utils as ru
import dtk_ep4.dtk_pre_process_adhocevents as adhoc

def application( json_config_path ):
    #ru.flattenConfig( json_config_path )
    #return "config.json"
    return adhoc.application( json_config_path )

