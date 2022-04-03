#!/usr/bin/python

import sys
import pdb
print( sys.path )
#pdb.set_trace()
import dtk_something

def application( path ):
    dtk_something.doit( path )
    pass
