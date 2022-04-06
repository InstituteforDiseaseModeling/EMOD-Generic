#!/usr/bin/python
import os
import json
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
    sys.path.append( str(Path('../../../shared_embedded_py_scripts/dtk_test').resolve().absolute()) )

import dtk_test.dtk_sft as sft
from dtk_test.dtk_OutputFile import ReportEventRecorder

"""
The Outbreak interventions are taken out in the campaign.json since we decided to disable Max_Distributions_Per_Node for
 NodeTargeted intervention. Please see https://github.com/InstituteforDiseaseModeling/covid_emod_hintschoolclosure/issues/81.
The original dtk_post_process.py script is writen as it could work with Outbreak interventions, but it should still
work with the updated campaign.json. 
------------------------------------------------------------------------
This test is testing the Max_Distributions_Per_Node parameter in StandardInterventionDistributionEventCoordinator with 
IndividualTargeted intervention OutbreakIndividual and NodeTargeted intervention Outbreak. 

"""

class Campaign:
    Max_Distributions_Per_Node = "Max_Distributions_Per_Node"
    Events = "Events"
    Event_Coordinator_Config = "Event_Coordinator_Config"
    Intervention_Config = "Intervention_Config"
    class_name = "class"
    Target_Demographic = "Target_Demographic"
    Everyone = "Everyone"
    ExplicitAgeRanges = "ExplicitAgeRanges"
    Target_Age_Min = "Target_Age_Min"
    Target_Age_Max = "Target_Age_Max"
    ExplicitGender = "ExplicitGender"
    Target_Gender = "Target_Gender"
    Male = "Male"
    Female = "Female"
    OutbreakIndividual = "OutbreakIndividual"
    Outbreak = "Outbreak"
    Number_Cases_Per_Node = "Number_Cases_Per_Node"
    Import_Age = "Import_Age"
    Nodeset_Config = "Nodeset_Config"
    NodeSetAll = "NodeSetAll"
    NodeSetNodeList = "NodeSetNodeList"
    Node_List = "Node_List"
    Start_Day = "Start_Day"


class Demo:
    Nodes = "Nodes"
    NodeID = "NodeID"
    NodeAttributes = "NodeAttributes"
    InitialPopulation = "InitialPopulation"


def load_demo_file(demo_filename):
    """
        Read InitialPopulation and NodeIDs from demographic file.
    Args:
        demo_filename:

    Returns: node_population, node_list

    """
    with open(demo_filename, 'r') as demo_file:
        demo = json.load(demo_file)
        try:
            nodes = demo[Demo.Nodes]
            node_list = []
            node_population = None
            for node in nodes:
                node_id = node[Demo.NodeID]
                node_list.append(node_id)
                cur_node_population = node[Demo.NodeAttributes][Demo.InitialPopulation]
                if node_population:
                    if cur_node_population != node_population:
                        raise ValueError(f"Every node should have the same {Demo.InitialPopulation}, please fix the "
                                         f"{demo_filename}.\n")
                else:
                    node_population = cur_node_population

            return node_population, node_list

        except KeyError as ke:
            raise KeyError(f"{ke} is not found in {demo_filename}.\n")


def load_campaign(campaign_filename):
    """
        Load Max_Distributions_Per_Node, start_day, intervention class name, target node ids and target demographic from
    Outbreak(includes Number_Cases_Per_Node and Import_Age) and OutbreakIndividual interventions.
    Args:
        campaign_filename:

    Returns:

    """
    with open(campaign_filename, 'r') as campaign_file:
        cf = json.load(campaign_file)
        try:
            events = cf[Campaign.Events]
            res = dict()
            for event in events:
                # Load start_day
                start_day = event[Campaign.Start_Day]
                if start_day in res:
                    raise ValueError(f"There are multiple intervention at day {start_day}, please make sure there is "
                                     f"only one intervention at a time.\n")
                else:
                    res[start_day] = dict()

                ecc = event[Campaign.Event_Coordinator_Config]
                # Load Max_Distributions_Per_Node
                res[start_day][Campaign.Max_Distributions_Per_Node] = ecc[Campaign.Max_Distributions_Per_Node] if \
                Campaign.Max_Distributions_Per_Node in ecc else 2.14748e+09
                # Load Nodeset_Config
                nodeset_config = event[Campaign.Nodeset_Config][Campaign.class_name]
                if nodeset_config == Campaign.NodeSetAll:
                    res[start_day][Campaign.Nodeset_Config] = []
                else:
                    res[start_day][Campaign.Nodeset_Config] = event[Campaign.Nodeset_Config][Campaign.Node_List]
                # Load intervention
                ic = ecc[Campaign.Intervention_Config]
                if ic[Campaign.class_name] == Campaign.OutbreakIndividual:
                    res[start_day][Campaign.class_name] = Campaign.OutbreakIndividual
                    # Load Target_Demographic
                    target_demographic = ecc[Campaign.Target_Demographic]
                    if target_demographic == Campaign.ExplicitAgeRanges:
                        min_age = ecc[Campaign.Target_Age_Min]
                        max_age = ecc[Campaign.Target_Age_Max]
                        res[start_day][Campaign.Target_Demographic] = [min_age, max_age]
                    elif target_demographic == Campaign.ExplicitGender:
                        gender = ecc[Campaign.Target_Gender]
                        res[start_day][Campaign.Target_Demographic] = gender
                    else:
                        res[start_day][Campaign.Target_Demographic] = Campaign.Everyone
                elif ic[Campaign.class_name] == Campaign.Outbreak:
                    res[start_day][Campaign.class_name] = Campaign.Outbreak
                    res[start_day][Campaign.Import_Age] = ic[Campaign.Import_Age]
                    res[start_day][Campaign.Number_Cases_Per_Node] = ic[Campaign.Number_Cases_Per_Node]
            return res

        except KeyError as ke:
            raise KeyError(f"{ke} is not found in {campaign_filename}.\n")


def compare_num_of_cases(new_infection_t_node, num_of_cases, start_day, sft_report_file, intervention_name, node_id,
                         target_demographic=None):
    result = True
    actual_num_of_case = len(new_infection_t_node)
    msg = f"the {intervention_name} should infect {num_of_cases} {target_demographic} individuals at day {start_day} " \
          f"in node {node_id}, it actually infect {actual_num_of_case} {target_demographic} individuals."
    if num_of_cases != actual_num_of_case:
        result = False
        sft_report_file.write(f"\tBAD: {msg}.\n")
    else:
        sft_report_file.write(f"\tGOOD: {msg} \n")
    return result


def create_report_file(event_df, report_event_recorder, config_filename, campaign_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        demo_file = sft.get_config_parameter(config_filename, ["Demographics_Filenames"])[0][-1]
        sft_report_file.write(f"Loading node lists and initial individual from {demo_file}:\n")
        node_population, node_list = load_demo_file(demo_file)
        sft_report_file.write(f"We have nodes: {node_list} and each node has {node_population} individuals.\n")

        sft_report_file.write(f"Loading interventions from {campaign_filename}:\n")
        campaign_obj = load_campaign(campaign_filename)
        if not len(campaign_obj):
            success = False
            sft_report_file.write(f"BAD: There is no intervention under test in {campaign_filename}.\n")

        sft_report_file.write(f"Testing data from {report_event_recorder}:\n")
        for start_day, intervention in campaign_obj.items():
            sft_report_file.write(f"start_day = {start_day}, intervention = {intervention}: \n")
            intervention_name = intervention[Campaign.class_name]
            target_nodes = intervention[Campaign.Nodeset_Config]
            max_distributions_per_node = intervention[Campaign.Max_Distributions_Per_Node]
            if not len(target_nodes):
                target_nodes = node_list

            new_infection_t = event_df[(event_df[ReportEventRecorder.Column.Time.name] == start_day) &
                                       (event_df[ReportEventRecorder.Column.Event_Name.name] ==
                                        ReportEventRecorder.Event.NewInfection.name)]
            # Main test
            # Make sure the actual number of cases matched the expected numbers in each node. Based on the Target_Demographic
            # configuration and type of intervention that we are testing, perform extra testing in each node.
            if intervention_name == Campaign.Outbreak:
                if max_distributions_per_node == 0:
                    num_of_cases = 0
                else:
                    num_of_cases = intervention[Campaign.Number_Cases_Per_Node]
                import_age = intervention[Campaign.Import_Age] + 1
                for node_id in target_nodes:
                    new_infection_t_node = new_infection_t[new_infection_t[ReportEventRecorder.Column.Node_ID.name] == node_id]
                    # main test
                    if not compare_num_of_cases(new_infection_t_node, num_of_cases, start_day, sft_report_file,
                                                intervention_name, node_id):
                        success = False

                    # make sure import age is honored.
                    if num_of_cases and set(new_infection_t_node[ReportEventRecorder.Column.Age.name]) != set([import_age]):
                        success = False
                        sft_report_file.write(f"\tBAD: the {Campaign.Import_Age} should be {import_age} for "
                                              f"{Campaign.Outbreak} at day {start_day} in node {node_id}.\n")
                    # Remove data from tested node id
                    new_infection_t = new_infection_t[new_infection_t[ReportEventRecorder.Column.Node_ID.name] != node_id]

            elif intervention_name == Campaign.OutbreakIndividual:
                target_demographic = intervention[Campaign.Target_Demographic]
                if target_demographic == Campaign.Everyone:
                    num_of_cases = min(node_population, max_distributions_per_node)
                    for node_id in target_nodes:
                        new_infection_t_node = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] == node_id]
                        # main test
                        if not compare_num_of_cases(new_infection_t_node, num_of_cases, start_day, sft_report_file,
                                                    intervention_name, node_id):
                            success = False
                        # Remove data from tested node id
                        new_infection_t = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] != node_id]
                elif target_demographic == Campaign.Male or target_demographic == Campaign.Female:
                    if target_demographic == Campaign.Male:
                        gender = "M"
                        the_other_gender = Campaign.Female
                    else:
                        gender = "F"
                        the_other_gender = Campaign.Male

                    for node_id in target_nodes:
                        all_individuals_t_node = event_df[(event_df[ReportEventRecorder.Column.Time.name] == start_day) &
                                   (event_df[ReportEventRecorder.Column.Event_Name.name] ==
                                    ReportEventRecorder.Event.EveryUpdate.name) &
                                    (event_df[ReportEventRecorder.Column.Node_ID.name] == node_id)]
                        individual_per_gender_t_node = all_individuals_t_node[
                            all_individuals_t_node[ReportEventRecorder.Column.Gender.name] == gender]
                        num_of_cases = min(len(individual_per_gender_t_node), max_distributions_per_node)
                        new_infection_t_node = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] == node_id]
                        # main test
                        if not compare_num_of_cases(new_infection_t_node, num_of_cases, start_day, sft_report_file,
                                                    intervention_name, node_id, target_demographic):
                            success = False

                        # Make sure no case is distributed to the other gender
                        if not new_infection_t_node[
                            new_infection_t_node[ReportEventRecorder.Column.Gender.name] != gender].empty:
                            success = False
                            sft_report_file.write(f"\tBAD: the {Campaign.OutbreakIndividual} should not infect any "
                                                  f"{the_other_gender} individual "
                                                  f"at day {start_day} in node {node_id}.\n")

                        # Remove data from tested node id
                        new_infection_t = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] != node_id]
                else: # target age range
                    min_age, max_age = target_demographic
                    for node_id in target_nodes:
                        all_individuals_t_node = event_df[(event_df[ReportEventRecorder.Column.Time.name] == start_day) &
                                   (event_df[ReportEventRecorder.Column.Event_Name.name] ==
                                    ReportEventRecorder.Event.EveryUpdate.name) &
                                    (event_df[ReportEventRecorder.Column.Node_ID.name] == node_id)]
                        individual_per_age_target_t_node = all_individuals_t_node[
                            (all_individuals_t_node[ReportEventRecorder.Column.Age.name] >= min_age * sft.DAYS_IN_YEAR + 1) &
                            (all_individuals_t_node[ReportEventRecorder.Column.Age.name] <= max_age * sft.DAYS_IN_YEAR + 1)]
                        num_of_cases = min(len(individual_per_age_target_t_node), max_distributions_per_node)
                        new_infection_t_node = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] == node_id]
                        # main test
                        if not compare_num_of_cases(new_infection_t_node, num_of_cases, start_day, sft_report_file,
                                                    intervention_name, node_id, target_demographic):
                            success = False

                        # Make sure no case is distributed to outside the age bin
                        df_empty = new_infection_t_node[
                            (new_infection_t_node[ReportEventRecorder.Column.Age.name] < min_age * sft.DAYS_IN_YEAR + 1) |
                            (new_infection_t_node[ReportEventRecorder.Column.Age.name] > max_age * sft.DAYS_IN_YEAR + 1)]
                        if not df_empty.empty:
                            success = False
                            sft_report_file.write(f"\tBAD: the {Campaign.OutbreakIndividual} should not infect any "
                                                  f"age younger than {min_age} or older than {max_age} individual "
                                                  f"at day {start_day} in node {node_id}. They are "
                                                  f"{df_empty[ReportEventRecorder.Column.Individual_ID.name].tolist()}.\n")
                        # Remove data from tested node id
                        new_infection_t = new_infection_t[
                            new_infection_t[ReportEventRecorder.Column.Node_ID.name] != node_id]

            if not new_infection_t.empty:
                success = False
                sft_report_file.write(f"\tBAD: the {intervention_name} should not infect any individual "
                                      f"at day {start_day} in nodes(Not targeted in Nodeset Config). They are: "
                                      f"{set(new_infection_t[ReportEventRecorder.Column.Node_ID.name])}.\n")

        sft_report_file.write(sft.format_success_msg(success))

    return success


def application(output_folder="output", stdout_filename="test.txt",
                report_event_recorder="ReportEventRecorder.csv",
                config_filename="config.json", campaign_filename="campaign.json",
                report_name=sft.sft_output_filename,
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("stdout_filename: " + stdout_filename + "\n")
        print("report_event_recorder: " + report_event_recorder + "\n")
        print("config_filename: " + config_filename + "\n")
        print("campaign_filename: " + campaign_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("debug: " + str(debug) + "\n")

    sft.wait_for_done(stdout_filename)

    event_df = ReportEventRecorder(file=os.path.join(output_folder, "ReportEventRecorder.csv")).df

    create_report_file(event_df, report_event_recorder, config_filename, campaign_filename, report_name)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-R', '--report_event_recorder', default="ReportEventRecorder.csv",
                        help="ReportEventRecorder.csv to load (ReportEventRecorder.csv)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-C', '--campaign', default="campaign.json", help="campaign name to load (campaign.json)")
    parser.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout,
                report_event_recorder=args.report_event_recorder,
                config_filename=args.config, campaign_filename=args.campaign,
                report_name=args.reportname, debug=args.debug)

