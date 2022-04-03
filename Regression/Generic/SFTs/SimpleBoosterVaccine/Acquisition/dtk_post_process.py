#!/usr/bin/python

import os.path as path
import dtk_test.dtk_sft as sft
import json

import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)
import math
import dtk_test.dtk_SimpleBoosterVaccine_Support as sbvs

KEY_NEW_INFECTIONS_GROUP = ["New Infections:QualityOfCare:1_VaccineOnly",
                            "New Infections:QualityOfCare:2_BoosterOnly",
                            "New Infections:QualityOfCare:3_VaccinePlusBooster",
                            "New Infections:QualityOfCare:4_BoosterPlusVaccine",
                            "New Infections:QualityOfCare:5_TwoBoosters"]
KEY_STATISTICAL_POPULATION_GROUP = ["Statistical Population:QualityOfCare:1_VaccineOnly",
                                    "Statistical Population:QualityOfCare:2_BoosterOnly",
                                    "Statistical Population:QualityOfCare:3_VaccinePlusBooster",
                                    "Statistical Population:QualityOfCare:4_BoosterPlusVaccine",
                                    "Statistical Population:QualityOfCare:5_TwoBoosters"]

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


def calc_immunity(vax_event_obj, debug=False):
    """
    calculate the expected immunity for each group. we are using values in Campaign file.
    :param debug:
    :return: immunity
    """
    # Testing across 5 different tests here.  These two values remain consistent throughout the tests.
    # Booster threshold jumps from 0 to 0.6 after 1st outbreak but is not used here.
    prime = vax_event_obj[sbvs.CampaignKeys.Prime_Effect]
    boost = vax_event_obj[sbvs.CampaignKeys.Boost_Effect]

    initial = 0.55  # From campaign file, but doesn't align with other events & other tests don't use this value
    immunity = []
    immunity.append([initial, initial])   # 1_VaccineOnly
    immunity.append([prime, prime])       # 2_BoosterOnly
    immunity.append([initial + (1.0 - initial) * boost, initial + (1.0 - initial) * prime])  # 3_VaccinePlusBooster
    immunity.append([prime + (1.0 - prime) * initial, prime + (1.0 - prime) * initial])      # 4_BoosterPlusVaccine
    immunity.append([prime + (1.0 - prime) * boost, prime + (1.0 - prime) * prime])          # 5_TwoBoosters
    if debug:
        print(immunity)
    return immunity


def create_report_file(vax_event_obj, outbreak_event_obj, param_obj, report_data_obj, report_name, debug):
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj[sbvs.ConfigKeys.CONFIG_NAME]) + ", Run number: " +
                      str(param_obj[sbvs.ConfigKeys.RUN_NUMBER]) +
                      "\n# Test compares the reported vs. expected new infections to examine acquisition effects.\n")
        success = True
        immunity = calc_immunity(vax_event_obj, debug)
        new_infections = []
        expected_new_infections = []
        timestep = outbreak_event_obj[sbvs.CampaignKeys.Start_Day]
        if not report_data_obj:
            success = False
            outfile.write("BAD: There is no data in the PropertyReport report.")
        else:
            for i in range(outbreak_event_obj[sbvs.CampaignKeys.Number_Repetitions]):
                for j in range(len(KEY_NEW_INFECTIONS_GROUP)):
                    new_infection = report_data_obj[KEY_NEW_INFECTIONS_GROUP[j]][timestep]
                    statistical_population = report_data_obj[KEY_STATISTICAL_POPULATION_GROUP[j]][timestep]
                    expected_new_infection = statistical_population * (1.0 - immunity[j][i])
                    tolerance = 0.0 if expected_new_infection == 0.0 else 2e-2 * statistical_population
                    low_acceptance_bound = expected_new_infection - tolerance
                    high_acceptance_bound = expected_new_infection + tolerance
                    if math.fabs(new_infection - expected_new_infection) > tolerance:
                        success = False
                        outfile.write(
                            "BAD: At time step {0}, {1} reports {2} new infections, which is not within acceptance "
                            "range of ({3}, {4}).  Expected {5}.\n".format(timestep, KEY_NEW_INFECTIONS_GROUP[i],
                                                                           new_infection, low_acceptance_bound,
                                                                           high_acceptance_bound,
                                                                           expected_new_infection))
                    new_infections.append(new_infection)
                    expected_new_infections.append(expected_new_infection)
                timestep += outbreak_event_obj[sbvs.CampaignKeys.Timesteps_Between_Repetitions]
        if success:
            outfile.write("GOOD: For all time steps, the new infections were within tolerance (within 2% of statistical"
                          " population) of the expected.\n")
        outfile.write(sft.format_success_msg(success))
    sft.plot_data(new_infections, expected_new_infections,
                  label1="Actual", label2="Expected",
                  xlabel="group: 0-4 outbreak 1, 5-9 outbreak 2", ylabel="new infection",
                  title="Actual new infection vs. expected new infection",
                  category='New_infections', show=True)
    if debug:
        print("SUMMARY: Success={0}\n".format(success))
    return success


def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                propertyreport_name="PropertyReport.json",
                report_name=sft.sft_output_filename,
                debug=False):

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
    vax_event_obj = sbvs.parse_vax_event(campaign_filename, 1)
    outbreak_event_obj = sbvs.parse_outbreak_event(campaign_filename, 11)
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
                        help="Property report name to load (PropertyReport.json")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)
