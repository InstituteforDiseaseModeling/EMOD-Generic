#!/usr/bin/python
import os
import pandas as pd
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/').resolve().absolute()) )

import json
import math

from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
from dtk_test.dtk_StdOut import SearchType
import dtk_test.dtk_sft as dtk_sft
from dtk_test.dtk_StdOut import StdOut

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#acquisition-risk

This is a test for zero Acquisition_Transmission_Correlation with Enable_Acquisition_Heterogeneity set to 1 and 
AcquisitionHeterogeneityVariance set to 0.3 and 1.6 in 2 nodes. Base_Infectivity_Distribution is set to 
Constant_Distribution.

In this test, since Acquisition_Transmission_Correlation = 0, infectiousness should not change at all. We are checking:
1. All infectiousness == Base_Infectivity_Constant
2. covariance of infectiousness and mod_risk should be 0
3. variance of mod_risk == AcquisitionHeterogeneityVariance of each node

output: (for both nodes)
mean_infectiousness.png 
variance_infectiousness.png
variance_mod_risk.png 

"""


class Channels:
    Statistical_Population = "Statistical Population"


def load_node_acquisition_heterogeneity_variance(demog_filename='demographics_AcquisitionHeterogeneityVariance_overlay.json'):
    with open(demog_filename) as infile:
        demog = json.load(infile)
    return demog['Nodes'][0]['IndividualAttributes']['AcquisitionHeterogeneityVariance'], \
           demog['Nodes'][1]['IndividualAttributes']['AcquisitionHeterogeneityVariance']


class ZeroAcquisitionTransmissionCorrelationTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Enable_Acquisition_Heterogeneity,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Acquisition_Transmission_Correlation,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(ZeroAcquisitionTransmissionCorrelationTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        node_variance_from_demog = load_node_acquisition_heterogeneity_variance()
        self.msg.append(f'node_variance_from_demog = {node_variance_from_demog}\n')
        acquisition_transmission_correlation = self.params[ConfigKeys.Acquisition_Transmission_Correlation]
        base_infectiousness = self.params[ConfigKeys.Base_Infectivity_Constant]

        if self.params[ConfigKeys.Enable_Acquisition_Heterogeneity] != 1:
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Enable_Acquisition_Heterogeneity} to 1 in '
                            f'{self.config_filename}.\n')
        elif self.params[ConfigKeys.Base_Infectivity_Distribution] != 'CONSTANT_DISTRIBUTION':
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Base_Infectivity_Distribution} to CONSTANT_DISTRIBUTION in '
                            f'{self.config_filename}.\n')
        elif acquisition_transmission_correlation != 0:
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Acquisition_Transmission_Correlation} to 0 in '
                            f'{self.config_filename}.\n')

        else:
            # get mod_risk
            mod_df = StdOut(self.stdout_filename,
                            filter_string_list=["CreateSusceptibility: Creating susceptibility"],
                            load_df_param=[["ID", "init_mod_acq", "init_mod_risk"],
                                           ["individual ", "init_mod_acq ", "init_mod_risk "],
                                           [SearchType.VAL, SearchType.VAL, SearchType.VAL]],
                            first_only=False).df

            self.parse_report_event_recorder()
            csv_event_reporter = self.csv.df
            csv_event_reporter = csv_event_reporter[["Individual_ID", "Node_ID"]]
            # get node id from report event recorder and add it as a column into mod_df
            mod_df = pd.merge(left=mod_df, right=csv_event_reporter, how='outer', left_on='ID', right_on='Individual_ID')
            mod_df = mod_df.drop_duplicates()
            mod_df = mod_df[["Individual_ID", "Node_ID", "init_mod_risk"]]

            # get infectiousness, contagion and prob
            self.parse_stdout(filter_string_list=["Update(): Time", "total contagion = "],
                              load_df_param=[["ID", "Contagion", "Prob", "Infectiousness"],
                                             ["id = ", "total contagion = ", "prob=", "infectiousness="],
                                             [SearchType.VAL, SearchType.VAL, SearchType.VAL, SearchType.VAL]],
                              first_only=False)

            stdout = self.stdout.df

            # get node id and init_mod_risk from rmod_df and add it as 2 columns into stdout df
            # stdout["Node_ID"] = stdout.apply(lambda row: csv_event_reporter[csv_event_reporter[
            #     "Individual_ID"] == row.ID]["Node_ID"].iloc[0], axis=1)
            df = pd.merge(left=stdout, right=mod_df, how='outer', left_on='ID', right_on='Individual_ID')
            df = df.drop_duplicates()

            df_node_1 = df[df["Node_ID"] == 1].sort_values(by=["Time", "Individual_ID"])
            df_node_2 = df[df["Node_ID"] == 2].sort_values(by=["Time", "Individual_ID"])

            node_1_variance, node_2_variance = node_variance_from_demog
            simulation_duration = self.params[ConfigKeys.Simulation_Duration]
            infectiousness_variance_list = [[list(), list()], [list(), list()]]
            infectiousness_mean_list = [[list(), list()], [list(), list()]]
            mod_risk_variance_list = [[list(), list()], [list(), list()]]
            for node_variance, df_node, node_id in zip([node_1_variance, node_2_variance],
                                                       [df_node_1, df_node_2],
                                                       [1, 2]):
                self.msg.append(f"Testing node {node_id}:\n")

                # check infectiousness
                if (df_node_1["Infectiousness"] != base_infectiousness).any():
                    self.success = False
                    self.msg.append(
                        f'BAD: Infectiousness should equal to {base_infectiousness} for all individual in node {node_id} '
                        f'since AcquisitionHeterogeneityVariance = 0.\n')
                else:
                    self.msg.append(
                        f'GOOD: Infectiousness equal to {base_infectiousness} for all individual in node {node_id} '
                        f'since AcquisitionHeterogeneityVariance = 0.\n')

                for t in range(1, simulation_duration):
                    self.msg.append(f"\tTesting t {t}:\n")
                    df_node_t = df_node[df_node["Time"] == t]
                    df_node_t_i = df_node[(df_node["Time"] == t) & (df_node["Infectiousness"] != 0)]["Infectiousness"]
                    infectiousness_mean = df_node_t_i.mean()
                    infectiousness_mean_list[node_id - 1][0].append(infectiousness_mean)
                    infectiousness_mean_list[node_id - 1][1].append(base_infectiousness)
                    # check mean infectiousness
                    if math.fabs(base_infectiousness - infectiousness_mean) / base_infectiousness > 5e-2:
                        self.success = False
                        self.msg.append(
                            f'BAD: at t {t}, expected mean infectiousness is {base_infectiousness}, '
                            f'get {infectiousness_mean} in node {node_id}. '
                            f'AcquisitionHeterogeneityVariance and Acquisition_Transmission_Correlation should not '
                            f'change the mean infectiousness value.\n')

                    import numpy as np
                    array_test = np.array(df_node_t[["Infectiousness", "init_mod_risk"]]).T
                    cov = np.cov(array_test)
                    if math.fabs(cov[0][0]) > 1e-2:
                        self.success = False
                        self.msg.append(f"\t\tBAD: variances of infectiousness should be about 0 at t {t} in node {node_id}"
                                        f", since {ConfigKeys.Acquisition_Transmission_Correlation} = "
                                        f"{acquisition_transmission_correlation}. Got {cov[0][0]}.\n")
                    else:
                        self.msg.append(f"\t\tGOOD: variances of infectiousness is about 0 at t {t} in node {node_id}"
                                        f", since {ConfigKeys.Acquisition_Transmission_Correlation} = "
                                        f"{acquisition_transmission_correlation}. Got {cov[0][0]}.\n")
                    if cov[0][1] != cov[1][0]:
                        self.success = False
                        self.msg.append(f"\t\tBAD: The off-diagonal elements of covariance function should be the same."
                                        f"Got {cov[0][1]} != {cov[1][0]}\n")
                    else:
                        self.msg.append(f"\t\tGOOD: The off-diagonal elements of covariance function are the same."
                                        f"Got {cov[0][1]} != {cov[1][0]}\n")
                    if math.fabs(cov[0][1]) > 1e-2:
                        self.success = False
                        self.msg.append(f"\t\tBAD: covariances of infectiousness and mod_risk should be about 0 at t "
                                        f"{t} in node {node_id} , since {ConfigKeys.Acquisition_Transmission_Correlation} = "
                                        f"{acquisition_transmission_correlation}. Got {cov[0][1]}.\n")
                    else:
                        self.msg.append(f"\t\tGOOD: covariances of infectiousness and mod_risk are about 0 at t "
                                        f"{t} in node {node_id} , since {ConfigKeys.Acquisition_Transmission_Correlation} = "
                                        f"{acquisition_transmission_correlation}. Got {cov[0][1]}.\n")
                    if math.fabs(cov[1][1] - node_variance) > 1e-1 * node_variance:
                        self.success = False
                        self.msg.append(f"\t\tBAD: variances of mod_risk should be about {node_variance} at t "
                                        f"{t} in node {node_id} , since AcquisitionHeterogeneityVariance = "
                                        f"{node_variance} in demographics file. Got {cov[1][1]}.\n")
                    else:
                        self.msg.append(f"\t\tGOOD: variances of mod_risk is about {node_variance} at t "
                                        f"{t} in node {node_id} , since AcquisitionHeterogeneityVariance = "
                                        f"{node_variance} in demographics file. Got {cov[1][1]}.\n")

                    infectiousness_variance_list[node_id - 1][0].append(round(cov[0][0], 2))
                    infectiousness_variance_list[node_id - 1][1].append(acquisition_transmission_correlation)

                    mod_risk_variance_list[node_id - 1][0].append(round(cov[1][1], 2))
                    mod_risk_variance_list[node_id - 1][1].append(node_variance)

            # plot
            for node_id in [1, 2]:
                dtk_sft.plot_data(dist1=infectiousness_mean_list[node_id - 1][0],
                                  dist2=infectiousness_mean_list[node_id - 1][1],
                                  label1="mean_infectiousness", label2="expected mean_infectiousness",
                                  title=f"mean_infectiousness_{node_id}", category=f"mean_infectiousness_{node_id}",
                                  sort=False)
                dtk_sft.plot_data(dist1=infectiousness_variance_list[node_id - 1][0],
                                  dist2=infectiousness_variance_list[node_id - 1][1],
                                  label1="variance_infectiousness", label2="expected variance_infectiousness",
                                  title=f"variance_infectiousness_{node_id}",
                                  category=f"variance_infectiousness_{node_id}", sort=False)
                dtk_sft.plot_data(dist1=mod_risk_variance_list[node_id - 1][0],
                                  dist2=mod_risk_variance_list[node_id - 1][1],
                                  label1="variance_mod_risk", label2="expected variance_mod_risk",
                                  title=f"variance_mod_risk_{node_id}",
                                  category=f"variance_mod_risk_{node_id}", sort=False)
            pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = ZeroAcquisitionTransmissionCorrelationTest()
    else:
        my_sft = ZeroAcquisitionTransmissionCorrelationTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
