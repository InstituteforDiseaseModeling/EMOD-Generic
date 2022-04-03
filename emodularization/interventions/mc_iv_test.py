#!/usr/bin/python

#import pdb
import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_malariachallenge as mc

def report_challenge( type, num, coverage ):
    print( type, str( num ), str( coverage ) )

mc.my_set_callback( report_challenge )
mc.distribute()
schema = mc.get_schema()
print( schema )
