#!/usr/bin/python
import json
import os
import pathlib
import time

"""
This is the pre-processing script to use ad-hoc events in your campaign files. It will 'scrape' your campaign.json for events
that aren't available in the bulit-in list and map them to GP_EVENT_xxx events. It will currently handle up to 100, though 
that can be changed if someone really truly needs more.
Note that this works in conjunction with a post-proc script that maps all the GP_EVENT's back to the user-specified names.
Several things to note about design choices and possible areas of improvement: 
    1) It would obviously be great to get the list of built-in events from the code but currently that would mean parsing a C 
    header file (which relies on a macro). There are various kinds of 'meh' associated with that approach. For now I've 
    copy-pasted the list of actual campaign-related events from the built-in list here. Looking for a great idea for how to
    do this.
    2) There is no perfect way of finding events in the campaign file so I've relied on parsing and our current conventions:
    - _Events, Choice_Names, Event_Trigger, Event_To_Broadcast. Ideally there'd be a hard-and-fast rule.
    3) This code is super-simple except for the recursive march through the campaign file, but that's similar to how we do
    that same thing elsewhere and have been for some time.
"""
adhoc_events = []
builtin_events = [ "StartedART", "StoppedART", "Births", "TBTestPositive", "TBTestNegative", "TBTestDefault", "Blackout" ]
def recursive_json( ref_json, flat_input_json ):
    for val in ref_json:
        try:
            #if not leaf, call recursive_json_leaf_reader
            if isinstance( ref_json[val], dict ): 
                #print( "recursing on nested object: val = " + val )
                recursive_json( ref_json[val], flat_input_json ) 
            elif isinstance( ref_json[val], list ):
                #print( "Iterating over list under val: " + val )
                for obj in ref_json[val]: 
                    # Ignore lists of numbers and strings
                    if isinstance( obj, dict ) or isinstance( obj, list ): 
                        recursive_json( obj, flat_input_json ) 
                    elif "_Events" in val or val == "Choice_Names":
                        for event in ref_json[val]:
                            print( event )
                            if event != "NoTrigger" and event not in adhoc_events and event not in builtin_events and len(event)>0:
                                adhoc_events.append( event )
            else:
                #print( "Considering " + str(val ))
                if( "_Event" in val or "Event_Trigger" in val or val == "Event" or val == "Event_To_Broadcast" ) and "Event_Type" not in val:
                    broadcast_event = ref_json[val]
                    print( broadcast_event  )
                    if broadcast_event not in adhoc_events and broadcast_event not in builtin_events and len(broadcast_event)>0:
                        adhoc_events.append( broadcast_event )
    
                if val not in flat_input_json:
                    flat_input_json[val] = ref_json[val]
                #else:
                    #print( "Ignoring {0} because already present.".format( val ) )
        except Exception as ex:
            print( "Exception processing {0} in {1}. Usually only happens in NChooser.".format( val, ref_json ) )

def application( config ):
    if pathlib.Path( "config_xform.json" ).is_file():
        # This means (in the multcore context) that another 'thread' (core) got here first and is doing the work for us.
        # We know what the filename is going to be.
        # TBD: We should really add code that waits until config_xform.json does not equal "HOLD".
        while pathlib.Path( "config_xform.json" ).stat().st_size <= 5:
            time.sleep( 0.01 )
        return "config_xform.json"

    with open( "config_xform.json", "w" ) as f:
        f.write( "HOLD" )
    
    print( "DTK PRE PROC SCRIPT: Scrape all ad-hoc events and map to GPIO_X." )
    camp_filename = json.loads( open( "config.json" ).read() )["parameters"]["Campaign_Filename"]
    camp_json = json.loads( open( camp_filename ).read() )

    event_map = {}
    for camp_event in camp_json["Events"]:
        output_json = {}
        recursive_json( camp_event, output_json )

    for event in adhoc_events:
        counter = len( event_map )
        builtin = "GP_EVENT_{:03d}".format(counter)
        event_map[ event ] = builtin

    camp_json_str = json.dumps( camp_json )
    for event in event_map:
        camp_json_str = camp_json_str.replace( '"' + event + '"', '"' + event_map[event]  + '"')

    camp_json = json.loads( camp_json_str )

    with open( "campaign_xform.json", "w" ) as camp_json_handle:
        camp_json_handle.write( json.dumps( camp_json, sort_keys=True, indent=4 ) )

    # Do event mapping in config.json
    # 1) Load
    config_json = json.loads( open( "config.json" ).read() )

    # Could do a nifty for loop but obsessing about copy-pasting can sometimes lead to unnecessarily opaque code
    crf = ""
    if "Custom_Reports_Filename" in config_json["parameters"]:
        crf = config_json["parameters"]["Custom_Reports_Filename"]
    if os.path.exists( crf ):
        report_json = json.loads( open( crf ).read() )
        report_json_str = json.dumps( report_json )
        for event in event_map:
            report_json_str = report_json_str.replace( '"' + event + '"', '"' + event_map[event]  + '"')

        report_json = json.loads( report_json_str )

        with open( "custom_reports_xform.json", "w" ) as report_json_handle:
            report_json_handle.write( json.dumps( report_json, sort_keys=True, indent=4 ) )

    # 2) Make changes
    config_json["parameters"]["Campaign_Filename"] = "campaign_xform.json"
    if os.path.exists( crf ):
        config_json["parameters"]["Custom_Reports_Filename"] = "custom_reports_xform.json"
    config_json_str = json.dumps( config_json )
    reverse_map = {}
    for user_name, builtin_name in event_map.items():
        config_json_str = config_json_str.replace( '"' + str(user_name) + '"', '"' + str(builtin_name) + '"' )
        reverse_map[ builtin_name ] = user_name

    # 3) Save
    config_json = json.loads( config_json_str )
    config_json["parameters"]["Event_Map"] = reverse_map
    with open( "config_xform.json", "w" ) as conf_json_handle:
        conf_json_handle.write( json.dumps( config_json, sort_keys=True, indent=4 ) )

    return "config_xform.json"
