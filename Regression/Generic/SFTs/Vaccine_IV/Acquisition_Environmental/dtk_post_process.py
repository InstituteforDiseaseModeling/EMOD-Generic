#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )
import dtk_test.dtk_sft as sft
from dtk_test.dtk_InsetChart import InsetChart
import dtk_test.dtk_RouteAwareSimpleVaccine_Support as RASV_support


"""

Testing Vaccine Effect in Vaccine with AcquisitionBlocking and ENVIRONMENTAL

"""


def application(output_folder="output", stdout_filename="test.txt",
                property_report_name="PropertyReport.json",
                config_filename="config.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("property_report_name: " + property_report_name + "\n")
        print("config_filename: " + config_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    df = InsetChart(file=os.path.join(output_folder, property_report_name)).df

    RASV_support.create_report_file_acquisition_effect(df, property_report_name, config_filename, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-p', '--propertyreport', default="PropertyReport.json",
                        help="Property report to load (PropertyReport.json)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                property_report_name=args.propertyreport,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)

