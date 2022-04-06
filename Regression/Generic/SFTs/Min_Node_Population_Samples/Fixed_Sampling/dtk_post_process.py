#!/usr/bin/python
import os
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

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#population-sampling

This is a test for Min_Node_Population_Samples + FIXED_SAMPLING

"""


class Channels:
    Statistical_Population = "Statistical Population"


def load_node_population(demog_filename='demographics_3nodes.json'):
    with open(demog_filename) as infile:
        demog = json.load(infile)
    return demog['Nodes'][0]['NodeAttributes']['InitialPopulation'],\
           demog['Nodes'][1]['NodeAttributes']['InitialPopulation'], \
           demog['Nodes'][2]['NodeAttributes']['InitialPopulation']


class MinNodePopulationSamplesFixedTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Individual_Sampling_Type,
                            ConfigKeys.Min_Node_Population_Samples,
                            ConfigKeys.Base_Individual_Sample_Rate,
                            ConfigKeys.x_Base_Population]

    def load_config(self):
        super(MinNodePopulationSamplesFixedTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        node_population_from_demog = load_node_population()
        self.msg.append(f'node_population_from_demog = {node_population_from_demog}\n')

        if self.params[ConfigKeys.Individual_Sampling_Type] != 'FIXED_SAMPLING':
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Individual_Sampling_Type} to FIXED_SAMPLING in '
                            f'{self.config_filename}.\n')
        else:
            min_node_population_samples = self.params[ConfigKeys.Min_Node_Population_Samples]
            base_individual_sample_rate = self.params[ConfigKeys.Base_Individual_Sample_Rate]

            # parse test.txt to get sampled population for each node
            self.parse_stdout(filter_string_list=["'OutbreakIndividual' interventions at node "],
                              load_df_param=[["sampled pop", "node id"],
                                             ["gave out ", "at node "],
                                             [SearchType.VAL, SearchType.VAL]],
                              first_only=False)
            stdout = self.stdout.df.drop_duplicates()
            sampled_pop = stdout['sampled pop'].tolist()
            node_ids = stdout['node id'].tolist()

            stat_pop_expected = 0

            for node_id in node_ids:
                node_population_before_sampling = node_population_from_demog[int(node_id - 1)]
                sampled_pop_calculated = node_population_before_sampling * base_individual_sample_rate
                sampled_pop_reported = sampled_pop[int(node_id - 1)]
                exact_compare = False
                if node_population_before_sampling > min_node_population_samples:
                    if sampled_pop_calculated < min_node_population_samples:
                        sampled_pop_expected = min_node_population_samples
                        stat_pop_expected += node_population_before_sampling
                    else:
                        sampled_pop_expected = sampled_pop_calculated
                        stat_pop_expected += sampled_pop_expected * (1 / base_individual_sample_rate)
                else:
                    sampled_pop_expected = node_population_before_sampling
                    exact_compare = True
                    stat_pop_expected += node_population_before_sampling

                # check if we have the correct sampling population for each node
                tolerance = 0.05 * node_population_before_sampling * base_individual_sample_rate if not \
                    exact_compare else 0
                if math.fabs(sampled_pop_expected - sampled_pop_reported) > tolerance:
                    self.success = False
                    self.msg.append(
                        f'BAD: expected {sampled_pop_expected} in node {node_id} after sampling, got '
                        f'{sampled_pop_reported} (tolerance = {tolerance}).\n')
                else:
                    self.msg.append(
                        f'GOOD: expected {sampled_pop_expected} in node {node_id} after sampling, got '
                        f'{sampled_pop_reported} (tolerance = {tolerance}).\n')

            # check if the total population reported correctly in InsetChart()

            self.parse_json_report(channel_names=[Channels.Statistical_Population])
            inset_chart = self.json_report
            stat_pop = inset_chart.df[Channels.Statistical_Population].iloc[0]  # vital dynamic is off

            tolerance = 0.05 * sum(node_population_from_demog)
            if math.fabs(stat_pop_expected - stat_pop) > tolerance:
                self.success = False
                self.msg.append(
                    f'BAD: expected {stat_pop_expected} total population in InsetChart.json, got '
                    f'{stat_pop}(tolerance = {tolerance}).\n')
            else:
                self.msg.append(
                    f'GOOD: expected {stat_pop_expected} total population in InsetChart.json, got '
                    f'{stat_pop}(tolerance = {tolerance}).\n')
                pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = MinNodePopulationSamplesFixedTest()
    else:
        my_sft = MinNodePopulationSamplesFixedTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
