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


KEY_NEW_INFECTIONS_GROUP = ["New Infections:QualityOfCare:1_Seed_Control",
                            "New Infections:QualityOfCare:2_Seed_Test",
                            "New Infections:QualityOfCare:3_Control",
                            "New Infections:QualityOfCare:4_Test"]

KEY_STATISTICAL_POPULATION_GROUP = ["Statistical Population:QualityOfCare:1_Seed_Control",
                                    "Statistical Population:QualityOfCare:2_Seed_Test",
                                    "Statistical Population:QualityOfCare:3_Control",
                                    "Statistical Population:QualityOfCare:4_Test"]

KEY_DISEASE_DEATHS_GROUP = ["Disease Deaths:QualityOfCare:1_Seed_Control",
                            "Disease Deaths:QualityOfCare:2_Seed_Test",
                            "Disease Deaths:QualityOfCare:3_Control",
                            "Disease Deaths:QualityOfCare:4_Test"]


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
                      "\n# Test compares the new vs. expected new infections in the Group 2_Seed_Test and the "
                      "Group 4_Test, and mortality blocking with reported vs. expected disease deaths.\n")
        success = True
        timestep = outbreak_event_obj[sbvs.CampaignKeys.Start_Day]
        effect = vax_event_obj[sbvs.CampaignKeys.Prime_Effect]
        new_infections = []
        statistical_populations = []
        disease_deaths = []
        for i in range(len(KEY_NEW_INFECTIONS_GROUP)):
            new_infection = report_data_obj[KEY_NEW_INFECTIONS_GROUP[i]][timestep]
            statistical_population = report_data_obj[KEY_STATISTICAL_POPULATION_GROUP[i]][timestep]
            # disease death in the last two groups happen 1 day later than the first two groups.
            disease_death = report_data_obj[KEY_DISEASE_DEATHS_GROUP[i]][int(timestep + i/2)]
            new_infections.append(new_infection)
            statistical_populations.append(statistical_population)
            disease_deaths.append(disease_death)
        # test acquisition blocking
        new_infection_seed_test = new_infections[1]
        statistical_population_seed_test = statistical_populations[1]
        expected_new_infection_seed_test = statistical_population_seed_test * (1.0 - effect)
        tolerance_1 = 0.0 if expected_new_infection_seed_test == 0.0 else 2e-2 * statistical_population_seed_test
        low_acceptance_bound_1 = expected_new_infection_seed_test - tolerance_1
        high_acceptance_bound_1 = expected_new_infection_seed_test + tolerance_1
        if math.fabs(new_infection_seed_test - expected_new_infection_seed_test) > tolerance_1:
            success = False
            outfile.write("BAD: At time step {0}, {1} reported new infections in Group 2_Seed_Test, which was not "
                          "within acceptance range of ({3}, {4}).  Expected {2}.\n"
                          .format(timestep, new_infection_seed_test, low_acceptance_bound_1,
                                  high_acceptance_bound_1, expected_new_infection_seed_test))
        if success:
            outfile.write("GOOD: For all time steps in Group 2_Seed_Test, the new infections were within tolerance "
                          "(within 2% of statistical population seed test) of the expected.\n")
        # test transmission blocking
        new_infection_seed_control = new_infections[0]
        new_infection_control = new_infections[2]
        new_infection_test = new_infections[3]
        expected_new_infection_test = (1.0 - effect) * new_infection_control * \
            new_infection_seed_test/float(new_infection_seed_control)
        statistical_population_test = statistical_populations[3]
        tolerance_2 = 0.0 if expected_new_infection_test == 0.0 else 2e-2 * statistical_population_test
        low_acceptance_bound_2 = expected_new_infection_test - tolerance_2
        high_acceptance_bound_2 = expected_new_infection_test + tolerance_2
        if math.fabs(new_infection_test - expected_new_infection_test) > tolerance_2:
            success = False 
            outfile.write("BAD: At time step {0}, {1} reported new infections in Group 4_Test, which was not "
                          "within acceptance range of ({2}, {3}).  Expected {4}.\n"
                          .format(timestep, new_infection_test, low_acceptance_bound_2, high_acceptance_bound_2,
                                  expected_new_infection_test))
        if success:
            outfile.write("GOOD: For all time steps in Group 4_Test, the new infections were within tolerance "
                          "(within 2% of statistical population test) of the expected.\n")
        #  test mortality blocking 
        disease_death_seed_test = disease_deaths[1]
        expected_disease_death_seed_test = new_infection_seed_test * (1.0 - effect)
        tolerance_3 = 0.0 if expected_disease_death_seed_test == 0.0 else 2e-2 * new_infection_seed_test
        low_acceptance_bound_3 = expected_disease_death_seed_test - tolerance_3
        high_acceptance_bound_3 = expected_disease_death_seed_test + tolerance_3
        if math.fabs(disease_death_seed_test - expected_disease_death_seed_test) > tolerance_3:
            success = False
            outfile.write("BAD: At time step {0}, {1} reported disease deaths in Group 2_Seed_Test, which was not "
                          "within acceptance range of ({2}, {3}). Expected {4}.\n"
                          .format(timestep, disease_death_seed_test, low_acceptance_bound_3, high_acceptance_bound_3,
                                  expected_disease_death_seed_test))
        if success:
            outfile.write("GOOD: For all time steps in Group 2_Seed_Test, the new infections were within tolerance "
                          "(within 2% of new_infection_seed_test) of the expected.\n")
        outfile.write(sft.format_success_msg(success))
    sft.plot_data(new_infections, disease_deaths,
                  label1="new_infections", label2="disease_death",
                  xlabel="0:1_Seed_Control, 1:2_Seed_Test, 2:3_Control, 4:3_Test",
                  title="new_infections vs. disease_death",
                  category='New_infections_vs_disease_death', show=True)
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
    vax_event_obj = sbvs.parse_vax_event(campaign_filename, 0)
    outbreak_event_obj = sbvs.parse_outbreak_event(campaign_filename, 2)
    report_data_obj = sbvs.parse_json_report(KEY_NEW_INFECTIONS_GROUP, KEY_STATISTICAL_POPULATION_GROUP,
                                             KEY_DISEASE_DEATHS_GROUP, output_folder, propertyreport_name, debug)
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
