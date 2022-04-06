#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/').resolve().absolute()) )

import json
import numpy as np

import dtk_test.dtk_sft as dtk_sft
from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
from dtk_test.dtk_OutputFile import ReportEventRecorder

"""
spec: https://github.com/kfrey-idm/Documentation/blob/master/specs/Generic-spec.rst#modifications---overdispersion
This is a test with multi nodes with different InfectivityOverdispersion values(3 and 0) in demographics file. 
It performs the following tests:
1. The InfectivityOverdispersion doesn't affect mean of actual infectivity. Verify by comparing ratio of daily New 
    Infections between node 1 and node 2 with ratio of infected individual in the Seed group.
2. Node with larger InfectivityOverdispersion value should have higher variance of daily New Infections.
    
"""

def load_InfectivityOverdispersion(demo_filename='demographics_2nodes_overdispersion_overlay.json'):
    with open(demo_filename) as infile:
        demo = json.load(infile)
    return demo['Nodes'][0]['NodeAttributes']['InfectivityOverdispersion'], \
           demo['Nodes'][1]['NodeAttributes']['InfectivityOverdispersion']


def get_daily_new_infection(event_df, node_id, simulation_duration):
    new_infection_per_node = event_df[event_df[ReportEventRecorder.Column.Node_ID.name] == node_id][
        [ReportEventRecorder.Column.Time.name, ReportEventRecorder.Column.Event_Name.name]]

    new_infection_per_node.rename(columns={ReportEventRecorder.Column.Event_Name.name:
                                           ReportEventRecorder.Event.NewInfection.name}, inplace=True)

    for t in range(1, simulation_duration):
        if t not in new_infection_per_node[ReportEventRecorder.Column.Time.name].tolist():
            max_index = max(new_infection_per_node.index)
            # append row to the dataframe
            new_infection_per_node.loc[max_index + 1] = [t, 0]
    return new_infection_per_node[ReportEventRecorder.Event.NewInfection.name]


class OverdispersionTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.json_report_name = "PropertyReport.json"
        self.params_keys = [ConfigKeys.Config_Name,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(OverdispersionTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        infectivity_over_dispersion = load_InfectivityOverdispersion()
        self.msg.append(f'InfectivityOverdispersion = {infectivity_over_dispersion}\n')

        self.parse_report_event_recorder()

        event_df = self.csv.df

        infected_seed = event_df[(event_df["QualityOfCare"] == '1_Seed') &
                                 (event_df[ReportEventRecorder.Column.Time.name] == 1)].\
            groupby(ReportEventRecorder.Column.Node_ID.name).count().reset_index()

        infected_seed_node_1 = infected_seed[infected_seed[ReportEventRecorder.Column.Node_ID.name] == 1
                                             ][ReportEventRecorder.Column.Event_Name.name].iloc[0]

        infected_seed_node_2 = infected_seed[infected_seed[ReportEventRecorder.Column.Node_ID.name] == 2
                                             ][ReportEventRecorder.Column.Event_Name.name].iloc[0]

        event_df = event_df.groupby([ReportEventRecorder.Column.Time.name, ReportEventRecorder.Column.Node_ID.name]).\
            count().reset_index()

        new_infection_node_1 = get_daily_new_infection(event_df, 1, self.params[ConfigKeys.Simulation_Duration]) - infected_seed_node_1
        new_infection_node_2 = get_daily_new_infection(event_df, 2, self.params[ConfigKeys.Simulation_Duration]) - infected_seed_node_2

        mean_node_1 = dtk_sft.mean_f(new_infection_node_1)
        mean_node_2 = dtk_sft.mean_f(new_infection_node_2)

        mean_ratio = mean_node_1 / mean_node_2
        expected_mean_ratio = infected_seed_node_1 / infected_seed_node_2

        msg = f'mean_node_1 / mean_node_2 is {mean_ratio} while expected ratio = {expected_mean_ratio}.\n'
        if np.isclose(mean_ratio, expected_mean_ratio, rtol=0.25):
            self.msg.append('\tGOOD: ' + msg)
        else:
            self.success = False
            self.msg.append('\tBAD: ' + msg)

        variance_node_1 = dtk_sft.variance_f(new_infection_node_1)
        variance_node_2 = dtk_sft.variance_f(new_infection_node_2)

        variance_ratio = variance_node_1 / variance_node_2

        if infectivity_over_dispersion[0] > infectivity_over_dispersion[1]:
            msg = f"InfectivityOverdispersion in Node 1 is greater than Node 2, expected variance ratio is greater" \
                  f" than {expected_mean_ratio}, got variance_node_1 / variance_node_2 = {variance_ratio}.\n"
            if variance_ratio <= expected_mean_ratio:
                self.success = False
                self.msg.append('\tBAD: ' + msg)
            else:
                self.msg.append('\tGOOD: ' + msg)
        else:
            msg = f"InfectivityOverdispersion in Node 1 is less than Node 2, expected variance ratio is less than" \
                  f" {expected_mean_ratio}, got variance_node_1 / variance_node_2 = {variance_ratio}.\n"
            if variance_ratio > expected_mean_ratio:
                self.success = False
                self.msg.append('\tBAD: ' + msg)
            else:
                self.msg.append('\tGOOD: ' + msg)

        expected_variance_ratio = expected_mean_ratio * (1.0 + mean_node_1 * infectivity_over_dispersion[0]) / \
                                  (1.0 + mean_node_2 * infectivity_over_dispersion[1])

        msg = f'variance_node_1 / variance_node_2 is {variance_ratio} while expected ratio = {expected_variance_ratio}.\n'
        if np.isclose(variance_ratio, expected_variance_ratio, rtol=0.25):
            self.msg.append('\tGOOD: ' + msg)
        else:
            self.success = False
            self.msg.append('\tBAD: ' + msg)

        dtk_sft.plot_histogram(new_infection_node_1, 'New Infection_node_1', 0, 60)
        dtk_sft.plot_histogram(new_infection_node_2, 'New Infection_node_2', 0, 60)
        pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = OverdispersionTest()
    else:
        my_sft = OverdispersionTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
