#!/usr/bin/python3
#import sys
#sys.path.append( "build/lib.linux-x86_64-3.6" )
"""
I'll leave Test team to create real tests but for now you can see that the immunity is 50% for 180 days
per the sv.json and waning box duration then goes back to 1.0. All sorts of cool testing can be done
with ease on interventions and their effects.
"""
import dtk_generic_intrahost as gi
import dtk_vaccine_intervention as sv
import json
#import pdb

id = gi.create( ( 0, 100.0, 1.0 ) )
print( json.dumps( gi.serialize( id ) ) )

#iv = sv.get_intervention()
#gi.give_intervention( ( id, iv ) )
sv.distribute( gi.get_individual_for_iv( id ) )
for _ in range(365):
    gi.update( id )
    imm = gi.get_immunity( id ) 
    print( str( imm ) )

# Plan A is to dump serialized individual but interventions container crashes on serialization???
#print( json.dumps( gi.serialize( id ) ) )
