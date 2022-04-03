#!/usr/bin/python

import icjjson2csv
import pp_with_sqlite
import os

def application( output_path ):
    icjjson2csv.doit( output_path )
    pp_with_sqlite.main( os.path.join( output_path, "InsetChart.csv" ) )
