#!/usr/bin/python

if __name__ == '__main__':
    import os
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
import dtk_test.dtk_RouteAwareSimpleVaccine_Support as RASV_support


"""
Testing TransmissionBlocking in RouteAwareSimpleVaccine with both TRANSMISSIONROUTE_ENVIRONMENTAL and 
TRANSMISSIONROUTE_CONTACT Vaccine_Route.
"""


def application(output_folder="output", stdout_filename="test.txt",
                config_filename="config.json",
                event_db_filename="simulation_events.db",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("event_db_filename: " + event_db_filename + "\n")
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    db_df = RASV_support.convert_sqlite_to_csv(db_file=event_db_filename)
    # Get the NewInfections event by Transmission and Contact routes only.
    db_df = db_df[~db_df[RASV_support.Columns.notes].astype(str).str.contains("OUTBREAK")]

    RASV_support.create_report_file_vaccine_route(db_df, event_db_filename, config_filename, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-e', '--event_db', default="simulation_events.db",
                        help="SQLite db to load (simulation_events.db)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-C', '--campaign', default="campaign.json", help="campaign name to load (campaign.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                event_db_filename=args.event_db,
                config_filename=args.config,
                report_name=args.reportname, debug=args.debug)

