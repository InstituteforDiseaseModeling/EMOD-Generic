#!/usr/bin/python3

import dtk_artbasic_iv as iv
import dtk_hiv_intrahost as hi
art = iv.get_intervention()
person = hi.create( ( 0, 3650.0, 1.0 ) )
print( str( person ) )
hi.give_intervention( ( person, art ) )

