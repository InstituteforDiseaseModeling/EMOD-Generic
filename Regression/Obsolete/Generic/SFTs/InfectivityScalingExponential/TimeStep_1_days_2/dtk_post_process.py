#!/usr/bin/python

import dtk_test.dtk_InfectivityScalingExponential_Support as ISE_Support
import dtk_test.dtk_sft as dtk_sft
import json

import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)

def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                insetchart_name="InsetChart.json",
                report_name=dtk_sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("insetchart_name: " + insetchart_name + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")
    dtk_sft.wait_for_done()
    param_obj = ISE_Support.load_emod_parameters(config_filename)
    simulation_timestep = param_obj["Simulation_Timestep"]
    if debug:
        print("simulation time step is : {0} days".format(simulation_timestep))
    output_df = ISE_Support.parse_output_file(stdout_filename, simulation_timestep, debug)
    report_df = ISE_Support.parse_json_report(output_folder, insetchart_name, debug)
    ISE_Support.create_report_file(param_obj, output_df, report_df, report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-i', '--insetchartname', default="InsetChart.json", help="insetchart to test(InsetChart.json)")
    parser.add_argument('-r', '--reportname', default=dtk_sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                config_filename=args.config, insetchart_name=args.insetchartname,
                report_name=args.reportname, debug=args.debug)
