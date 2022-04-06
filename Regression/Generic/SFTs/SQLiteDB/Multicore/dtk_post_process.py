#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
import dtk_test.dtk_RouteAwareSimpleVaccine_Support as RASV_support
from dtk_test.dtk_OutputFile import ReportEventRecorder

"""

This test is testing NewInfection and NewlySymptomatic events and IP information in SQLite3 db with multicore setting.

"""


def create_report_file(db_df, event_df, sqlit_db, report_event_recorder, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing {sqlit_db} with {report_event_recorder} in multicore setting.\n")
        simulation_duration = sft.get_simulation_duration(config_filename=config_filename)
        for t in range(simulation_duration):
            db_df_t = db_df[db_df["SIM_TIME"] == t]
            event_df_t = event_df[event_df[ReportEventRecorder.Column.Time.name] == t]
            for event_name in [ReportEventRecorder.Event.NewInfection.name,
                               ReportEventRecorder.Event.NewlySymptomatic.name]:
                db_df_t_event = db_df_t[db_df_t["EVENT"] == event_name]
                event_df_t_event = event_df_t[event_df_t[ReportEventRecorder.Column.Event_Name.name] == event_name]
                for index, row in event_df_t_event.iterrows():
                    ind_id = row[ReportEventRecorder.Column.Individual_ID.name]
                    g_id = row["Geographic"]
                    ip_pair = "Geographic=" + str(g_id)
                    if ind_id not in db_df_t_event["INDIVIDUAL"].tolist():
                        success = False
                        sft_report_file.write(f"BAD: at time {t}, {ind_id} not in the database for {event_name}.\n")
                    else:
                        if t != 0 and event_name == ReportEventRecorder.Event.NewInfection.name:
                            dp_ips = db_df_t_event[db_df_t_event["INDIVIDUAL"] == ind_id]["IPS"].tolist()[0]
                            if ip_pair not in dp_ips:
                                success = False
                                sft_report_file.write(f"BAD: at time {t}, {ind_id} has IPS: {dp_ips} , "
                                                      f"while the ip should be {ip_pair}.\n")
                        db_df_t_event = db_df_t_event[db_df_t_event["INDIVIDUAL"] != ind_id]
                if not db_df_t_event.empty:
                    success = False
                    sft_report_file.write(f"BAD: at time {t}, {len(db_df_t_event)} individuals "
                          f"({df_write_t_event['INDIVIDUAL'].tolist()}) is not in the ReportEventRecorder.csv "
                          f"for {event_name}.\n")

        sft_report_file.write(sft.format_success_msg(success))

    return success


def application(output_folder="output", stdout_filename="test.txt",
                report_event_recorder="ReportEventRecorder.csv",
                sqlit_db="simulation_events.db",
                config_filename="config.json", campaign_filename="campaign.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("report_event_recorder: " + report_event_recorder + "\n")
        print("sqlit_db: " + sqlit_db + "\n")
        print("config_filename: " + config_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    db_df = RASV_support.convert_sqlite_to_csv(db_file=sqlit_db)
    event_df = ReportEventRecorder(file=os.path.join(output_folder, "ReportEventRecorder.csv")).df

    create_report_file(db_df, event_df, sqlit_db, report_event_recorder, config_filename, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-R', '--report_event_recorder', default="ReportEventRecorder.csv",
                        help="ReportEventRecorder.csv to load (ReportEventRecorder.csv)")
    parser.add_argument('-l', '--sqlit_db', default="simulation_events.db",
                        help="SQLite db to load (simulation_events.db)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-C', '--campaign', default="campaign.json", help="campaign name to load (campaign.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                report_event_recorder=args.report_event_recorder, sqlit_db=args.sqlit_db,
                config_filename=args.config, campaign_filename=args.campaign,
                report_name=args.reportname, debug=args.debug)

