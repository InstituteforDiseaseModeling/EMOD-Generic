#!/usr/bin/python

import os.path as path
import math
import json
import dtk_test.dtk_sft as sft
import dtk_test.dtk_SimpleBoosterVaccine_Support as sbvs

import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)

KEY_NEW_INFECTIONS_GROUP = ["New Infections:QualityOfCare:1_Seed",
                            "New Infections:QualityOfCare:2_Susceptible"]
KEY_STATISTICAL_POPULATION_GROUP = ["Statistical Population:QualityOfCare:1_Seed",
                                    "Statistical Population:QualityOfCare:2_Susceptible"]
Interventions = ["1st outbreak: no TransmissionBlocking",
                 "2nd: booster_h threshold",
                 "3rd: booster_l threshold",
                 "4th: vaccine only",
                 "5th: booster_l + booster_h",
                 "6th: booster_h + booster_l",
                 "7th: booster_l + vaccine",
                 "8th: vaccine + booster_l",
                 "9th: vaccine + 2 booster_l",
                 "10th: vaccine + 2 booster_h"]


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


def calc_tb_effect(debug=False):
    """
    calculate the expected TransmissionBlocking effect before outbreak.
    :param debug:
    :return: immunity
    """
    effects = []
    # 1st outbreak: no TransmissionBlocking
    effects.append(0.0)
    # 2nd: booster_h threshold
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.BOOSTER_HIGH]))
    # 3rd: booster_l threshold
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.BOOSTER_LOW]))
    # 4th: vaccine only
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.VACCINE]))
    # 5th: booster_l + booster_h
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.BOOSTER_LOW, sbvs.Vaccine.BOOSTER_HIGH]))
    # 6th: booster_h + booster_l
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.BOOSTER_HIGH, sbvs.Vaccine.BOOSTER_LOW]))
    # 7th: booster_l + vaccine
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.BOOSTER_LOW, sbvs.Vaccine.VACCINE]))
    # 8th: vaccine + booster_l
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.VACCINE, sbvs.Vaccine.BOOSTER_LOW]))
    # 9th: vaccine + 2 booster_l
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.VACCINE, sbvs.Vaccine.BOOSTER_LOW, sbvs.Vaccine.BOOSTER_LOW]))
    # 10th: vaccine + 2 booster_h
    effects.append(sbvs.apply_vaccines([sbvs.Vaccine.VACCINE, sbvs.Vaccine.BOOSTER_HIGH, sbvs.Vaccine.BOOSTER_HIGH]))

    if debug:
        with open("DEBUG_calc_tb_effects.json", "w") as effects_file:
            json.dump(effects, effects_file, indent=4, sort_keys=True)
    return effects


def create_report_file(outbreak_event_obj, param_obj, report_data_obj, report_name, debug):
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj[sbvs.ConfigKeys.CONFIG_NAME]) + ", Run number: " +
                      str(param_obj[sbvs.ConfigKeys.RUN_NUMBER]) +
                      "\n# Test compares the transmission blocking effects to the expected "
                      "as well as the reported vs. expected new infections.\n")
        success = True
        timestep = outbreak_event_obj[sbvs.CampaignKeys.Start_Day]
        tb_effects = calc_tb_effect(debug)
        tb_effect_baseline = float(tb_effects[0])  # use the number of new infection from the 1st outbreak as a baseline
        new_infection_baseline = report_data_obj[KEY_NEW_INFECTIONS_GROUP[1]][timestep]
        statistical_population = report_data_obj[KEY_STATISTICAL_POPULATION_GROUP[1]][timestep]  # not any death
        new_infections = []
        expected_new_infections = []
        new_infections.append(new_infection_baseline)
        expected_new_infections.append(new_infection_baseline)
        actual_tb_effects = []
        actual_tb_effects.append(tb_effect_baseline)

        for i in range(1, len(Interventions)):  # no need to test the 1st outbreak
            timestep += outbreak_event_obj[sbvs.CampaignKeys.Timesteps_Between_Repetitions]
            new_infection = report_data_obj[KEY_NEW_INFECTIONS_GROUP[1]][timestep]
            tb_effect = tb_effects[i]
            # because expected_new_infection / (1.0 - tb_effect) = new_infection_baseline / (1.0- tb_effect_baseline),
            # so
            expected_new_infection = (1.0 - tb_effect) * new_infection_baseline / (1.0 - tb_effect_baseline)
            tolerance = 0.0 if expected_new_infection == 0.0 else 2e-2 * statistical_population
            actual_tb_effect = 1.0 - (new_infection * (1.0 - tb_effect_baseline) / new_infection_baseline)
            low_acceptance_bound = expected_new_infection - tolerance
            high_acceptance_bound = expected_new_infection + tolerance
            if math.fabs(new_infection - expected_new_infection) > tolerance:
                success = False
                outfile.write("BAD: At time step {0}, {1} reports {2} new infections, which is not within acceptance "
                              "range of ({3}, {4}).  Expected {5}.\n".format(timestep, KEY_NEW_INFECTIONS_GROUP[i],
                                                                             new_infection, low_acceptance_bound,
                                                                             high_acceptance_bound,
                                                                             expected_new_infection))
                outfile.write("Actual TransmissionBlocking effect is {0}, expected {1}.\n"
                              .format(actual_tb_effect, tb_effect))
            new_infections.append(new_infection)
            expected_new_infections.append(expected_new_infection)
            actual_tb_effects.append(actual_tb_effect)
        if success:
            outfile.write("GOOD: For all time steps, the new infections were within tolerance (within 2% of statistical"
                          " population) of the expected.\n")
        sft.plot_data(new_infections, expected_new_infections,
                      label1="Actual", label2="Expected",
                      xlabel="outbreak", ylabel="new infection",
                      title="Actual new infection vs. expected new infection",
                      category='New_infections', show=True)
        if debug:
            with open("DEBUG_new_infections.txt", "w") as file:
                for i in range(len(new_infections)):
                    file.write("{0}, {1}.\n".format(new_infections[i], expected_new_infections[i]))
            with open("DEBUG_effects.txt", "w") as file:
                for i in range(len(actual_tb_effects)):
                    file.write("{0}, {1}.\n".format(actual_tb_effects[i], tb_effects[i]))

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
    outbreak_event_obj = sbvs.parse_outbreak_event(campaign_filename, 0)
    report_data_obj = sbvs.parse_json_report(KEY_NEW_INFECTIONS_GROUP, KEY_STATISTICAL_POPULATION_GROUP,
                                             None, output_folder, propertyreport_name, debug)
    create_report_file(outbreak_event_obj, param_obj, report_data_obj, report_name, debug)


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
