#!/usr/bin/python
# This SFT test the following statement:
# Seasonality. The amount of environmental contagion (dose) to which an individual is exposed is attenuated/amplified
# by a seasonality factor that varies throughout the year according to a trapezoidal pattern.
# The ramp durations and cutoff days need to follow a rule where: ramp_up_duration + ramp_down_duration
# + cutoff_days < 365. Peak start is when the pattern starts and then it's calculated backwards from ramp up end time.
# Please see documentation for more details.
#
# Test:
# In this test we check that seasonal attenuation calculated in code matches what we expect based on the config
# settings and we verify that the contagion each individual is exposed to is total,
# per-individual contagion attenuated by the seasonal attenuation factor.
#
# The config settings create an environment where Cutoff Days is 0, so the ramp up starts
# immediately after ramp down ends.
# "Environmental_Cutoff_Days": 0,
# "Environmental_Peak_Start": 360,
# "Environmental_Ramp_Down_Duration": 170
# "Environmental_Ramp_Up_Duration": 30

import dtk_test.dtk_SeasonalAttenuation_Support as dtk_SAS
import dtk_test.dtk_sft as sft

def application(output_folder="output",
                config_filename="config.json",
                stdout="test.txt",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("config_filename: " + config_filename + "\n")
        print("stdout: " + stdout + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout)
    dtk_SAS.application(report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output,
                config_filename=args.config,
                stdout=args.stdout,
                report_name=args.reportname, debug=args.debug)
