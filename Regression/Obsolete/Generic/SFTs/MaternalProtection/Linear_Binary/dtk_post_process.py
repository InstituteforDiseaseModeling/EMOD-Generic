#!/usr/bin/python
"""
The test methodology is trying to match the susceptibility correction for all agents for all maternal_type, as follows:
Both "FRACTIONAL" implementations assign an equal susceptibility to all agents. This value is a function of age only:
Linear: Susceptibility = Slope * Age + SusZero
Sigmoid: Susceptibility = SusInit + (1.0 - SusInit) / (1.0 + EXP( (HalfMaxAge - Age) / SteepFac) )
Both "BINARY" implementations assign an age cutoff to each agent. If the agent age is less than the cutoff age, then the
agent has susceptibility reduced to zero.
If the agent age is greater than or equal to the cutoff age, the agent does not have reduced susceptibility. The age
assigned to each agent is randomly assigned:
Linear: AgeCutoff = (RAND - SusZero) / Slope
Sigmoid: AgeCutoff = HalfMaxAge - SteepFac * LOG( (1.0 - SusInit) / (RAND - SusInit) - 1.0 + 0.001)
if the SFT passed, the output file (normally scientific_feature_report.txt) will show params for run as well as
 "Success=True", as follows:
    Simulation parmaters: simulation_duration=5, simulation_timestep=1.0, enable_birth=1, base_infectivity=0.25
    maternal_protection_type = LINEAR:
    susceptibility_type = BINARY:
    It's a linear type with linear_slope:0.012 and linear_susZero:0.0
    SUMMARY: Success=True
if the SFT failed, the output file (normally scientific_feature_report.txt) will show params for run, "Success=False",
as well as which test(s) failed, as follows:
    Simulation parmaters: simulation_duration=5, simulation_timestep=1.0, enable_birth=1, base_infectivity=0.25
    maternal_protection_type = LINEAR:
    susceptibility_type = FRACTIONAL:
    It's a linear type with linear_slope:0.012 and linear_susZero:0.0
    BAD: actual mod_acquire for individual 1.0 at time step 0.0 is 2.0, expected 0.0.
    SUMMARY: Success=False
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
