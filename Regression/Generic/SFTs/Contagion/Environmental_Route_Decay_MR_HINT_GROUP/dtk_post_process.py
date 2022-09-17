#!/usr/bin/python

if __name__ == '__main__':
    import os
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
import dtk_test.dtk_General_Support as General_Support
from dtk_test.dtk_General_Support import ConfigKeys, DemographicsKeys, InsetKeys
import dtk_test.dtk_HINT_Support as hint_support
import dtk_test.dtk_Environmental_Contagion_HINT_Support as ECH_support


def application(output_folder="output", stdout_filename="test.txt",
                property_report_name="PropertyReport.json",
                insetchart_name="InsetChart.json",
                config_filename="config.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("property_report_name: " + property_report_name + "\n")
        print("insetchart_name: " + insetchart_name + "\n")
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    config_obj = hint_support.load_config_parameters(config_filename, ECH_support.config_keys, debug)

    demo_path = "Assets" if stdout_filename == "StdOut.txt" else ""
    property_list = hint_support.load_demo_mr_overlay_file(
        config_obj[ConfigKeys.Demographics_Filenames][-1], demo_path, debug)
    property_keys = hint_support.build_channel_string_for_property(property_list, ECH_support.channels, debug)
    property_df = hint_support.parse_property_report_json(property_report_name, output_folder, property_keys, debug)
    property_obj = property_list[0] # this test only has one property object

    inset_chart_obj = General_Support.parse_inset_chart(output_folder, insetchart_name, ECH_support.inset_channels, debug)
    ECH_support.create_report_file(config_obj, property_df, property_obj, inset_chart_obj, insetchart_name, property_report_name,
                       report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-p', '--propertyreport', default="PropertyReport.json",
                        help="Property report to load (PropertyReport.json)")
    parser.add_argument('-j', '--insetchart', default="InsetChart.json",
                        help="json report to load (InsetChart.json)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                property_report_name=args.propertyreport, insetchart_name=args.insetchart,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)

