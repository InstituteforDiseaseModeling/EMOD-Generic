#!/usr/bin/python
import json
import pdb

adhoc_events = []
def recursive_json( ref_json, flat_input_json ):
    for val in ref_json:
        #if not leaf, call recursive_json_leaf_reader
        #if json.dumps( ref_json[val] ).startswith( "{" ) or json.dumps( ref_json[val] ).startswith( "[" ): # change to isinstance
        #if json.dumps( ref_json[val] ).startswith( "{" ): #or json.dumps( ref_json[val] ).startswith( "[" ): # change to isinstance
        if isinstance( ref_json[val], dict ): 
            #print( "recursing on nested object: val = " + val )
            recursive_json( ref_json[val], flat_input_json ) 
        elif isinstance( ref_json[val], list ):
            print( "Iterating over list under val: " + val )
            for obj in ref_json[val]: 
                # Ignore lists of numbers and strings
                if isinstance( obj, dict ) or isinstance( obj, list ): 
                    #print( "Found list or dict inside list to recurse." )
                    #print( obj )
                    recursive_json( obj, flat_input_json ) 
                elif "_Events" in val:
                    for event in ref_json[val]:
                        print( event )
                        if event != "NoTrigger" and event not in adhoc_events:
                            print( "Found event with name {0} under (array) val {1}.".format( event, val ) )
                            adhoc_events.append( event )
        else:
            #print( "Considering " + str(val ))
            if val == "Event" or val.startswith( "Event_") or ( ( "_Event" in val or "Event_Trigger" in val ) and "Num" not in val and "Count" not in val ):
                broadcast_event = ref_json[val]
                if broadcast_event not in adhoc_events:
                    print( "Found event with name {0} under (dict) val {1}.".format( broadcast_event, val ) )
                    adhoc_events.append( broadcast_event )

            if val not in flat_input_json:
                flat_input_json[val] = ref_json[val]
            #else:
                #print( "Ignoring {0} because already present.".format( val ) )

def application( config ):
    print( "DTK PRE PROC SCRIPT: Scrape all ad-hoc events and map to GPIO_X." )
    camp_json = json.loads( open( "campaign.json" ).read() )

    event_map = {}
    for camp_event in camp_json["Events"]:
        output_json = {}
        recursive_json( camp_event, output_json )
        """
        for key in output_json.keys():
            print( "Considering key: " + key )
            if "_Event" in key or "Event_Trigger" in key:
                broadcast_event = output_json[key]
                print( broadcast_event  )
                if broadcast_event not in adhoc_events:
                    adhoc_events.append( broadcast_event )
        """

    if len(adhoc_events) == 0:
        print( "ERROR: Didn't find any ad-hoc/user-defined events campaign file." )

    for event in adhoc_events:
        counter = len( event_map )
        builtin = "GP_EVENT_{:02d}".format(counter)
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

    # 2) Make changes
    config_json["parameters"]["Campaign_Filename"] = "campaign_xform.json"
    config_json_str = json.dumps( config_json )
    reverse_map = {}
    for event in event_map:
        config_json_str = config_json_str.replace( event, event_map[event] )
        reverse_map[ event_map[event] ] = event

    # 3) Save
    config_json = json.loads( config_json_str )
    config_json["parameters"]["Event_Map"] = reverse_map
    with open( "config_xform.json", "w" ) as conf_json_handle:
        conf_json_handle.write( json.dumps( config_json, sort_keys=True, indent=4 ) )

    return "config_xform.json"
