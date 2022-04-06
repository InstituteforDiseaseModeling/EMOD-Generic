#!/usr/bin/python

import dtk_test.dtk_sft as sft
import os.path as path
import json
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
    pass

"""
ExplicitIdDemographic a simple test
1. Start with an outbreak day 4 target ID 1 and 1000.
2.  VERIFY New Infections == 2
"""


def application(output_folder="output", config_filename="config.json",
                jsonreport_name="InsetChart.json", stdout_filename="test.txt",
                report_name=sft.sft_output_filename, debug=False):
    if debug:
        print("output_folder: " + output_folder + "\n")
        print("config_filename: " + config_filename + "\n")
        print("insetchart_name: " + jsonreport_name + "\n")
        print("stdout filename:" + stdout_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done()
    # parse config file
    # Verify incubation period
    # Verify infectious period
    # Verify base infectivity
    # Verify campaign filename
    config_params = None
    with open(config_filename) as infile:
        config_params = json.load(infile)['parameters']
        pass
    success = True

    # Load these up for verification later
    campaign_filename = config_params['Campaign_Filename']
    # parse campaign file
    with open(campaign_filename) as infile:
        campaign_json = json.load(infile)
        pass
    events = campaign_json['Events']
    outbreak_day = events[0]['Start_Day']

    # Load ICJ channels
    # Verify count of New Infections
    inset_path = path.join(output_folder, jsonreport_name)
    with open(inset_path) as infile:
        inset_json = json.load(infile)
        pass

    new_infections_channel = inset_json['Channels']['New Infections']['Data']
    sft_messages = []
    if new_infections_channel[outbreak_day] != 2:
        success = False
        sft_messages.append(f"BAD: Should be 2 new infections at t={outbreak_day} "
                                    f"got {new_infections_channel[outbreak_day]}.\n")
    else:
        sft_messages.append(f"GOOD: Should be 2 new infections at t={outbreak_day} "
                                    f"got {new_infections_channel[outbreak_day]}.\n")

    with open(sft.sft_output_filename, 'w', newline='') as outfile:
        outfile.writelines(sft_messages)
        outfile.write(sft.format_success_msg(success))
    print(sft.format_success_msg(success))


if __name__ == "__main__":
    # execute only if run as a script
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-j', '--jsonreport', default="InsetChart.json", help="Json report to load (InsetChart.json)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output, config_filename=args.config,
                jsonreport_name=args.jsonreport,
                report_name=args.reportname, debug=args.debug)
    pass

