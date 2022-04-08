#!/usr/bin/python

import dtk_test.dtk_sft as sft
import dtk_test.dtk_ZeroInfectivity_Terminate_Support as zts

"""
Enable Termination on Zero Infectivity causes the simulation to stop when there are no infections left.
This tests the following:
GIVEN a multicore generic simulation AND Zero Infectivity Termination enabled,
WHEN the simulation has passed the Minimum_End_Time AND there is still infectivity
THEN the simulation continues until zero infectivity and terminates 
"""

def application( output_folder="output",
                 config_filename="config.json",
                 chart_filename="InsetChart.json",
                 report_name=sft.sft_output_filename,
                 debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("config_filename: " + config_filename + "\n")
        print("insetchart_name: " + chart_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done()
    config_object = zts.parse_config_file(config_filename, debug=debug)
    # Load config file: Simulation_Duration, Enable_Termination..., Minimum_End_Time

    chart_object = zts.parse_json_report(output_folder=output_folder)
    # Load inset chart: get statistical population channel and infected channel

    zts.create_report_file(config_object=config_object, report_object=chart_object,
                          report_name=report_name, debug=debug)
    pass


if __name__ == "__main__":
    # execute only if run as a script
    application( "output" )