#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/').resolve().absolute()) )

import json
import numpy as np
import math

from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
import dtk_test.dtk_sft as dtk_sft
from dtk_test.dtk_OutputFile import CsvOutput

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Interventions-spec.rst#travelrestriction

This is a test for TravelRestriction intervention with different Multiplier_Outbound and Multiplier_Inbound values([0 - 1]). 

The test compares the migration count with expected values for all timestep in each node and plots the values.

"""


class Channels:
    Statistical_Population = "Statistical Population"


def load_multipliers(campaign_filename='campaign.json'):
    with open(campaign_filename) as infile:
        campaign = json.load(infile)
    res = dict()
    for event in campaign["Events"]:
        start_day = event["Start_Day"]
        duration = event["Event_Coordinator_Config"]["Intervention_Config"]["Duration"]
        multiplier_outbound = event["Event_Coordinator_Config"]["Intervention_Config"]["Multiplier_Outbound"]
        multiplier_inbound = event["Event_Coordinator_Config"]["Intervention_Config"]["Multiplier_Inbound"]
        node_list = event["Nodeset_Config"]["Node_List"]
        for node in node_list:
            res[node] = {"Start_Day": start_day,
                         "Duration": duration,
                         "Multiplier_Outbound": multiplier_outbound,
                         "Multiplier_Inbound": multiplier_inbound}
    return res


class TravelRestrictionsTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.params_keys = [ConfigKeys.Enable_Regional_Migration,
                            ConfigKeys.Migration_Model,
                            ConfigKeys.Enable_Vital_Dynamics,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(TravelRestrictionsTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        campaign_dict = load_multipliers(self.campaign_filename)
        self.msg.append(f'TravelRestriction interventions in campaign are configured as = {campaign_dict}.\n')

        if self.params[ConfigKeys.Enable_Regional_Migration] != 1:
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Enable_Regional_Migration} to 1 in '
                            f'{self.config_filename}.\n')
        elif self.params[ConfigKeys.Migration_Model] != 'FIXED_RATE_MIGRATION':
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Migration_Model} to FIXED_RATE_MIGRATION in '
                            f'{self.config_filename}.\n')
        elif self.params[ConfigKeys.Enable_Vital_Dynamics] != 0:
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Enable_Vital_Dynamics} to 0 in '
                            f'{self.config_filename}.\n')
        else:
            # check total population
            self.parse_json_report(Channels.Statistical_Population)
            stat_pop = self.json_report.df[Channels.Statistical_Population]
            if len(stat_pop.unique()) != 1:
                self.success = False
                self.msg.append(f"BAD: TravelRestriction interventions should not change total population, please check "
                                f"{Channels.Statistical_Population} channels in {self.json_report_name}.\n")

            # # get node population from ReportNodeDemographics.csv
            # node_report = CsvOutput(file=os.path.join(self.output_folder, "ReportNodeDemographics.csv"))
            # node_df = node_report.df

            # due to #4506, need to get node population from built-in ReportEventRecorder.csv
            event_report = CsvOutput(file=os.path.join(self.output_folder, "ReportEventRecorder.csv"))
            event_df = event_report.df
            node_df = event_df.groupby(['Time', 'Node_ID']).size().to_frame('NumIndividuals').reset_index()

            # get migration count from ReportHumanMigrationTracking.csv
            csv_report = CsvOutput(file=os.path.join(self.output_folder, "ReportHumanMigrationTracking.csv"))
            migration_df = csv_report.df
            migration_dict = dict()
            node_count = 10
            expected_start_day = 5
            expected_duration = 20
            for node in range(1, node_count + 1):
                self.msg.append(f"Testing node {node}: \n")
                simulation_duration = self.params[ConfigKeys.Simulation_Duration]
                migration_dict[node] = [list(), list()]
                migration_node_df = migration_df[(migration_df[" From_NodeID"] == node) &
                                                 (migration_df["Time"] != simulation_duration)]
                # check destination node id
                to_node = migration_node_df[" To_NodeID"]
                expected_to_node = node + 1 if node != 10 else 1
                if not np.array_equal(to_node.unique(), [expected_to_node]):
                    self.success = False
                    self.msg.append(f"\tBAD: To_NodeID for migration out of From_NodeID = {node} should be "
                                    f"{expected_to_node} not {to_node.unique()}, please check "
                                    f"ReportHumanMigrationTracking.csv.\n")
                else:
                    self.msg.append(f"\tGOOD: To_NodeID for migration out of From_NodeID = {node} is "
                                    f"{expected_to_node}.\n")

                if node in campaign_dict:
                    start_day = campaign_dict[node]["Start_Day"]
                    if start_day != expected_start_day:
                        self.msg.append(f"\tWARNING: please set Start_Day for node {node} to {expected_start_day}.\n")
                    duration = campaign_dict[node]["Duration"]
                    if duration != expected_duration:
                        self.msg.append(f"\tWARNING: please set Duration for node {node} to {expected_duration}.\n")
                    multiplier_outbound = campaign_dict[node]["Multiplier_Outbound"]
                else:
                    start_day = expected_start_day
                    duration = expected_duration
                    multiplier_outbound = 1

                if expected_to_node in campaign_dict:
                    multiplier_inbound_next_node = campaign_dict[expected_to_node]["Multiplier_Inbound"]
                else:
                    multiplier_inbound_next_node = 1

                migration_rate = 0.1
                failed_count = 0
                # check migration count with multiplier and migration rate, we are only need to check one of the
                # migration direction
                for t in range(simulation_duration):
                    migration_count = len(migration_node_df[migration_node_df["Time"] == t])
                    migration_dict[node][0].append(migration_count)
                    node_pop = node_df[(node_df["Time"] == t) &
                                       (node_df["Node_ID"] == node)]["NumIndividuals"].iloc[0]

                    if t < start_day or t > start_day + duration:
                        intervention_time = False
                        actual_multiplier = 1
                    elif t == start_day:
                        intervention_time = True
                        actual_multiplier = multiplier_outbound
                    elif t == start_day + duration:
                        intervention_time = True
                        actual_multiplier = multiplier_inbound_next_node
                    else:
                        intervention_time = True
                        actual_multiplier = multiplier_outbound * multiplier_inbound_next_node

                    expected_migration_count = node_pop * (1.0 - math.exp(-1 * migration_rate)) * actual_multiplier
                    migration_dict[node][1].append(expected_migration_count)

                    # during the intervention, small tolerance
                    if math.fabs(expected_migration_count - migration_count) > expected_migration_count * 0.1 and \
                            intervention_time:
                        failed_count += 1
                        self.msg.append(f"\t\tWARNING: time: {t} node: {node} has {migration_count} outbound migrations,"
                                        f" expected {expected_migration_count}. (node_pop in source node = {node_pop}, "
                                        f"multiplier_outbound = {multiplier_outbound}), multiplier_inbound_next_node = "
                                        f"{multiplier_inbound_next_node}.\n")
                    # all simulation duration, large tolerance
                    if math.fabs(expected_migration_count - migration_count) > expected_migration_count * 0.4:
                        self.success = False
                        self.msg.append(f"\t\tBAD: time: {t} node: {node} has {migration_count} outbound migrations, "
                                        f"expected {expected_migration_count}. The migration count doesn't match the "
                                        f"expected value based on the fixed rate migration model. (node_pop in source "
                                        f"node = {node_pop}, migration_rate = {migration_rate}, "
                                        f"actual_multiplier = {actual_multiplier}).\n")
                if failed_count > 0.4 * duration:
                    self.success = False
                    self.msg.append(f"\tBAD: node {node} failed migration outbound test for {failed_count} times "
                                    f"during the duration({duration} days) of TravelRestriction intervention.\n")
                else:
                    self.msg.append(f"\tGOOD: node {node} failed migration outbound test for {failed_count} times "
                                    f"during the duration({duration} days) of TravelRestriction intervention. We "
                                    f"consider this is a pass.\n")

                dtk_sft.plot_data(dist1=migration_dict[node][0],
                                  dist2=migration_dict[node][1],
                                  label1="outbound_migration_count", label2="expected outbound_migration_count",
                                  title=f"outbound_migration_count_{node}", category=f"outbound_migration_count_{node}",
                                  sort=False)
            pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = TravelRestrictionsTest()
    else:
        my_sft = TravelRestrictionsTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
