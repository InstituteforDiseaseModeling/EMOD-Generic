#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../../shared_embedded_py_scripts/').resolve().absolute()) )

import json
import numpy as np
from scipy import stats

import dtk_test.dtk_sft as dtk_sft
from dtk_test.dtk_sft_class import SFT
from dtk_test.dtk_General_Support import ConfigKeys
from dtk_test.dtk_StdOut import SearchType

"""
spec: https://github.com/kfrey-idm/Documentation/blob/master/specs/Generic-spec.rst#modifications---overdispersion
This is a test with single node demographics file. It performs the following tests:
1. the actual infectivity(contagion) used for determining the infection rate is equal to rand_gamma using k = 
    1.0/InfectivityOverdispersion and theta = infectivity*InfectivityOverdispersion.
2. the contagion distribution has mean and variance as follows:
    mean = infectivity*dt
    variance = mean*(1 + InfectivityOverdispersion*mean)
3. the new infection distribution has mean and variance as follows:
    mean = mean of (infection prob * susceptible population at each time step)
    variance = mean*(1 + InfectivityOverdispersion*mean)

"""


class Channels:
    # using stat_pop channel to get the # of infected pop in the seed group
    # everyone in the seed group should be infected by Outbreak at every time step
    # their infection is cleared in the same time step since infectiousness duration is less than 1
    seed_infected        = "Statistical Population:QualityOfCare:1_Seed"
    susceptible_infected = "Infected:QualityOfCare:2_Susceptible"
    new_infection        = "New Infections:QualityOfCare:2_Susceptible"
    stat_pop             = "Statistical Population:QualityOfCare:2_Susceptible"


def load_InfectivityOverdispersion(demo_filename='demographics_overdispersion_overlay.json'):
    with open(demo_filename) as infile:
        demo = json.load(infile)
    return demo['Nodes'][0]['NodeAttributes']['InfectivityOverdispersion']


class OverdispersionTest(SFT):
    def __init__(self):
        super().__init__()
        self.json_report_name = "PropertyReport.json"
        self.params_keys = [ConfigKeys.Config_Name,
                            ConfigKeys.Base_Infectivity_Constant,
                            ConfigKeys.Base_Infectivity_Distribution]

    def load_config(self):
        super(OverdispersionTest, self).load_config(params_keys=self.params_keys)

    # overwrite the test method
    def test(self):
        infectivity_over_dispersion = load_InfectivityOverdispersion()
        self.msg.append(f'InfectivityOverdispersion = {infectivity_over_dispersion}\n')

        if self.params[ConfigKeys.Base_Infectivity_Distribution] != 'CONSTANT_DISTRIBUTION':
            self.success = False
            self.msg.append(f'BAD: please set {ConfigKeys.Base_Infectivity_Distribution} to CONSTANT_DISTRIBUTION in '
                            f'{self.config_filename}.\n')
        else:
            base_infectivity = self.params[ConfigKeys.Base_Infectivity_Constant]

            # parse test.txt to get the contagion and infection probability for every time step
            self.parse_stdout(filter_string_list=["Update(): Time:", "total contagion ="],
                              load_df_param=[["group", "contagion", "prob"],
                                             ["group_id = ", "total contagion = ", 'prob='],
                                             [SearchType.VAL, SearchType.VAL, SearchType.VAL]],
                              first_only=True)
            stdout = self.stdout.df.drop_duplicates()
            # filter out the seed group
            stdout = stdout[stdout['group'] == 1]
            contagion = stdout['contagion'].tolist()
            prob = stdout['prob'].tolist()

            # parse PropertyReport.json to get the Infected data for Seed group,  New Infection data and population data
            # for Susceptible group
            self.parse_json_report(channel_names=[Channels.seed_infected,
                                                  Channels.new_infection,
                                                  Channels.stat_pop,
                                                  Channels.susceptible_infected])
            inset_chart = self.json_report
            # Susceptible group
            stat_pop = inset_chart.df[Channels.stat_pop].iloc[0] # vital dynamic is off
            new_infection = inset_chart.df[Channels.new_infection].tolist()
            susceptible_pop = (inset_chart.df[Channels.stat_pop] - inset_chart.df[Channels.susceptible_infected]).tolist()
            # Infected for Seed group
            seed_infected = inset_chart.df[Channels.seed_infected]
            value_counts = seed_infected.value_counts().reset_index().rename(
                columns={'index': Channels.seed_infected,
                         Channels.seed_infected: 'count'})
            if len(value_counts) > 2:
                self.success = False
                self.msg.append(f'BAD: {Channels.seed_infected} should contain at most 2 unique values(including 0).\n')
            else:
                # skip the first day when there is no infected individual in Seed group
                new_infection.remove(0)

                n_infected = value_counts[value_counts[Channels.seed_infected] != 0][Channels.seed_infected].iloc[0]

                self.msg.append('1. Test actual infectivity (contagion):\n')
                mean_contagion = dtk_sft.mean_f(contagion)
                expected_mean_contagion = base_infectivity * n_infected / stat_pop
                msg = f'mean_contagion is {mean_contagion} while expected {expected_mean_contagion}.\n'
                if np.isclose(mean_contagion, expected_mean_contagion, rtol=0.25):
                    self.msg.append('\tGOOD: ' + msg)
                else:
                    self.success = False
                    self.msg.append('\tBAD: ' + msg)

                gamma_k = 1.0 / infectivity_over_dispersion
                gamma_theta = expected_mean_contagion * infectivity_over_dispersion

                variance_contagion = dtk_sft.variance_f(contagion)
                expected_variance_contagion = gamma_k * (gamma_theta ** 2)
                msg = f'variance_contagion is {variance_contagion} while expected {expected_variance_contagion}.\n'
                if np.isclose(variance_contagion, expected_variance_contagion, rtol=0.25):
                    self.msg.append('\tGOOD: ' + msg)
                else:
                    self.success = False
                    self.msg.append('\tBAD: ' + msg)

                # ks test
                result = dtk_sft.test_gamma(contagion, gamma_k, gamma_theta, report_file=self.msg)
                if not result:
                    self.success = False
                    self.msg.append(f"\tBAD: Contagion data doesn't pass ks test for Gamma distribution with k = {gamma_k}"
                                    f" and theta = {gamma_theta}.\n")
                else:
                    self.msg.append(f"\tGOOD: Contagion data passes ks test for Gamma distribution with k = {gamma_k}"
                                    f" and theta = {gamma_theta}.\n")
                dtk_sft.plot_cdf(contagion, stats.gamma.rvs(gamma_k, 0, gamma_theta, len(contagion)),
                                 category=os.path.join('contagion_cdf'),
                                 title=f'Cumulative distribution function\nk={gamma_k}, theta={gamma_theta}',
                                 line=True, show=False)

                self.msg.append('2. Test new infection:\n')
                mean_new_infection = dtk_sft.mean_f(new_infection)
                expected_mean_new_infection = dtk_sft.mean_f([p * s for p, s in zip(prob, susceptible_pop)])
                msg = f'mean_new_infection is {mean_new_infection} while expected {expected_mean_new_infection}.\n'
                if np.isclose(mean_new_infection, expected_mean_new_infection, rtol=0.25):
                    self.msg.append('\tGOOD: ' + msg)
                else:
                    self.success = False
                    self.msg.append('\tBAD: ' + msg)

                variance_new_infection = dtk_sft.variance_f(new_infection)
                expected_variance_new_infection = mean_new_infection * \
                                                  (1 + mean_new_infection * infectivity_over_dispersion)
                msg = f'variance_new_infection is {variance_new_infection} while expected ' \
                      f'{expected_variance_new_infection}.\n'
                if np.isclose(variance_new_infection, expected_variance_new_infection, rtol=0.25):
                    self.msg.append('\tGOOD: ' + msg)
                else:
                    self.success = False
                    self.msg.append('\tBAD: ' + msg)

                dtk_sft.plot_histogram(new_infection, 'New Infection', 0, 60)
                pass


def application(output_folder="output"):
    my_sft = OverdispersionTest()
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    application()
