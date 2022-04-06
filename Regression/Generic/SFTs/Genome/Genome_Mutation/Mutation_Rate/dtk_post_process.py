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

This is a test for Genome_Mutation_Rates and Genome_Infectivity_Multipliers.

This test focus on the Infectiousness values and run the following checks:
1. The Genome value of each infection will be incremented by one with a rate specified by that infection's Genome 
   index (using zero based indexing).
2. If an infection has a Genome value that is equal to or greater than the length of the **Genome_Mutation_Rates** list,
   then the last rate value in the list is used. 
3. No more than one increment may occur during each time step.
4. Tested with Constant Infectivity Distribution. Infectious period is the same as simulaiton duration.

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


class GenomeMutationTest(SFT):
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
                            ConfigKeys.Genome_Mutation_Rates,
                            ConfigKeys.Log2_Number_of_Genomes_per_Clade,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(GenomeMutationTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        startday_and_genome = load_genome(self.campaign_filename)
        self.msg.append(f'OutbreakIndividual start day and Genome = {startday_and_genome}.\n')
        genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]
        self.msg.append(f'Genome_Infectivity_Multipliers = {genome_infectivity_multipliers}\n')

        if self.params[ConfigKeys.Enable_Strain_Tracking] != 1 or \
                (self.params[ConfigKeys.Enable_Genome_Dependent_Infectivity] != 1) or \
                (self.params[ConfigKeys.Enable_Genome_Mutation] != 1) or \
                (self.params[ConfigKeys.Base_Infectivity_Distribution] != 'CONSTANT_DISTRIBUTION'):
            self.success = False
            self.msg.append(f'BAD: please make sure the following parameters have the correct values in '
                            f'{self.config_filename}:\n'
                            f'{ConfigKeys.Enable_Strain_Tracking} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Dependent_Infectivity} = 1\n'
                            f'{ConfigKeys.Enable_Genome_Mutation} = 1\n'
                            f'{ConfigKeys.Base_Infectivity_Distribution} = CONSTANT_DISTRIBUTION\n')
        else:
            # get Infectiousness
            self.parse_report_event_recorder()
            csv_event_reporter = self.csv.df
            csv_event_reporter = csv_event_reporter[["Time", "Individual_ID", "Infectiousness"]]

            # test Infectiousness against Genome
            base_infectivity_constant = self.params[ConfigKeys.Base_Infectivity_Constant]
            simulation_duration = self.params[ConfigKeys.Simulation_Duration]
            genome_infectivity_multipliers = self.params[ConfigKeys.Genome_Infectivity_Multipliers]
            genome_mutation_rates = self.params[ConfigKeys.Genome_Mutation_Rates]
            number_of_genome = 2 ** self.params[ConfigKeys.Log2_Number_of_Genomes_per_Clade]
            infectiousness_lookup = []
            for genome_id in range(number_of_genome):
                if genome_id < len(genome_infectivity_multipliers):
                    infectiousness_lookup.append(genome_infectivity_multipliers[genome_id] * base_infectivity_constant)
                else:
                    infectiousness_lookup.append(genome_infectivity_multipliers[-1] * base_infectivity_constant)

            if len(startday_and_genome) != 1:
                self.success = False
                self.msg.append(f"BAD: please make sure only have one OutbreakIndividual in "
                                f"{self.campaign_filename}.\n")
            start_day = next(iter(startday_and_genome.keys()))

            ids = csv_event_reporter["Individual_ID"].unique().tolist()

            genome_mutation_counters = {}

            def add_genome_to_counter(genome, genome_mutation_counters):
                if genome in genome_mutation_counters:
                    genome_mutation_counters[genome].append(0)
                else:
                    genome_mutation_counters[genome] = [0]

            for ind_id in ids:
                genome = startday_and_genome[start_day]
                add_genome_to_counter(genome, genome_mutation_counters)

                infectiousness_ind = csv_event_reporter[csv_event_reporter["Individual_ID"] == ind_id][
                    "Infectiousness"].tolist()
                for infectiousness in infectiousness_ind:
                    expected_infectiousness = infectiousness_lookup[genome]
                    if infectiousness == expected_infectiousness:
                        genome_mutation_counters[genome][-1] += 1
                    else:
                        # mutation happens
                        # find my genome based on infectiousness
                        if genome + 1 < len(infectiousness_lookup):
                            if infectiousness == infectiousness_lookup[genome + 1]:
                                genome += 1
                                if genome < len(genome_mutation_rates) - 1:
                                    add_genome_to_counter(genome, genome_mutation_counters)
                                    genome_mutation_counters[genome][-1] += 1
                                else:
                                    # Don't compare the mutation length of last genome.
                                    break
                            else:
                                self.success = False
                                self.msg.append(f"BAD: ind {ind_id} should mutate to genome {genome + 1} from genome "
                                                f"{genome}. His/her infectiousness is {infectiousness}, expected "
                                                f"{infectiousness_lookup[genome + 1]}.\n")
                                break

            for genome, mutation_lengths in genome_mutation_counters.items():
                rate = genome_mutation_rates[genome] if genome < len(genome_mutation_rates) else genome_mutation_rates[-1]
                # if not stats_test.test_exponential(mutation_lengths, rate, integers=True, roundup=True,
                if not dtk_sft.test_exponential(mutation_lengths, rate, integers=True, roundup=True,
                                                  round_nearest=False, plot=False,
                                                  plot_name=f"plot_mutation_length_exponential_{genome}", msg=self.msg):
                    self.success = False
                    self.msg.append(f"BAD: mutation length for genome {genome} failed ks test for exponential "
                                    f"distribution with rate = {rate}.\n")

            pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = GenomeMutationTest()
    else:
        my_sft = GenomeMutationTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
