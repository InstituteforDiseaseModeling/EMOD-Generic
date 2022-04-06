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

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#acquisition-risk

This is a test for AcquisitionHeterogeneityVariance with Enable_Acquisition_Heterogeneity set to 1 and 
Acquisition_Transmission_Correlation set to 0.3. This simulation has 4 nodes and each nodes has different 
AcquisitionHeterogeneityVariance values(zero and non-zero).

This test focus on the init_mod_risk values and run the following checks:
1. Variance == AcquisitionHeterogeneityVariance of each node
2. Mean == 1
3. log-normal ks test with mu and sigma calculated based on the variance and mean.
4. init_mod_acq should remain the same(1) for all individuals.

output:
init_mod_risk_node.png (init_mod_risk vs. scipy log-normal)
init_mod_risk_node_cdf.png (cdf of init_mod_risk vs. stats.lognorm.cdf)

"""


class Channels:
    Statistical_Population = "Statistical Population"


def load_node_acquisition_heterogeneity_variance(demog_filename='demographics_AcquisitionHeterogeneityVariance_overlay.json'):
    with open(demog_filename) as infile:
        demog = json.load(infile)
    return demog['Nodes'][0]['IndividualAttributes']['AcquisitionHeterogeneityVariance'], \
           demog['Nodes'][1]['IndividualAttributes']['AcquisitionHeterogeneityVariance'], \
           demog['Nodes'][2]['IndividualAttributes']['AcquisitionHeterogeneityVariance'], \
           demog['Nodes'][3]['IndividualAttributes']['AcquisitionHeterogeneityVariance']


class AcquisitionHeterogeneityVarianceTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Enable_Acquisition_Heterogeneity,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Acquisition_Transmission_Correlation,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(AcquisitionHeterogeneityVarianceTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        node_variance_from_demog = load_node_acquisition_heterogeneity_variance()
        self.msg.append(f'node_variance_from_demog = {node_variance_from_demog}.\n')
        acquisition_transmission_correlation = self.params[ConfigKeys.Acquisition_Transmission_Correlation]
        self.msg.append(f'Acquisition_Transmission_Correlation = {acquisition_transmission_correlation}\n')

        if self.params[ConfigKeys.Enable_Acquisition_Heterogeneity] != 1:
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Enable_Acquisition_Heterogeneity} to 1 in '
                            f'{self.config_filename}.\n')
        elif self.params[ConfigKeys.Base_Infectivity_Distribution] != 'CONSTANT_DISTRIBUTION':
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Base_Infectivity_Distribution} to CONSTANT_DISTRIBUTION in '
                            f'{self.config_filename}.\n')
        else:
            # get mod_risk
            self.parse_stdout(filter_string_list=["CreateSusceptibility: Creating susceptibility"],
                              load_df_param=[["ID", "init_mod_acq", "init_mod_risk"],
                                           ["individual ", "init_mod_acq ", "init_mod_risk "],
                                           [SearchType.VAL, SearchType.VAL, SearchType.VAL]],
                              first_only=False)

            mod_df = self.stdout.df

            # get node id
            self.parse_report_event_recorder()
            csv_event_reporter = self.csv.df
            csv_event_reporter = csv_event_reporter[["Individual_ID", "Node_ID"]]
            # get node id from report event recorder and add it as a column into mod_df
            mod_df = pd.merge(left=mod_df, right=csv_event_reporter, how='outer', left_on='ID', right_on='Individual_ID')
            mod_df = mod_df.drop_duplicates()

            node_1_variance, node_2_variance, node_3_variance, node_4_variance = node_variance_from_demog

            # test node 1, variance = 0
            self.msg.append("Testing node 1:\n")
            if node_1_variance != 0:
                self.success = False
                self.msg.append(
                    'BAD: please set AcquisitionHeterogeneityVariance to 0 for node 1 in demographics file.\n')
            else:
                mod_df_node_1 = mod_df[mod_df["Node_ID"] == 1].sort_values(by=["Time", "Individual_ID"])
                # check mod_acq
                if (mod_df_node_1["init_mod_acq"] != 1).any():
                    self.success = False
                    self.msg.append(
                        '\tBAD: init_mod_acq in node 1 not equal to 1.\n')
                else:
                    self.msg.append(
                        '\tGOOD: init_mod_acq in node 1 equal to 1.\n')
                # check mod_risk
                if (mod_df_node_1["init_mod_risk"] != 1).any():
                    self.success = False
                    self.msg.append(
                        '\tBAD: init_mod_risk in node 1 not equal to 1, but AcquisitionHeterogeneityVariance '
                        '= 0.\n')
                else:
                    self.msg.append(
                        '\tGOOD: init_mod_risk in node 1 equal to 1, since AcquisitionHeterogeneityVariance '
                        '= 0.\n')

            # test node 2,3,4 variance != 0
            if node_2_variance == 0 or node_3_variance == 0 or node_4_variance == 0:
                self.success = False
                self.msg.append(
                    'BAD: please set AcquisitionHeterogeneityVariance to non zero value for node 2, 3, 4 in '
                    'demographics file.\n')
            else:
                for node_id in [2, 3, 4]:
                    self.msg.append(f"Testing node {node_id}:\n")
                    mod_df_node = mod_df[mod_df["Node_ID"] == node_id].sort_values(by=["Time", "Individual_ID"])
                    node_variance = node_variance_from_demog[node_id - 1]
                    # check mod_acq
                    if (mod_df_node["init_mod_acq"] != 1).any():
                        self.success = False
                        self.msg.append(
                            f'\tBAD: init_mod_acq in node {node_id} not equal to 1.\n')
                    else:
                        self.msg.append(
                            f'\tGOOD: init_mod_acq in node {node_id} equal to 1.\n')
                    # check mod_risk
                    # check mean
                    mod_risk_mean = mod_df_node["init_mod_risk"].mean()
                    if math.fabs(mod_risk_mean - 1) > 5e-2:
                        self.success = False
                        self.msg.append(
                            f'\tBAD: mean of init_mod_risk in node {node_id} not equal to 1, got {mod_risk_mean}.\n')
                    else:
                        self.msg.append(
                            f'\tGOOD: mean of init_mod_risk in node {node_id} equal to {mod_risk_mean}.\n')

                    # check variance
                    mod_risk_var = mod_df_node["init_mod_risk"].var()
                    if math.fabs(mod_risk_var - node_variance) > 5e-2 * node_variance:
                        self.success = False
                        self.msg.append(
                            f'\tBAD: variance of init_mod_risk in node {node_id} not equal to {node_variance}, got '
                            f'{mod_risk_var}.\n')
                    else:
                        self.msg.append(
                            f'\tGOOD: variance of init_mod_risk in node {node_id} equal to {mod_risk_var}, expected '
                            f'{node_variance}.\n')

                    # ks test for lognormal
                    if node_variance < 0.2:  # skip big variance, need huge data size to verify big variance
                        lognormal_test = dtk_sft.test_lognorm(mod_df_node["init_mod_risk"], -1 * node_variance/2,
                                                              math.sqrt(math.log(node_variance + 1)),
                                                              report_file=None, category=None, round=True, plot=True,
                                                              plot_name=f"init_mod_risk_node_{node_id}")
                        if not lognormal_test:
                            self.success = False
                            self.msg.append(
                                f'\tBAD: init_mod_risk in node {node_id} failed lognormal ks test with mu = '
                                f'{-1 * node_variance/2} and'
                                f' sigma = {math.sqrt(math.log(node_variance + 1))}.\n')
                        else:
                            self.msg.append(
                                f'\tGOOD: init_mod_risk in node {node_id} passed lognormal ks test with mu = '
                                f'{-1 * node_variance / 2} and'
                                f' sigma = {math.sqrt(math.log(node_variance + 1))}.\n')

            pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = AcquisitionHeterogeneityVarianceTest()
    else:
        my_sft = AcquisitionHeterogeneityVarianceTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
