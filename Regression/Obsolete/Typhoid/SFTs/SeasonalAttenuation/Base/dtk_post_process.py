#!/usr/bin/python
# This SFT test the following statement:
# Seasonality. The amount of environmental contagion (dose) to which an individual is exposed is attenuated/amplified
# by a seasonality factor
#              that varies throughout the year according to a trapezoidal pattern.
#              The ramp durantions and cutoff days need to follow a rule where: ramp_up_duration + ramp_down_duration
#               + cutoff_days < 365
#              check environmental amplification and exposure for each individual, in each time step

import dtk_test.dtk_SeasonalAttenuation_Support as dtk_SAS
import dtk_test.dtk_sft as dtk_sft

def application(output_folder="output",
                config_filename="config.json",
                stdout="test.txt",
                report_name=dtk_sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("config_filename: " + config_filename + "\n")
        print("stdout: " + stdout + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    dtk_sft.wait_for_done(stdout)
    dtk_SAS.application(report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-r', '--reportname', default=dtk_sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output,
                config_filename=args.config,
                stdout=args.stdout,
                report_name=args.reportname, debug=args.debug)
