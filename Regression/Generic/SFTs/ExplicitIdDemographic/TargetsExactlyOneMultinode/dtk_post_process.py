#!/usr/bin/python

import dtk_test.dtk_sft as sft
import os.path as path
import json
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
    pass

"""
ExplicitIdDemographic
0. Config enables event db
1. Start with an outbreak day 5
2. After day 10, start finding people with New Infections, and add a txn blocking vax for each one
3. At end of simulation:
3.1 VERIFY campaign cost == infections
3.2 MAYBE VERIFY rate of new infections declines dramatically 
"""

"""
Enhanced: Only give as many as 20 vax a day.
If more than 20, rest get written into a "vax queue" file
So at each timestep, see if "vax queue" file exists and fill from there first
Maybe have a log file about "What we know and what we've tried to do?"
"""

"""
What could go wrong with Explicit ID?
- Migration: ID may change if in different node
- Birth / Death: How would I test targeting a dead person
- Invalid ID: What does model do if ID doesn't exist?

What features do I want: 
- Interrogate population: Is person still around?
"""

"""
Adds an outbreak event to the simulation at the day
specified by the 'Build_Campaign_Event_At' parameter in config.json
1. dtk_pre_process parses config.json and 
1.1 adds the event_template to the config.json file
1.2 reads the 'Build_Campaign_Event_At' parameter out of config.json and
1.2.1 writes this as a constant into dtk_in_process.py
2. dtk_in_process receives a timestep string from Eradication and
2.1 Checks to see if this is equal to the constant that dtk_pre_process wrote into here
2.1.1 And if it is, reads the config.json to write a campaign event for an outbreak on the next day
3. dtk_post_process parses the InsetChart.json, and
3.1 Makes sure that there are no infections before the event day and
3.2 There are some infected on the event day
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

    sft_messages = []
    # Preconditions: constant incubation, infectiousness, infectivity
    for distro in ['Incubation_Period_Distribution',
                   'Infectious_Period_Distribution',
                   'Base_Infectivity_Distribution']:
        if "CONSTANT_DISTRIBUTION" != config_params[distro]:
            sft_messages.append(f"WARNING: distribution {distro} expected to"
                                f" be CONSTANT_DISTRIBUTION, but got {config_params[distro]}.\n")
            pass
        pass

    # Load these up for verification later
    incubation_duration_period = config_params['Incubation_Period_Constant']
    infectious_duration_period = config_params['Infectious_Period_Constant']
    start_time = config_params['Start_Time']
    campaign_filename = config_params['Campaign_Filename']

    # parse campaign file
    with open(campaign_filename) as infile:
        campaign_json = json.load(infile)
        pass

    events = campaign_json['Events']
    if len(events) != 1:
        sft_messages.append(f"WARNING: Expected only one campaign event in starter "
                            f"campaign.json, but found {len(events)}.\n")
        pass
    outbreak_day = events[0]['Start_Day']


    # Load ICJ channels
    # Verify count of interventions (campaign cost)
    # Get cumulative infections channel count on outbreak day
    inset_path = path.join(output_folder, jsonreport_name)
    with open(inset_path) as infile:
        inset_json = json.load(infile)
        pass

    new_infections_channel = inset_json['Channels']['New Infections']['Data']
    campaign_cost_channel = inset_json['Channels']['Campaign Cost']['Data']

    # Test simulation outputs
    # Verify that at outbreak day + incubation period we have new infections
    secondary_infections_day = outbreak_day + incubation_duration_period
    final_infections_day = secondary_infections_day + infectious_duration_period
    secondary_infection_count = 0
    success = False  # Begin as a pessimist.
    new_infections_yesterday = None
    new_infections_back_two  = None
    campaign_cost_yesterday = None
    campaign_cost_back_two  = None
    for day in range(len(new_infections_channel)):
        new_infections_today = new_infections_channel[day]
        campaign_cost_today = campaign_cost_channel[day]
        if day < outbreak_day:
            if new_infections_channel[day] > 0:
                sft_messages.append(f"WARNING: Should be no infections until t={outbreak_day} "
                                    f"got {new_infections_today} at t={day}.\n")
                pass
            if campaign_cost_channel[day] > 0:
                sft_messages.append(f"WARNING: Should be no campaign cost accrued until after "
                                    f"{secondary_infections_day}. Got {campaign_cost_today} "
                                    f"at t={day}")
            pass
        elif day==outbreak_day:
            if new_infections_channel[day] == 0:
                sft_messages.append(f"BAD: No infections on {outbreak_day}, test is invalid.\n")
            else:
                sft_messages.append(f"GOOD: Got {new_infections_today} new infections "
                                    f"on outbreak day {outbreak_day}.\n")
                success = True
                pass
            pass
        elif day < secondary_infections_day:
            if campaign_cost_channel[day] > 0:
                success = False
                sft_messages.append(f"BAD: Should have no campaign cost until {secondary_infections_day}"
                                    f" but got {campaign_cost_today} on t={day}, test is invalid.\n")
                pass
            pass
        elif day < final_infections_day + 1:
            secondary_infection_count += new_infections_today
            sft_messages.append(f"\t Day: {day} had {new_infections_today} secondary infections.\n")
            expected_new_cost_today = campaign_cost_channel[day -1 ] + new_infections_back_two
            if campaign_cost_today != expected_new_cost_today:
                sft_messages.append(f"BAD: With {new_infections_today} secondary infections, expected "
                                    f"the same addition to campaign cost, but got {campaign_cost_today}.\n")
                success = False
                pass
            pass
        else:
            if campaign_cost_today > campaign_cost_channel[-1]:
                sft_messages.append(f"BAD: Expected no new costs after final infections day. At "
                                    f"t={day}, got {campaign_cost_today} which is higher than"
                                    f" yesterday's {campaign_cost_yesterday}.\n")
                pass
            pass
        new_infections_back_two  = new_infections_yesterday
        new_infections_yesterday = new_infections_today
        campaign_cost_back_two  = campaign_cost_yesterday
        campaign_cost_yesterday = campaign_cost_today
        pass

    final_msg_format = f"Expected the final campaign cost: {campaign_cost_today} " \
                       f"to equal the secondary infection count: {secondary_infection_count}"
    if campaign_cost_today == secondary_infection_count:
        sft_messages.append(f"GOOD: {final_msg_format}.\n")
    else:
        sft_messages.append(f"BAD: {final_msg_format}.\n")
        success = False
    with open(sft.sft_output_filename,'w', newline='') as outfile:
        outfile.writelines(sft_messages)
        outfile.write(sft.format_success_msg(success))

    print(sft.format_success_msg(success))
    # Verify new infections for whole infectious period
    # Verify that for +infectious period after that, no more infections
    # Verify that for that until sim end, no more new infections

    # Test cumulative infections - outbreak interventions = campaign cost[-1]


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

