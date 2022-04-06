#!/usr/bin/python
import json
# from dtk_test.sft_class import SFT, arg_parser
# from dtk_test.general_support import ConfigKeys
# import idm_test.plot as my_plot
from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
import dtk_test.dtk_sft as dtk_sft
"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#strain-tracking

This is a test for Enable_Genome_Dependent_Infectivity and Genome_Infectivity_Multipliers with Outbreak(Import Case).

This test focus on the Infectiousness values and run the following checks:
1. The infectivity of each infection will be multiplied by a value from that list based on that infection's Genome 
    index (using zero based indexing)
2. If an infection has a Genome value that is equal to or greater than the length of the 
    **Genome_Infectivity_Multipliers** list, then the last multiplier value in the list is used.
3. Different Clade values should not affect any of the infectitity multiplier
4. Tested with Constant Infectivity Distribution.

output:
Infectiousness.png 

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


class GenomeDependentInfectivityOutbreakTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Enable_Strain_Tracking,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Infectious_Period_Constant,
                            ConfigKeys.Enable_Genome_Dependent_Infectivity,
                            ConfigKeys.Genome_Infectivity_Multipliers,
                            ConfigKeys.Enable_Genome_Mutation,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(GenomeDependentInfectivityOutbreakTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        startday_and_genome = load_genome(self.campaign_filename)
        self.msg.append(f'OutbreakIndividual start day and Genome = {startday_and_genome}.\n')
        genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]
        self.msg.append(f'Genome_Infectivity_Multipliers = {genome_infectivity_multipliers}\n')

        if self.params[ConfigKeys.Enable_Strain_Tracking] != 1 or \
                (self.params[ConfigKeys.Enable_Genome_Dependent_Infectivity] != 1) or \
                (self.params[ConfigKeys.Enable_Genome_Mutation] != 0) or \
                (self.params[ConfigKeys.Base_Infectivity_Distribution] != 'CONSTANT_DISTRIBUTION'):
            self.success = False
            self.msg.append(f'BAD: please make sure the following parameters have the correct values in '
                            f'{self.config_filename}:\n'
                            f'{ConfigKeys.Enable_Strain_Tracking} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Dependent_Infectivity} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Mutation} = 0\n'
                            f'{ConfigKeys.Base_Infectivity_Distribution} = CONSTANT_DISTRIBUTION\n')
        else:
            # get Infectiousness
            self.parse_report_event_recorder()
            csv_event_reporter = self.csv.df
            csv_event_reporter = csv_event_reporter[["Time", "Individual_ID", "Infectiousness"]]

            # test Infectiousness against Genome
            base_infectivity_constant = self.params[ConfigKeys.Base_Infectivity_Constant]
            infectious_period_constant = self.params[ConfigKeys.Infectious_Period_Constant]
            simulation_duration = self.params[ConfigKeys.Simulation_Duration]
            genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]

            start_days = sorted(startday_and_genome.keys())
            pre_start_day = start_days.pop(0)
            start_day = start_days.pop(0)

            infectiousness_for_plotting = [list(), list()]

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
                expected_infectiousness = multiplier * base_infectivity_constant
                infectiousness_all = csv_event_reporter[csv_event_reporter["Time"] == t]["Infectiousness"].unique()
                if len(infectiousness_all) > 2:
                    self.success = False
                    self.msg.append(f"BAD: Infectiousness should be either 0 or {expected_infectiousness} for all "
                                    f"individual at time {t}. Got {infectiousness_all}.\n")
                else:
                    infectiousness = sorted(infectiousness_all)[-1]
                    if infectiousness != expected_infectiousness:
                        self.success = False
                        self.msg.append(f"BAD: Infectiousness should be {expected_infectiousness} for all individual "
                                        f"at time {t}. Got {infectiousness}.\n")
                    infectiousness_for_plotting[0].append(infectiousness)
                    infectiousness_for_plotting[1].append(expected_infectiousness)

            # my_plot.plot_data(infectiousness_for_plotting[0], infectiousness_for_plotting[1],
            dtk_sft.plot_data(infectiousness_for_plotting[0], infectiousness_for_plotting[1],
                              label1='infectiousness', label2='expected infectiousness',
                              title='infectiousness', category='infectiousness',
                              xlabel='t')
        pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = GenomeDependentInfectivityOutbreakTest()
    else:
        my_sft = GenomeDependentInfectivityOutbreakTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
