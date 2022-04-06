#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
import dtk_test.dtk_RouteAwareSimpleVaccine_Support as RASV_support

"""

This test is making sure SQLite3 db works with Serialization.

"""

def create_report_file(df_write, df_load, sqlit_db_write, sqlit_db, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        simulation_start_time, simulation_duration = sft.get_config_parameter(config_filename,
                                                                              ["Start_Time",
                                                                               "Simulation_Duration"])

        success = True

        sft_report_file.write(f"Testing {sqlit_db} with {sqlit_db_write}.\n")

        # adjust simulation time on both db.
        df_write = df_write[df_write["SIM_TIME"] >= simulation_start_time]
        #df_load["SIM_TIME"] = df_load["SIM_TIME"] + simulation_start_time
        df_load = df_load[(df_load["SIM_TIME"] <= simulation_start_time + simulation_duration - 1) &
                          (df_load["SIM_TIME"] >= simulation_start_time)]

        # drop index column
        compare_df_write = df_write.reset_index().drop('index', axis=1)
        compare_df_load = df_load.reset_index().drop('index', axis=1)
        if compare_df_write.equals(compare_df_load):
            sft_report_file.write("GOOD: SQLite works fine with Serialization.\n")
        else:
            success = False
            df1 = compare_df_write.merge(compare_df_load, how='outer', indicator=True).loc[
                lambda x: x['_merge'] == 'left_only']
            df2 = compare_df_write.merge(compare_df_load, how='outer', indicator=True).loc[
                lambda x: x['_merge'] == 'right_only']

            sft_report_file.write("BAD: there is bug in SQLite db with Serialization.\n")
            sft_report_file.write(f"\tThere are data in {sqlit_db_write} but not in {sqlit_db}, please see df1.csv.\n")
            df1.to_csv('df1.csv', index=False)
            sft_report_file.write(f"\tThere are data in {sqlit_db} but not in {sqlit_db_write}, please see df2.csv.\n")
            df2.to_csv('df2.csv', index=False)

        sft_report_file.write(sft.format_success_msg(success))

    return success

def application(output_folder="output", stdout_filename="test.txt",
                sqlit_db_write="dtk_simulation_events.db",
                sqlit_db="simulation_events.db",
                config_filename="config.json", campaign_filename="campaign.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("sqlit_db_write: " + sqlit_db_write + "\n")
        print("sqlit_db: " + sqlit_db + "\n")
        print("config_filename: " + config_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    if not os.path.isfile(sqlit_db_write):
        dtk_file = sqlit_db_write.replace('.db', '.dtk')
        if os.path.isfile(dtk_file):
            os.rename(dtk_file, sqlit_db_write)
        else:
            raise FileNotFoundError(f"{sqlit_db_write} not found.")

    df_write = RASV_support.convert_sqlite_to_csv(db_file=sqlit_db_write)
    df_load = RASV_support.convert_sqlite_to_csv(db_file=sqlit_db)

    create_report_file(df_write, df_load, sqlit_db_write, sqlit_db, config_filename, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-w', '--sqlit_db_write', default="dtk_simulation_events.db",
                        help="SQLite db to be compared to (dtk_simulation_events.db)")
    parser.add_argument('-l', '--sqlit_db', default="simulation_events.db",
                        help="SQLite db to load (simulation_events.db)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-C', '--campaign', default="campaign.json", help="campaign name to load (campaign.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                sqlit_db_write=args.sqlit_db_write, sqlit_db=args.sqlit_db,
                config_filename=args.config, campaign_filename=args.campaign,
                report_name=args.reportname, debug=args.debug)

