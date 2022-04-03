#####################################################################################################################
# This test validates that individuals remain 100% immune to the serotypes with which they have been infected forever.
#     In this campaign, the entire population in infected with Strain 2 on day 10, then with Strain 4 on day 110.
#     We validate via the Immunity channels that the entire population remains immuned to Strain 2 starting day 10, and 
#     and immuned to Strain 4 after day 110.
#####################################################################################################################

#!/usr/bin/python

import re
import json
import math
import pdb
import dtk_plot_wrapper
import dtk_sft

# C version: infectiousness = exp( -1 * _infectiousness_param_1 * pow(duration - _infectiousness_param_2,2) ) / _infectiousness_param_3;

def inf_calc( dur, c1, c2, c3 ):
    x = c1 * math.pow( (dur+1 - c2), 2 )
    return math.exp(-1*x) /c3

def application( report_file ):

    #Read config.json to calculate the population size
    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    if cdj["Enable_Demographics_Builtin"] != 1:
        raise Exception("\"Enable_Demographics_Initial\" should be set to 0 to control the population via default geography.")        
    node_pop = cdj["Default_Geography_Initial_Node_Population"]
    torus_size = cdj["Default_Geography_Torus_Size"]
    total_pop = torus_size ** 2 * node_pop
     
    #Read InsetChart.json to get the Immunity Population channel datta
    icj = json.loads( open( "output/InsetChart.json" ).read() )
    strain2immunity_channel = icj["Channels"]["Immunity Population (Strain 2)"]["Data"] 
    strain4immunity_channel = icj["Channels"]["Immunity Population (Strain 4)"]["Data"] 

    success = True
    with open(dtk_sft.sft_output_filename, "w" ) as report_file:
          for idx in range(0, len(strain2immunity_channel)):
                strain2immunity = strain2immunity_channel[idx]
                strain4immunity = strain4immunity_channel[idx]
                if (idx < 10):
                    if (strain2immunity != 0):
                        report_file.write( "Immunity (Strain 2) should be 0 before any outbreak starts.  Actual immunity is {0} at timestep {1}\n".format(strain2immunity, idx) )
                        success = False
                        break
                    if (strain4immunity != 0):
                        report_file.write( "Immunity (Strain 4) should be 0 before any outbreak starts.  Actual immunity is {0} at timestep {1}\n".format(strain4immunity, idx) )
                        success = False
                        break
                # The first outbreak (with strain 1) starts on day 10
                elif (idx  >= 10) and (idx < 110):
                    if ( strain2immunity != total_pop ):
                        report_file.write( "Immunity (Strain 2) should equal total population of {0} after first outbreak. Actual immunity is {1} at timestep {2}\n".format(total_pop, strain2immunity, idx) )
                        success = False
                        break
                # The second outbreak (with strain 3) starts on day 110
                elif (idx >= 110):
                    if ( strain2immunity != total_pop ):
                        report_file.write( "Immunity (Strain 2) should remain equal to the total population of {0} after second outbreak. Actual immunity is {1} at timestep {2}\n".format(total_pop, strain2immunity, idx) )
                        success = False
                        break
                    if ( strain4immunity != total_pop ):
                        report_file.write( "Immunity (Strain 4) should equal total population of {0} after second outbreak. Actual immunity is {1} at timestep {2}\n".format(total_pop, strain4immunity, idx) )
                        success = False
                        break
                
          report_file.write( dtk_sft.format_success_msg( success ) )

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )

