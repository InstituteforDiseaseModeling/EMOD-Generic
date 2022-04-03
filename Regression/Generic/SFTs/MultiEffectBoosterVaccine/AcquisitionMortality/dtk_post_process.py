#!/usr/bin/python

import dtk_test.dtk_sft as sft
import dtk_test.dtk_MultiEffectBoosterVaccine_Support as mbvs
import json
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
MultiEffectBoosterVaccine is derived from MultiEffectVaccine, and preserves many of the same parameters.
The behavior is much like MultiEffectVaccine, except that upon distribution and successful take,
the vaccine's effect is determined by the recipient's immune state. If the recipient's immunity
modifier in the corresponding channel (Acquisition, Transmission, Mortality) is above a user-specified
threshold (Boost_Threshold), then the vaccine's initial effect will be equal to the parameter
Prime_Effect. If the recipient's immune modifier is below this threshold, then the vaccine's
initial effect will be equal to the parameter Boost_Effect. After distribution, the effect evolves
according to the Waning_Config, just like a Simple Vaccine. In essence, this intervention provides
a MultiEffectVaccine with at least one effect to all naive (below- threshold) individuals, and at least one effect to
all primed (above-threshold) individuals; this behavior is intended to mimic, to some degree,
biological priming and boosting.
All parameters necessary to define a MultiEffectVaccine are also necessary for a MultiEffectBoosterVaccine
(though the Initial_Effect of the Waning_Config will be overwritten by one of Prime_Effect or
Boost_Effect upon distribution).
"""


def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                propertyreport_name="PropertyReport.json",
                report_name=sft.sft_output_filename,
                debug=False):

    param_obj = mbvs.load_emod_parameters(config_filename)
    sft.wait_for_done()
    campaign_filename = param_obj[mbvs.ConfigKeys.CAMPAIGN_NAME]
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("propertyreport_name: " + propertyreport_name + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")
    vax_event_obj = mbvs.parse_vax_event(campaign_filename, 0, debug)
    outbreak_event_obj = mbvs.parse_outbreak_event(campaign_filename, 1, debug)
    report_data_obj = mbvs.parse_json_report(KEY_NEW_INFECTIONS_GROUP, KEY_STATISTICAL_POPULATION_GROUP,
                                             KEY_DISEASE_DEATHS_GROUP, output_folder, propertyreport_name, debug)
    mbvs.create_report_file(vax_event_obj, outbreak_event_obj, param_obj, report_data_obj, report_name,
                            KEY_NEW_INFECTIONS_GROUP, KEY_STATISTICAL_POPULATION_GROUP, KEY_DISEASE_DEATHS_GROUP,
                            True, False, True, debug)


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
