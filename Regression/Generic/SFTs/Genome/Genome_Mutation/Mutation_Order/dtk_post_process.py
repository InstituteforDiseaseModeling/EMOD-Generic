#!/usr/bin/python
import json
# from dtk_test.sft_class import SFT, arg_parser
# from dtk_test.general_support import ConfigKeys
from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys

"""
spec: https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Generic-spec.rst#strain-tracking

This is a test for Enable_Genome_Mutation.

This test focus on testing the mutation order plus this statement:
The Genome value will not be incremented to a value greater than the maximum Genome value 
(as specified by **Log2_Number_of_Genomes_per_Clade**).
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


class GenomeMutationOrderTest(SFT):
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
        super(GenomeMutationOrderTest, self).load_config(params_keys=self.params_keys)

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
                # Only calculate the number of genome defined by Log2_Number_of_Genomes_per_Clade
                if genome_id < len(genome_infectivity_multipliers):
                    infectiousness_lookup.append(genome_infectivity_multipliers[genome_id] * base_infectivity_constant)
                else:
                    infectiousness_lookup.append(genome_infectivity_multipliers[-1] * base_infectivity_constant)

            if len(startday_and_genome) != 1:
                self.success = False
                self.msg.append(f"BAD: please make sure only have one OutbreakIndividual in "
                                f"{self.campaign_filename}.\n")

            if (len(genome_mutation_rates) <= number_of_genome) or \
                (len(genome_infectivity_multipliers) <= number_of_genome):
                self.success = False
                self.msg.append(f"BAD: please make sure lengths of {ConfigKeys.Genome_Infectivity_Multipliers} and "
                                f"{ConfigKeys.Genome_Mutation_Rates} are larger than number of genome per clade in  "
                                f"{self.config_filename}.\n")
            start_day = next(iter(startday_and_genome.keys()))

            ids = csv_event_reporter["Individual_ID"].unique().tolist()

            for ind_id in ids:
                genome = startday_and_genome[start_day]
                infectiousness_ind = csv_event_reporter[csv_event_reporter["Individual_ID"] == ind_id][
                    "Infectiousness"].tolist()
                for infectiousness in infectiousness_ind:
                    expected_infectiousness = infectiousness_lookup[genome]
                    if infectiousness != expected_infectiousness:
                        # mutation happens
                        # find my genome based on infectiousness
                        if genome + 1 < len(infectiousness_lookup):
                            if infectiousness == infectiousness_lookup[genome + 1]:
                                genome += 1
                                # mutate to next genome, this is correct
                            else:
                                self.success = False
                                self.msg.append(f"BAD: ind {ind_id} should mutate to genome {genome + 1} from genome "
                                                f"{genome}. His/her infectiousness is {infectiousness}, expected "
                                                f"{infectiousness_lookup[genome + 1]}.\n")
                                break
                        else:
                            self.success = False
                            self.msg.append(f"BAD: genome {genome} is the last genome in the clade (with "
                                            f"{ConfigKeys.Log2_Number_of_Genomes_per_Clade} = "
                                            f"{self.params[ConfigKeys.Log2_Number_of_Genomes_per_Clade]}),ind {ind_id} "
                                            f"should not mutate any more.\n")

            pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = GenomeMutationOrderTest()
    else:
        my_sft = GenomeMutationOrderTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
