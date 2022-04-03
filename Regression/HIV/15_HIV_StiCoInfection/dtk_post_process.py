#!/usr/bin/python
import json
import os

def recursive_json( ref_json, flat_input_json ):
    for val in ref_json:
        #if not leaf, call recursive_json_leaf_reader
        #if json.dumps( ref_json[val] ).startswith( "{" ) or json.dumps( ref_json[val] ).startswith( "[" ): # change to isinstance
        if json.dumps( ref_json[val] ).startswith( "{" ): #or json.dumps( ref_json[val] ).startswith( "[" ): # change to isinstance
            recursive_json( ref_json[val], flat_input_json ) 
        else:
            if val not in flat_input_json:
                flat_input_json[val] = ref_json[val]

def application( output_path ):
    print( "DTK POST PROC SCRIPT: Convert all GPIO_X labels in output files." )
    config_json = json.loads( open( "config_xform.json" ).read() )
    event_map = config_json["parameters"]["Event_Map"]

    for filename in os.listdir( output_path ):
        if filename.endswith( ".json" ) or filename.endswith( ".csv" ):
            lines = []
            with open( os.path.join( output_path, filename ) ) as report_in:
                for line in report_in:
                    for event in event_map:
                        line = line.replace( event, event_map[ event ] )
                    lines.append( line )

            with open( os.path.join( output_path, filename ), "w" ) as report_out:
                for line in lines:
                    report_out.write( line )

    return "blah"
