#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_timers

dtk_timers.init()
counts = 0
try:
    while dtk_timers.expired() == False:
        dtk_timers.dec( 1 )
        counts += 1
        print( "counts = " + str(counts) + ", expiration = " + str( dtk_timers.expired() ) )
except Exception as ex:
    print( str( ex ) )


