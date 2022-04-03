#!/usr/bin/python

import sys
import json

default_config = { "parameters": {} }
schema = json.loads( open( sys.argv[1] ).read() )
for group in schema["config"]:
    for param in schema["config"][group]:
        if 'default' in schema["config"][group][param]:
            value = schema["config"][group][param]['default']
            default_config[ "parameters" ][ param ] = value

with open( "default_config.json", "w" ) as outfile:
    json.dump( default_config, outfile, sort_keys=True, indent=4 )
