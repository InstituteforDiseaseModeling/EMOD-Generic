#!usr/bin/python
from __future__ import division
import dtk_test.dtk_sft as sft
import dtk_test.dtk_MeaslesVaccine_Support as mvs


"""
MeaslesVaccine is derived from SimpleVaccine, and preserves many of the same parameters.
The behavior is much like SimpleVaccine, except that it is age-dependent.  After distribution, the effect evolves
according to the Waning_Config, just like a Simple Vaccine.
All parameters necessary to define a SimpleVaccine are also necessary for a MeaslesVaccine
"""


def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done()
    param_obj = mvs.load_emod_parameters(config_filename, debug=debug)
    campaign_filename = param_obj[mvs.ConfigKeys.CAMPAIGN_FILENAME]
    campaign_obj = mvs.load_campaign_file(campaign_filename, debug=debug)
    mvs.create_report_file(param_obj, campaign_obj, report_name, stdout_filename, debug=debug)


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
