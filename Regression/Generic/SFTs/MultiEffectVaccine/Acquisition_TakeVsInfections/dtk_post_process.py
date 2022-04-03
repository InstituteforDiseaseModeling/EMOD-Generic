#!usr/bin/python
from __future__ import division
import dtk_test.dtk_sft as sft
import dtk_test.dtk_MultiEffectVaccine_Support as mvs


"""
MultiEffectVaccine allows a vaccine to have Acquisition, Transmission, and/or Mortality blocking effects.  In this test,
we are only testing Acquisition (Mortality and Transmission parameters in the config file aren't affecting test
behavior).  We expect the MultiEffectVaccine's efficacy to be the InitialEffect multiplied by the VaccineTake, and the
number of new infections to reflect this efficacy rate, and test this.  
From this, that means there are individuals whose VaccineTake does not match whether or not they were uninfected if 
InitialEffect is not 1, so we test if this number of individuals is equal to the expected number.
"""


def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                report_name=sft.sft_output_filename,
                insetchart_name="InsetChart.json",
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("insetchart_name: " + insetchart_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done()
    param_obj = mvs.load_emod_parameters(config_filename, debug=debug)
    campaign_filename = param_obj[mvs.ConfigKeys.CAMPAIGN_FILENAME]
    campaign_obj = mvs.load_campaign_file(campaign_filename, debug=debug)
    report_data_obj = mvs.parse_json_report(output_folder, insetchart_name, debug=debug)
    outbreak_event_obj = mvs.parse_outbreak_event(campaign_filename, 1, debug=debug)
    mvs.create_report_file(param_obj, campaign_obj, report_name, stdout_filename, debug=debug,
                           report_data_obj=report_data_obj, outbreak_event_obj=outbreak_event_obj)


if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="log file)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename,
                        help="report name (scientific_feature_report.txt)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = parser.parse_args()

    application(output_folder=args.output,
                config_filename=args.config,
                stdout_filename=args.stdout,
                report_name=args.reportname,
                debug=args.debug)
