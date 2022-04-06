#!/usr/bin/python
from collections import Counter, OrderedDict
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

This test is testing the LABEL column in SQLite3 db, which should be the ID of the infector/spreader. In this test, the 
individuals in group 1(1% of the individual) have 1000 times more infectious than the other group, so they become the 
super spreaders. 

"""

super_spreader_group = 1

def create_report_file(db_df, event_df, sqlit_db, report_event_recorder, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing {sqlit_db} with {report_event_recorder}.\n")

        event_df_newinfection = event_df[
            event_df[ReportEventRecorder.Column.Event_Name.name] == ReportEventRecorder.Event.NewInfection.name]
        event_newinfection_id_list = event_df_newinfection[ReportEventRecorder.Column.Individual_ID.name].tolist()
        db_df_misc = db_df[db_df["LABEL"] != 0]
        infector_dict = dict()
        infection_time_dict = dict()
        for index, row in db_df_misc.iterrows():
            # sft_report_file.write(f"Index: {index} Row: {row}\n")
            ind_id = row["INDIVIDUAL"]
            infector = row["LABEL"]
            t = row["SIM_TIME"]
            # Individuals should not infect themselves
            if ind_id == infector:
                success = False
                sft_report_file.write(f"BAD: infector {infector} infected individual {ind_id}, "
                                      f"they should not be the same person.\n")
            # Infector should be infected
            if infector not in event_newinfection_id_list:
                success = False
                sft_report_file.write(f"BAD: infector {infector} never get infected, so he should not be an "
                                      f"infector of {ind_id}.\n")
            elif t > 0:
                ip_pair = row["IPS"].split("|")[0]
                property_id, group_id = ip_pair.split("=")
                infector_df = event_df_newinfection[event_df_newinfection[
                                                        ReportEventRecorder.Column.Individual_ID.name] == infector]
                # with open("DEBUG_infector_dataframe.csv","w") as outfile:
                #     infector_df.to_csv(outfile)
                infector_group_id = infector_df[property_id].iloc[0]
                # Infector should be from the group 1 most of the times.
                if infector_group_id != super_spreader_group:
                    # success = False
                    sft_report_file.write(f"WARNING: infector {infector} is from group {infector_group_id} not from "
                                          f"group {super_spreader_group}, he should be less likely to infect individual"
                                          f" {ind_id}.\n")

                infector_infection_time = infector_df[ReportEventRecorder.Column.Time.name].iloc[0]
                # Infector should be infected before the current time step
                if infector_infection_time >= t:
                    success = False
                    sft_report_file.write(f"BAD: infector {infector} infects individual {ind_id} at time {t} before "
                                          f"he got infected at time {infector_infection_time}.\n")

                if infector_group_id not in infector_dict:
                    infector_dict[infector_group_id] = [infector]
                else:
                    infector_dict[infector_group_id].append(infector)
                if infector not in infection_time_dict:
                    infection_time_dict[infector] = infector_infection_time

        super_spreader_frequency = none_super_spreader_frequency=0
        with open("DEBUG_infector.txt", "w") as file:
            for group_id, infectors in infector_dict.items():
                if group_id == super_spreader_group:
                    super_spreader_frequency = len(infectors)
                else:
                    none_super_spreader_frequency += len(infectors)

                file.write(f"group: {group_id}: {len(infectors)}\n")
                test = Counter(infectors)
                sorted_test = OrderedDict(sorted(test.items(), key=lambda kv: kv[1], reverse=True))
                for infector in sorted_test.keys():
                    infection_time = infection_time_dict[infector]
                    file.write(f"{infector}, {sorted_test[infector]}, {infection_time}\n")
                file.write(f"end group: {group_id}.\n")

        msg = f"There are {super_spreader_frequency} incidences caused by individuals from super spreader group " \
              f"{super_spreader_group}, while there are {none_super_spreader_frequency} incidences caused by " \
              f"individuals from other groups.\n"
        if none_super_spreader_frequency / super_spreader_frequency > 5e-2:
            success = False
            sft_report_file.write("BAD: " + msg)
        else:
            sft_report_file.write("GOOD: " + msg)

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

