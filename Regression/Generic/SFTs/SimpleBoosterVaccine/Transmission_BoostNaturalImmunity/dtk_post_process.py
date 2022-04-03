#!/usr/bin/python

import os.path as path
import json
import math
import dtk_test.dtk_sft as sft
import dtk_test.dtk_SimpleBoosterVaccine_Support as sbvs

import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)

KEY_NEW_INFECTIONS_GROUP = ["New Infections:QualityOfCare:1_Seed_Control",
                            "New Infections:QualityOfCare:2_Seed_Test",
                            "New Infections:QualityOfCare:3_Control",
                            "New Infections:QualityOfCare:4_Test"]

KEY_STATISTICAL_POPULATION_GROUP = ["Statistical Population:QualityOfCare:1_Seed_Control",
                                    "Statistical Population:QualityOfCare:2_Seed_Test",
                                    "Statistical Population:QualityOfCare:3_Control",
                                    "Statistical Population:QualityOfCare:4_Test"]


"""
SimpleBoosterVaccine is derived from SimpleVaccine, and preserves many of the same parameters.
The behavior is much like SimpleVaccine, except that upon distribution and successful take,
the vaccine's effect is determined by the recipient's immune state. If the recipient's immunity
modifier in the corresponding channel (Acquisition, Transmission, Mortality) is above a user-specified
threshold (Boost_Threshold), then the vaccine's initial effect will be equal to the parameter
Prime_Effect. If the recipient's immune modifier is below this threshold, then the vaccine's
initial effect will be equal to the parameter Boost_Effect. After distribution, the effect evolves
according to the Waning_Config, just like a Simple Vaccine. In essence, this intervention provides
a SimpleVaccine with one effect to all naive (below- threshold) individuals, and another effect to
all primed (above-threshold) individuals; this behavior is intended to mimic, to some degree,
biological priming and boosting.
All parameters necessary to define a SimpleVaccine are also necessary for a SimpleBoosterVaccine
(though the Initial_Effect of the Waning_Config will be overwritten by one of Prime_Effect or
Boost_Effect upon distribution).
"""


def create_report_file(vax_event_obj, outbreak_event_obj, param_obj, report_data_obj, report_name, debug):
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj[sbvs.ConfigKeys.CONFIG_NAME]) + ", Run number: " +
                      str(param_obj[sbvs.ConfigKeys.RUN_NUMBER]) +
                      "\n# Test compares the transmission blocking effects to the expected"
                      " as well as the reported vs. expected new infections.\n")
        success = True
        timestep = outbreak_event_obj[sbvs.CampaignKeys.Start_Day]
        tb_effect = vax_event_obj[sbvs.CampaignKeys.Prime_Effect]
        baseline = 2  # group 3_Control is the baseline
        test = 3      # group 4_Test is the test group
        # first outbreak
        pre_new_infection_baseline = report_data_obj[KEY_NEW_INFECTIONS_GROUP[baseline]][timestep]
        pre_new_infection_test = report_data_obj[KEY_NEW_INFECTIONS_GROUP[test]][timestep]
        # second outbreak
        timestep += outbreak_event_obj[sbvs.CampaignKeys.Timesteps_Between_Repetitions]
        new_infection_baseline = report_data_obj[KEY_NEW_INFECTIONS_GROUP[baseline]][timestep]
        statistical_population_baseline = report_data_obj[KEY_STATISTICAL_POPULATION_GROUP[baseline]][timestep]
        new_infection_test = report_data_obj[KEY_NEW_INFECTIONS_GROUP[test]][timestep]
        statistical_population_test = report_data_obj[KEY_STATISTICAL_POPULATION_GROUP[test]][timestep]
        # because expected_new_infection_test / ((1.0 - tb_effect)* statistical_population_test)=
        # new_infection_baseline / statistical_population_baseline, so
        expected_new_infection_test = (1.0 - tb_effect) * new_infection_baseline * statistical_population_test \
            / statistical_population_baseline
        tolerance = 0.0 if expected_new_infection_test == 0.0 else 2e-2 * statistical_population_test
        low_acceptance_bound = expected_new_infection_test - tolerance
        high_acceptance_bound = expected_new_infection_test + tolerance
        if math.fabs(new_infection_test - expected_new_infection_test) > tolerance:
            success = False
            outfile.write("BAD: At time step {0}, {1} reported new infections in Group 4_Test, which is not"
                          " within acceptance range ({2}, {3}).  Expected {4}.\n"
                          .format(timestep, new_infection_test, low_acceptance_bound, high_acceptance_bound,
                                  expected_new_infection_test))
        if success:
            outfile.write("GOOD: For all time steps, the new infections were within tolerance (within 2% of statistical"
                          " population) of the expected.\n")
        sft.plot_data([pre_new_infection_baseline, new_infection_baseline],
                      [pre_new_infection_test, new_infection_test],
                      label1="control_group", label2="test_group",
                      xlabel="0: first outbreak, 1: second outbreak",ylabel="new infection",
                      title="control vs. test",
                      category='New_infections', show=True)

        outfile.write(sft.format_success_msg(success))
        if debug:
            print("SUMMARY: Success={0}\n".format(success))
        return success

def application( output_folder="output", stdout_filename="test.txt",
                 config_filename="config.json",campaign_filename="campaign.json",
                 demographics_filename = "../../demographics.json",
                 propertyreport_name="PropertyReport.json",
                 report_name=sft.sft_output_filename,
                 debug=False):
    if debug:
        print( "output_folder: " + output_folder )
        print( "stdout_filename: " + stdout_filename+ "\n" )
        print( "config_filename: " + config_filename + "\n" )
        print( "campaign_filename: " + campaign_filename + "\n" )
        print( "demographics_filename: " + demographics_filename + "\n" )
        print( "propertyreport_name: " + propertyreport_name + "\n" )
        print( "report_name: " + report_name + "\n" )
        print( "debug: " + str(debug) + "\n" )

    sft.wait_for_done()
    param_obj = sbvs.load_emod_parameters(config_filename)
    campaign_filename = param_obj[sbvs.ConfigKeys.CAMPAIGN_NAME]
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("propertyreport_name: " + propertyreport_name + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")
    vax_event_obj = sbvs.parse_vax_event(campaign_filename, 2)
    outbreak_event_obj = sbvs.parse_outbreak_event(campaign_filename, 1)
    report_data_obj = sbvs.parse_json_report(KEY_NEW_INFECTIONS_GROUP, KEY_STATISTICAL_POPULATION_GROUP,
                                             None, output_folder, propertyreport_name, debug)
    create_report_file(vax_event_obj, outbreak_event_obj, param_obj, report_data_obj, report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script

    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-p', '--propertyreportname', default="PropertyReport.json",
                        help="Property report name to load (PropertyReport.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)
