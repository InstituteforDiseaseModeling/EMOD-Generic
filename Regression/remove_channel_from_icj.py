#!/usr/bin/python

import json
import os
import sys

icj = json.loads( open( sys.argv[1] ).read() )

icj["Channels"].pop( "Log Prevalence" )

with open( sys.argv[1], "w" ) as icj_file:
    icj_file.write( json.dumps( icj, indent=4 ) )
