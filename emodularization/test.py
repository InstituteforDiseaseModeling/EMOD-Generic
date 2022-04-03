#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_datetime

print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )
dtk_datetime.update( 1 )
print( str( dtk_datetime.get_time() ) )

print( str( dtk_datetime.get_base_year() ) )
dtk_datetime.set_base_year( 1234 )
print( str( dtk_datetime.get_base_year() ) )

# test is less

print( str( dtk_datetime.is_less_than( 5 ) ) )
print( str( dtk_datetime.is_less_than( 1234+5 ) ) )
