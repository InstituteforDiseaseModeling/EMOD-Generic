#!/usr/bin/python
# This SFT test the following statement:
# At each time step, the # of new Acute infections in log (- exit acute at the same time step)matches the data in "Number of New Acute Infections" channel in InsetChart.json
# note:"Enable_Vital_Dynamics": 0

import re
import json
import datetime
import time
import dtk_sft as sft


def get_val( key, line ):
    regex = key + "(\d*\.*\d*)"
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError


def application( do_not_use):
    report_file = ""
    debug = True
    if not report_file:
        report_file = "test.txt"
    sft.wait_for_done()
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )

    isj = json.loads(open("output/InsetChart.json").read())["Channels"]
    env_contagion_population = isj["New Infections By Route (ENVIRONMENT)"]["Data"]

    regex0 = "for individual 123"
    regex1 = "current_dose_attenuation_environment"

    success = False
    start_time = datetime.datetime.now()
    max_time = datetime.timedelta(seconds=600)
    elapsed_time = datetime.timedelta(seconds=0)
    while not (success or elapsed_time > max_time):
        env_attenuation = []
        filtered_lines = []
        with open( report_file ) as logfile:
            for line in logfile:
                if regex0 in line and regex1 in line:
                    env_attenuation.append(float(line.split()[8].rstrip(',')))
                    if debug:
                        filtered_lines.append(line)
        if len(env_attenuation) >= len(env_contagion_population):
            success = True
        else:
            time.sleep(10)
            elapsed_time = datetime.datetime.now() - start_time

    if debug:
        with open("filtered_lines.txt","w") as outfile:
            outfile.write("First filtered line split: \n" )
            splits = filtered_lines[0].split()
            for i in range(0, len(splits)):
                outfile.write("\t " + str(i) + " " + splits[i] + " \n")
            outfile.write("Filtered Lines: \n")
            for line in filtered_lines:
                outfile.write(line)

    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len(env_attenuation) == 0:
            # make sure we have found the correct debug data
            success = False
            report_file.write("BAD: Couldn't find current_dose_attenuation_environment for individual 213.\n" )
        if len(env_attenuation) != len(env_contagion_population):
            success = False
            err_template = "BAD: Length difference between Env attenuation: {0} Env_Contagion_Population: {1}\n"
            report_file.write(err_template.format(
                                  len(env_attenuation),
                                  len(env_contagion_population)))
        else:
            for i in range(0, len(env_contagion_population)-1):

                # should both be either zero (no shedding, no env contagion) or both non-zero (shedding and contagion)
                if env_attenuation[i] == 0 and env_contagion_population[i] != 0:
                    success = False
                    err_template = "BAD: At time {0}: the {1} is {2} from Stdout, " + \
                                   "while the contagion is {3} from InsetChart.json.\n"
                    report_file.write(err_template.format(
                        i,
                        regex1,
                        env_attenuation[i],
                        env_contagion_population[i]))
        if success:
            report_file.write( sft.format_success_msg( success ) )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application("")
