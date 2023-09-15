#!/usr/bin/python
"""
Maternal protection applies a WaningEffect for an agent's mod_acquire value
"""

import dtk_test.dtk_MaternalProtection_Support as MP_Support
import dtk_test.dtk_sft as dtk_sft
import json

import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)

def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                report_name=dtk_sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")
    dtk_sft.wait_for_done()
    param_obj = MP_Support.load_emod_parameters(config_filename)
    simulation_timestep = param_obj[MP_Support.KEY_SIMULATION_TIMESTEP]
    if debug:
        print(param_obj)
        print("simulation time step is : {0} days".format(simulation_timestep))
    output_df = MP_Support.parse_output_file(stdout_filename, debug)
    MP_Support.create_report_file(param_obj, output_df, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-r', '--reportname', default=dtk_sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)
