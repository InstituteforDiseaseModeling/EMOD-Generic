#!/usr/bin/python
import json
# from dtk_test.sft_class import SFT, arg_parser
# from dtk_test.general_support import ConfigKeys
# import idm_test.stats_test as stats_test
from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
import dtk_test.dtk_sft as dtk_sft

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#strain-tracking

This is a test for Enable_Genome_Dependent_Infectivity and Genome_Infectivity_Multipliers with 
Base_Infectivity_Distribution set to GAUSSIAN_DISTRIBUTION.

Comparing to the other tests in the same directory, this test focus on Gaussian distribution ks test for the 
infectiousness values with mean = multipler from Genome_Infectivity_Multipliers * base mean, std_dev = multipler from 
Genome_Infectivity_Multipliers * std_dev.

output:
Infectiousness_gaussian_{timestep}.png 

"""


def load_genome(campaign_filename='demographics_AcquisitionHeterogeneityVariance_overlay.json'):
    with open(campaign_filename) as infile:
        campaign = json.load(infile)
    res = dict()
    for event in campaign['Events']:
        genome = event['Event_Coordinator_Config']['Intervention_Config']['Genome']
        start_day = event['Start_Day']
        res[start_day] = genome
    return res


class GenomeDependentInfectivityGaussianTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Enable_Strain_Tracking,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Infectious_Period_Constant,
                            ConfigKeys.Enable_Genome_Dependent_Infectivity,
                            ConfigKeys.Genome_Infectivity_Multipliers,
                            ConfigKeys.Enable_Genome_Mutation,
                            ConfigKeys.Base_Infectivity_Gaussian_Mean,
                            ConfigKeys.Base_Infectivity_Gaussian_Std_Dev,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(GenomeDependentInfectivityGaussianTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        startday_and_genome = load_genome(self.campaign_filename)
        self.msg.append(f'OutbreakIndividual start day and Genome = {startday_and_genome}.\n')
        genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]
        self.msg.append(f'Genome_Infectivity_Multipliers = {genome_infectivity_multipliers}\n')

        if self.params[ConfigKeys.Enable_Strain_Tracking] != 1 or \
                (self.params[ConfigKeys.Enable_Genome_Dependent_Infectivity] != 1) or \
                (self.params[ConfigKeys.Enable_Genome_Mutation] != 0) or \
                (self.params[ConfigKeys.Base_Infectivity_Distribution] != 'GAUSSIAN_DISTRIBUTION'):
            self.success = False
            self.msg.append(f'BAD: please make sure the following parameters have the correct values in '
                            f'{self.config_filename}:\n'
                            f'{ConfigKeys.Enable_Strain_Tracking} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Dependent_Infectivity} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Mutation} = 0\n'
                            f'{ConfigKeys.Base_Infectivity_Distribution} = GAUSSIAN_DISTRIBUTION\n')
        else:
            # get Infectiousness
            self.parse_report_event_recorder()
            csv_event_reporter = self.csv.df
            csv_event_reporter = csv_event_reporter[["Time", "Individual_ID", "Infectiousness"]]

            # test Infectiousness against Genome
            base_infectivity_g_mean = self.params[ConfigKeys.Base_Infectivity_Gaussian_Mean]
            base_infectivity_g_std_dev = self.params[ConfigKeys.Base_Infectivity_Gaussian_Std_Dev]

            infectious_period_constant = self.params[ConfigKeys.Infectious_Period_Constant]
            simulation_duration = self.params[ConfigKeys.Simulation_Duration]
            genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]

            start_days = sorted(startday_and_genome.keys())
            pre_start_day = start_days.pop(0)
            start_day = start_days.pop(0)

            failed_test_count = passed_test_count = 0

            for t in range(simulation_duration):
                self.msg.append(f"Testing Infectiousness at time {t}:\n")
                if start_day is not None and (t >= start_day):
                    pre_start_day = start_day
                    if len(start_days):
                        start_day = start_days.pop(0)
                    else:
                        start_day = None
                if t == pre_start_day or (t <= pre_start_day + infectious_period_constant):
                    genome = startday_and_genome[pre_start_day]
                else:
                    genome = None

                if genome is not None:
                    if genome < len(genome_infectivity_multipliers):
                        multiplier = genome_infectivity_multipliers[genome]
                    else:
                        multiplier = genome_infectivity_multipliers[-1]
                else:
                    multiplier = 0
                expected_infectiousness_mean = multiplier * base_infectivity_g_mean
                expected_infectiousness_std_dev = multiplier * base_infectivity_g_std_dev if multiplier else 0
                infectiousness_all = csv_event_reporter[csv_event_reporter["Time"] == t]["Infectiousness"].tolist()
                if multiplier != 0:
                    # if not stats_test.test_eGaussNonNeg(infectiousness_all, expected_infectiousness_mean,
                    if not dtk_sft.test_eGaussNonNeg(infectiousness_all, expected_infectiousness_mean,
                                                       expected_infectiousness_std_dev,
                                                       plot=True, plot_name=f"infectiousness_gaussian_{t}",
                                                       msg=self.msg):
                        failed_test_count += 1
                        self.msg.append(f"WARNING: time {t} failed with ks test for Gaussian distribution with mean = "
                                        f"{expected_infectiousness_mean} and std_dev = "
                                        f"{expected_infectiousness_std_dev}.\n")
                    else:
                        passed_test_count += 1
                else:
                    if not all(i == 0 for i in infectiousness_all):
                        self.success = False
                        self.msg.append(f"BAD: infectiouseness should be all 0 at time {t}, got {infectiousness_all}.\n")
            if failed_test_count/(failed_test_count + passed_test_count) > 0.2:
                self.success = False
                self.msg.append(f"BAD: kstest failed {failed_test_count} times with "
                                f"{failed_test_count + passed_test_count} total tests.\n")
            else:
                self.msg.append(f"WARNING: kstest failed {failed_test_count} times with "
                                f"{failed_test_count + passed_test_count} total tests. Consider the whole test as a "
                                f"pass.\n")
        pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = GenomeDependentInfectivityGaussianTest()
    else:
        my_sft = GenomeDependentInfectivityGaussianTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
