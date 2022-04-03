#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_datetime

print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )
dtk_datetime.update( 1 )
print( "DTK datetime is now: {0}".format(dtk_datetime.get_time()) )

print( "DTK base year is:     {0}".format(dtk_datetime.get_base_year()) )
dtk_datetime.set_base_year( 1234 )
print( "DTK base year is now: {0}".format(dtk_datetime.get_base_year()) )

# test is less

print( "DTK datetime.is_less_than( 5 ) returns:    {0}".format(dtk_datetime.is_less_than( 5 )) )
print( "DTK dateimte.is_less_than( 1239 ) returns: {0}".format(dtk_datetime.is_less_than( 1234+5 )) )
