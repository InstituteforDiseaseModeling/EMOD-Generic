#!/usr/bin/python

import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts').resolve().absolute()) )

from scipy import stats
import math
# from dtk_test.sft_class import SFT, arg_parser
# from dtk_test.general_support import ConfigKeys
# import idm_test.plot as my_plot
from dtk_test.dtk_sft_class import SFT, arg_parser
from dtk_test.dtk_General_Support import ConfigKeys
import dtk_test.dtk_sft as dtk_sft
from dtk_test.dtk_StdOut import SearchType

"""
spec: TBD

This is a test for Enable_Nonuniform_Shedding with Base_Infectivity_Distribution and Infectious_Period_Distribution both
set to CONSTANT_DISTRIBUTION.

output:
Infectiousness.png 

"""


class NonuniformSheddingConstantDistributionTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # self.json_report_name = "InsetChart.json"
        self.params_keys = [ConfigKeys.Enable_Nonuniform_Shedding,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution,
                            ConfigKeys.Infectious_Period_Constant,
                            ConfigKeys.Infectious_Period_Distribution,
                            ConfigKeys.Shedding_Distribution_Alpha,
                            ConfigKeys.Shedding_Distribution_Beta,
                            ConfigKeys.Simulation_Duration]

    def load_config(self):
        super(NonuniformSheddingConstantDistributionTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        enable_nonuniform_shedding = self.params[ConfigKeys.Enable_Nonuniform_Shedding]
        base_infectivity_distribution = self.params[ConfigKeys.Base_Infectivity_Distribution]
        infectious_period_distribution = self.params[ConfigKeys.Infectious_Period_Distribution]
        base_infectiousness = self.params[ConfigKeys.Base_Infectivity_Constant]
        infectious_period = self.params[ConfigKeys.Infectious_Period_Constant]
        alpha = self.params[ConfigKeys.Shedding_Distribution_Alpha]
        beta = self.params[ConfigKeys.Shedding_Distribution_Beta]

        if enable_nonuniform_shedding != 1 or \
                (base_infectivity_distribution != "CONSTANT_DISTRIBUTION") or \
                (infectious_period_distribution != "CONSTANT_DISTRIBUTION"):
            self.success = False
            self.msg.append(f'BAD: please make sure the following parameters have the correct values in '
                            f'{self.config_filename}:\n'
                            f'{ConfigKeys.Enable_Nonuniform_Shedding} = 1\n'
                            f'{ConfigKeys.Base_Infectivity_Distribution} = CONSTANT_DISTRIBUTION\n'
                            f'{ConfigKeys.Infectious_Period_Distribution} = CONSTANT_DISTRIBUTION\n')
        else:
            # get Infectiousness
            self.parse_stdout(filter_string_list=["infectiousness=", "Update(): Time:"],
                              load_df_param=[["id", "infectiousness"],
                                             ["id = ", "infectiousness="],
                                             [SearchType.VAL, SearchType.VAL]],
                              first_only=False)

            infectiousness_df = self.stdout.df
            # all individual should have the same infectiousness at the same time step
            infectiousness_df = infectiousness_df.drop_duplicates(
                subset=['Time', 'infectiousness'],
                keep='last').reset_index(drop=True)

            # test Infectiousness against Beta distribution
            expected_infectiousness_list = []
            for index, row in infectiousness_df.iterrows():
                t = row["Time"]
                infectiousness = row["infectiousness"]
                x = t / infectious_period
                expected_infectiousness = stats.beta.pdf(x, alpha, beta) * base_infectiousness
                expected_infectiousness_list.append(expected_infectiousness)
                if not math.isclose(infectiousness, expected_infectiousness, rel_tol=0.01):
                    self.success = False
                    self.msg.append(f"BAD: at time {t}, expected_infectiousness({expected_infectiousness}) "
                                    f"!= infectiousness({infectiousness}).\n")
            # plotting
            dtk_sft.plot_data(infectiousness_df["infectiousness"].tolist(), expected_infectiousness_list,
                              label1="infectiousness", label2="expected_infectiousness", title='infectiousness',
                              xlabel='t', ylabel='infectiousness', category ='infectiousness', overlap=True)

        pass


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = NonuniformSheddingConstantDistributionTest()
    else:
        my_sft = NonuniformSheddingConstantDistributionTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
