#!/usr/bin/python
import os
import pandas as pd
import matplotlib
from sys import platform
if platform == "linux" or platform == "linux2":
    print('Linux OS. Using non-interactive Agg backend')
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
import dtk_test.dtk_RouteAwareSimpleVaccine_Support as RASV_support
from dtk_test.dtk_OutputFile import ReportEventRecorder

"""

This test is testing the InfectionCleared and DiseaseDeaths events in SQLite3 db. These events should match the information in 
ReportEventRecorder.csv. They need to be raised after their NewInfection events. The Recover events should be raised 
when the infection timers reach 0 and the DiseaseDeaths event should be raised before the infection timer ends. 



"""

def create_report_file(db_df, event_df, sqlit_db, report_event_recorder, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing {sqlit_db} with {report_event_recorder}.\n")
        simulation_duration = sft.get_simulation_duration(config_filename=config_filename)
        for t in range(simulation_duration):
            db_df_t = db_df[db_df["SIM_TIME"] == t]
            event_df_t = event_df[event_df[ReportEventRecorder.Column.Time.name] == t]
            for event_name in [ReportEventRecorder.Event.NewInfection.name,
                               ReportEventRecorder.Event.InfectionCleared.name,
                               ReportEventRecorder.Event.DiseaseDeaths.name]:

                db_event_name = event_name
                db_df_t_event = db_df_t[db_df_t["EVENT"] == db_event_name]
                event_df_t_event = event_df_t[event_df_t[ReportEventRecorder.Column.Event_Name.name] == event_name]
                for index, row in event_df_t_event.iterrows():
                    ind_id = row[ReportEventRecorder.Column.Individual_ID.name]
                    if ind_id not in db_df_t_event["INDIVIDUAL"].tolist():
                        success = False
                        sft_report_file.write(f"\tBAD: at time {t}, {ind_id} not in the database for {event_name}"
                                              f"({db_event_name}).\n")
                    else:
                        # Remove the individual if we already check it.
                        db_df_t_event = db_df_t_event[db_df_t_event["INDIVIDUAL"] != ind_id]
                # Make sure it's 1 to 1 matching
                if not db_df_t_event.empty:
                    success = False
                    sft_report_file.write(f"\tBAD: at time {t}, {len(db_df_t_event)} individuals "
                          f"({df_write_t_event['INDIVIDUAL'].tolist()}) is not in the ReportEventRecorder.csv "
                          f"for {event_name}({db_event_name}).\n")

        infectious_period, infectious_period_distribution, incubation_period, incubation_period_distribution = \
            sft.get_config_parameter(config_filename, ["Infectious_Period_Constant", "Infectious_Period_Distribution",
                                                       "Incubation_Period_Constant", "Incubation_Period_Distribution"])
        sft_report_file.write(f"  Infectious_Period_Distribution = {infectious_period_distribution}\n"
                              f"  Infectious_Period_Constant = {infectious_period}\n"
                              f"  Incubation_Period_Distribution = {incubation_period_distribution}\n"
                              f"  Incubation_Period_Constant = {incubation_period}\n")

        if infectious_period_distribution != "CONSTANT_DISTRIBUTION" or \
                incubation_period_distribution != "CONSTANT_DISTRIBUTION":
            success = False
            sft_report_file.write(f"\tBAD: Infectious_Period_Distribution is set to {infectious_period_distribution} and "
                                  f"Incubation_Period_Distribution is set to {incubation_period_distribution}, "
                                  f"please set them to CONSTANT_DISTRIBUTION.\n")
        else:
            if infectious_period + incubation_period >= simulation_duration:
                success = False
                sft_report_file.write(
                    f"\tBAD: Infectious_Period_Constant and Incubation_Period_Constant are set to {infectious_period} "
                    f"and {incubation_period}, the sum of them is longer than "
                    f"Simulation_Duration({simulation_duration}), please set them to shorter durations.\n")
            else:
                expected_recovered_duration    = infectious_period + incubation_period + 1 # Off by one
                expected_recovered_duration_ob = infectious_period
                sft_report_file.write(f"Making sure {ReportEventRecorder.Event.InfectionCleared.name} is "
                                      f"{expected_recovered_duration} days or {expected_recovered_duration_ob}"
                                      f"days and {ReportEventRecorder.Event.DiseaseDeaths.name} is less than "
                                      f"{expected_recovered_duration} days after "
                                      f"{ReportEventRecorder.Event.NewInfection.name} in {sqlit_db}.\n")
                # Filter data frame for events to test.
                db_events_in_order = [ReportEventRecorder.Event.InfectionCleared.name,
                                      ReportEventRecorder.Event.NewInfection.name,
                                      ReportEventRecorder.Event.DiseaseDeaths.name]
                db_df_test = db_df[db_df["EVENT"].isin(db_events_in_order)]
                for event in db_events_in_order:
                    num_of_event = len(db_df_test[db_df_test["EVENT"]==event])
                    if num_of_event < 5:
                        success = False
                        sft_report_file.write(f"\tBAD: there are only {num_of_event} {event} events"
                                              f" in {sqlit_db}, not enough data to test.\n")
                    else:
                        sft_report_file.write(f"\tGOOD: there are {num_of_event} {event} events"
                                              f" in {sqlit_db}.\n")

                # Reorder the data frame
                sft_report_file.write(f"\tSort test data by {db_events_in_order} in each simulation time step"
                                      f" in {sqlit_db}.\n")
                db_df_test['EVENT'] = pd.Categorical(db_df_test['EVENT'], db_events_in_order)
                db_df_test = db_df_test.sort_values(["SIM_TIME", "EVENT"], ascending=(True, True))

                new_infection_dictionary = dict()
                disease_death_dictionary = dict()
                for index, row in db_df_test.iterrows():
                    id = row["INDIVIDUAL"]
                    event = row["EVENT"]
                    time = row["SIM_TIME"]
                    if event == ReportEventRecorder.Event.NewInfection.name:
                        if id in disease_death_dictionary:
                            success = False
                            sft_report_file.write(f"\tBAD: individual {id} is already dead, he/she should not be "
                                                  f"infected at day {time}.\n")
                        else:
                            if id not in new_infection_dictionary:
                                new_infection_dictionary[id] = time
                            else:
                                success = False
                                sft_report_file.write(f"\tBAD: individual {id} is already infected since day "
                                                      f"{new_infection_dictionary[id]} and have not cleared infection since "
                                                      f"then, he/she should not be infected at day {time} again.\n")
                    elif event == ReportEventRecorder.Event.InfectionCleared.name:
                        if id in new_infection_dictionary:
                            infection_time = new_infection_dictionary[id]
                            if((time - infection_time != expected_recovered_duration) and
                               (time - infection_time != expected_recovered_duration_ob)):
                                success = False
                                sft_report_file.write(f"\tBAD: individual {id} got infected at day {infection_time} and "
                                                      f"cleared infection at day {time}, the infectious duration is "
                                                      f"{time - infection_time} days, expected "
                                                      f"{expected_recovered_duration} days.\n")
                            new_infection_dictionary.pop(id)
                        else:
                            success = False
                            sft_report_file.write(f"\tBAD: individual {id} cleared infection at day {time} before "
                                                  f"being infected.\n")
                    else: # elif event == ReportEventRecorder.Event.DiseaseDeaths.name:
                        if id in new_infection_dictionary:
                            infection_time = new_infection_dictionary[id]
                            if time - infection_time >= expected_recovered_duration:
                                success = False
                                sft_report_file.write(
                                    f"\tBAD: individual {id} got infected at day {infection_time} and "
                                    f"died of disease at day {time}, he/she should clear infection at day "
                                    f"{infection_time + expected_recovered_duration} days before he/she died"
                                    f" of disease.\n")
                            new_infection_dictionary.pop(id)
                            if id not in disease_death_dictionary:
                                disease_death_dictionary[id] = time - infection_time
                            else:
                                success = False
                                sft_report_file.write(
                                    f"\tBAD: individual {id} died of disease at day {time}, but he/she was already dead.\n")

                        else:
                            success = False
                            sft_report_file.write(f"\tBAD: individual {id} died of disease at day {time} before "
                                                  f"being infected.\n")

                sft_report_file.write(f"Make sure no one doesn't die or recover after {expected_recovered_duration} days.\n")
                for id, infection_time in new_infection_dictionary.items():
                    if simulation_duration - infection_time > expected_recovered_duration:
                        success = False
                        sft_report_file.write(f"\tBAD: individual {id} didn't die or recover after "
                                              f"{expected_recovered_duration} days of infection.\n")

                sft_report_file.write(f"Plotting disease death durations.\n")
                plot_disease_death_duration(disease_death_dictionary)

        sft_report_file.write(sft.format_success_msg(success))

    return success


def plot_disease_death_duration(disease_death_dictionary):
    disease_death_durations = disease_death_dictionary.values()
    num_bins = len(set(disease_death_durations))
    # the histogram of the data
    plt.hist(disease_death_durations, num_bins, density=True, facecolor='blue', alpha=0.5)

    plt.xlabel('Duration')
    plt.ylabel('Probability')
    plt.title(r'Disease Death Duration')

    # Tweak spacing to prevent clipping of ylabel
    plt.subplots_adjust(left=0.15)
    plt.savefig(f"disease_death_durations.png")
    if sft.check_for_plotting():
        plt.show()
    plt.close()


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

