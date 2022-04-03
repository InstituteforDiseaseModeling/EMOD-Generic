import sys
sys.path.append( ".." )
sys.path.append( "../../../Python" ) # for HPC execution. Not exactly robust. Fix with cross-platform pre-execution path aggregation
import json
import math
from scipy import stats
import unittest
import age_structure_support as age_support
import dtk_nodedemog
import dtk_test.dtk_sft as dtk_sft
import random
from hypothesis import given, strategies, settings, example
from datetime import timedelta
from shutil import copy2
import numpy as np
import os


class SimulatedPerson:
    def __init__(self, mcw, age, gender):
        self.weight = mcw
        self.age = age
        self.gender = gender

class SimpleDistributionTest(unittest.TestCase):
    @classmethod
    def setUp(self):
        self.debug = False
        self.showplot = False
        self.saveplot = False
        self.total_peeps_count = 0
        self.total_male = 0
        self.total_female = 0
        self.peeps = []
        self.Hypothesis_count = 0
        pass

    @classmethod
    def tearDown(self):
        if self.Hypothesis_count != 0:
            print("{0} Hypothesis tests ran".format(self.Hypothesis_count))
        pass

    # region debugging
    @classmethod
    def print_stats(self):
        print("Total peeps: {0}".format(self.total_peeps_count))
        print("Total males: {0}".format(self.total_male))
        print("Total females: {0}".format(self.total_female))
    # endregion

    # region callbacks
    def console_callback(self, mcw, age, gender):
        message = "New individual created. MCW: {0}\tAge: {1}\tGender: {2}"
        print(message.format(mcw, age, gender))

    def count_people_callback(self, mcw, age, gender):
        self.total_peeps_count += 1
        if gender == 0:
            self.total_male += 1
        else:
            self.total_female += 1

    def accumulate_people_callback(self, mcw, age, gender):
        self.peeps.append(SimulatedPerson(mcw, age, gender))
    # endregion

    @given(max_num = strategies.integers(min_value = 1, max_value = 10))
    @settings(max_examples = 100)
    def _test_hypothesis_random(self, max_num):
        random_num = int(random.random() * max_num)
        print(max_num, random_num)

    def test_distribution_off_test(self, initial_population = 10000):
        print("TEST DISTRIBUTION OFF: begin")
        # configure config and demographics
        demographics = age_support.get_json_template()
        single_node = age_support.change_node_ind_attribute(initial_population,
                                                att_key=age_support.DemographicsParameters.INITIALPOPULATION_KEY)
        demographics[age_support.DemographicsParameters.NODES_KEY] = [single_node]
        age_support.set_demographics_file(demographics)

        config = age_support.get_json_template(json_filename="nd_template.json")
        config[age_support.ConfigParameters.AGEDISTRIBUTIONTYPE_KEY] = age_support.ConfigParameters.DISTROOFF
        config[age_support.ConfigParameters.ENABLEBUILTINDEMO_KEY] = 0
        age_support.set_config_file(config)

        # set callback
        dtk_nodedemog.set_callback(self.accumulate_people_callback)

        # run sim
        dtk_nodedemog.populate_from_files()

        peep_ages = [] # array to store all ages
        for person in self.peeps:
            age = person.age
            peep_ages.append(age)

        if not os.path.exists('./result'):
            os.makedirs('./result')
        with open("result/distribution_of_age.txt", "w") as outfile:
            for age in peep_ages:
                outfile.write("{}\n".format(age))
        self.assertEqual(sum(peep_ages), 20 * age_support.Constants.DAYS_IN_YEAR * initial_population, msg = 'sum of total age is {}'.format(sum(peep_ages)))
        self.assertEqual(np.std(peep_ages), 0, msg = 'std of total age is {}'.format(np.std(peep_ages)))
        print("TEST DISTRIBUTION OFF: pass")
        pass

    def run_simple_initialization_test(self, initial_population=10000, flag=age_support.Constants.FLAG_UNIFORM, AgeDistribution1=0, AgeDistribution2=100):
        self.peeps = []

        # configure config and demographics
        age_support.configure_simple_age_initialization(initial_population, flag, AgeDistribution1, AgeDistribution2)

        # set callback
        dtk_nodedemog.set_callback(self.accumulate_people_callback)

        # run sim
        dtk_nodedemog.populate_from_files()

        peep_ages = [] # array to store all ages
        for person in self.peeps:
            age = round(person.age, 3)
            # age = person.age
            peep_ages.append(age)
        peep_ages = sorted(peep_ages)
        if not os.path.exists('./result'):
            os.makedirs('./result')
        with open("result/simple_{0}_age.txt".format(flag), "w") as outfile:
            for age in peep_ages:
                outfile.write("{}\n".format(age))
        return peep_ages


    @given(min = strategies.floats(min_value=0, max_value=10),
           max = strategies.floats(min_value=10, max_value=365000))
    @example(min = 0, max = 365000)
    @settings(max_examples=10, deadline=None)
    def test_simple_distribution_uniform(self, min, max):
        print("TEST SIMPLE DISTRIBUTION UNIFORM: begin")
        print("Hypothesis: min = {0}, max = {1}".format(min, max))
        initial_population = 10000
        flag = age_support.Constants.FLAG_UNIFORM
        age_peeps = self.run_simple_initialization_test(initial_population, flag, min, max)
        success = dtk_sft.test_uniform(age_peeps,
                                      min,
                                      max,
                                      round = True,
                                      significant_digits = 5)
        if self.debug or self.saveplot:
            scipy_distro = stats.uniform.rvs(min,
                                             max - min,
                                             len(age_peeps))
            dtk_sft.plot_data_sorted(age_peeps, scipy_distro,
                              label1="Ages from DTK",
                              label2="Uniform distro from scipy",
                              category="result/Age_data_simple_uniform_{0}".format(self.Hypothesis_count),
                              show=self.showplot)
            dtk_sft.plot_probability(age_peeps, scipy_distro,
                                     label1="ages from model",
                                     label2="ages from scipy distro",
                                     category="result/Probability_simple_uniform_{0}".format(self.Hypothesis_count),
                                     show=self.showplot)
        if not success:
            print("TEST SIMPLE DISTRIBUTION UNIFORM: failed")
            print("rerun TEST SIMPLE DISTRIBUTION UNIFORM")
            age_peeps = self.run_simple_initialization_test(initial_population, flag, min, max)
            success = dtk_sft.test_uniform(age_peeps,
                                      min,
                                      max,
                                      round = True,
                                      significant_digits = 5)
            if not success:
                if not os.path.exists('./failed'):
                    os.makedirs('./failed')
                copy2('demographics.json', 'failed/demographics_simple_uniform.json')
                copy2('result/simple_{0}_age.txt'.format(flag),'failed/simple_{0}_age.txt'.format(flag) )
        message = "kstest result for simple uniform distribution is False, " \
                  "initial_population = {0}, min = {1}, max = {2}".format(initial_population, min, max)
        self.assertTrue(success, msg = message)
        print("TEST SIMPLE DISTRIBUTION UNIFORM: pass")
        self.Hypothesis_count += 1
        pass

    @given(mean=strategies.floats(min_value=3650, max_value=32850),
           width=strategies.floats(min_value=1e-2, max_value=3650))
    @example(mean=18250, width=1825)
    @settings(max_examples=10, deadline=None)
    def test_simple_distribution_gaussian(self, mean, width):
        print("TEST SIMPLE DISTRIBUTION GAUSSIAN: begin")
        print("Hypothesis: mean = {0}, width = {1}".format(mean, width))
        initial_population = 10000
        flag = age_support.Constants.FLAG_GAUSSIAN
        age_peeps = self.run_simple_initialization_test(initial_population, flag, mean, width)
        # replaced with test_e
        # success = dtk_sft.test_gaussian(age_peeps,
        #                                mean,
        #                                width,
        #                                round=True,
        #                                 allow_negative = False)
        success = dtk_sft.test_eGaussNonNeg(age_peeps,
                                            mean,
                                            width,
                                            round=True)
        if self.debug or self.saveplot:
            # replaced with truncated normal distribution
            # scipy_distro = stats.norm.rvs(mean,
            #                               width,
            #                               len(age_peeps))
            a = -mean / width
            b = (sys.float_info.max - mean) / width
            scipy_distro = stats.truncnorm.rvs(a,
                                               b,
                                               mean,
                                               width,
                                               len(age_peeps))
            dtk_sft.plot_data_sorted(age_peeps, scipy_distro,
                              label1="Ages from DTK",
                              label2="Gaussian distro from scipy",
                              category="result/Age_data_simple_gaussian_{0}".format(self.Hypothesis_count),
                              show=self.showplot)
            dtk_sft.plot_probability(age_peeps, scipy_distro,
                                     label1="ages from model",
                                     label2="ages from scipy distro",
                                     category="result/Probability_simple_gaussian_{0}".format(self.Hypothesis_count),
                                     show=self.showplot)
        if not success:
            print("TEST SIMPLE DISTRIBUTION GAUSSIAN: failed")
            print("rerun TEST SIMPLE DISTRIBUTION GAUSSIAN")
            age_peeps = self.run_simple_initialization_test(initial_population, flag, mean, width)
            success = dtk_sft.test_gaussian(age_peeps,
                                       mean,
                                       width,
                                       round=True,
                                        allow_negative = False)
            if not success:
                if not os.path.exists('./failed'):
                    os.makedirs('./failed')
                copy2('demographics.json', 'failed/demographics_simple_gaussian.json')
                copy2('result/simple_{0}_age.txt'.format(flag),'failed/simple_{0}_age.txt'.format(flag) )
        message = "kstest result for simple Gaussian distribution is False, " \
                  "initial_population = {0}, mean = {1}, width = {2}".format(initial_population, mean, width)
        self.assertTrue(success, msg=message)
        print("TEST SIMPLE DISTRIBUTION GAUSSIAN: pass")
        self.Hypothesis_count += 1
        pass

    @given(dl=strategies.floats(min_value=1e-9, max_value=1e-3))
    @example(dl = 1)
    @example(dl = 0.001)
    @settings(max_examples=10, deadline=None)
    def test_simple_distribution_exponential(self, dl):
        print("TEST SIMPLE DISTRIBUTION EXPONENTIAL: begin")
        print("Hypothesis: decay length = {0}".format(dl))
        initial_population = 10000
        flag = age_support.Constants.FLAG_EXPONENTIAL
        age_peeps = self.run_simple_initialization_test(initial_population, flag, dl)
        success = dtk_sft.test_exponential(age_peeps, dl, report_file=None, integers=False, roundup=False, round_nearest=False)
        if self.debug or self.saveplot:
            scipy_distro = stats.expon.rvs(0,
                                          1.0/dl,
                                          len(age_peeps))
            dtk_sft.plot_data_sorted(age_peeps, scipy_distro,
                              label1="Ages from DTK",
                              label2="Exponential distro from scipy",
                              category="result/Age_data_simple_exponential_{0}".format(self.Hypothesis_count),
                              title="decay length={}".format(dl),
                              ylabel="age in day", xlabel="data point",
                              show=self.showplot)
            dtk_sft.plot_probability(age_peeps, scipy_distro,
                                     label1="ages from model",
                                     label2="ages from scipy distro",
                                     category="result/Probability_simple_exponential_{0}".format(self.Hypothesis_count),
                                     title="pdf, decay length={}".format(dl),
                                     ylabel="age in day", xlabel="prob",
                                     show=self.showplot)
        if not success:
            print("TEST SIMPLE DISTRIBUTION EXPONENTIAL: failed")
            print("rerun TEST SIMPLE DISTRIBUTION EXPONENTIAL")
            age_peeps = self.run_simple_initialization_test(initial_population, flag, dl)
            success = dtk_sft.test_exponential(age_peeps, dl, report_file=None, integers=False, roundup=False, round_nearest=False)
            if not success:
                if not os.path.exists('./failed'):
                    os.makedirs('./failed')
                copy2('demographics.json', 'failed/demographics_simple_exponential.json')
                copy2('result/simple_{0}_age.txt'.format(flag),'failed/simple_{0}_age.txt'.format(flag) )

        message = "kstest result for simple Exponential distribution is False, " \
                  "initial_population = {0}, decay length = {1}".format(initial_population, dl)
        self.assertTrue(success, msg=message)
        print("TEST SIMPLE DISTRIBUTION EXPONENTIAL: pass")
        self.Hypothesis_count += 1
        pass

    # Disabled BIMODAL test; see https://github.com/InstituteforDiseaseModeling/DtkTrunk/issues/3938
    #@given(p1=strategies.floats(min_value=1e-2, max_value=1), p2=strategies.integers(min_value=365, max_value=100000))
    #@example(p1=0.5, p2=10000)
    #@example(p1=0, p2=10000)
    #@example(p1=0.2, p2=0.2)
    #@settings(max_examples=30, deadline=None)
    #def test_simple_distribution_bimodal(self, p1, p2):
    #    print ("TEST SIMPLE DISTRIBUTION BIMODAL: begin")
    #    p1 = dtk_sft.round_to_n_digit(p1, 2)
    #    print ("Hypothesis: faction of param 1 = {0}, param 2 = {1}".format(p1, p2))
    #    initial_population = 10000
    #    flag = age_support.Constants.FLAG_BIMODAL
    #    age_peeps = self.run_simple_initialization_test(initial_population, flag, p1, p2)
    #    success = dtk_sft.test_bimodal(age_peeps, p1, p2)
    #    if self.debug or self.saveplot:
    #        dtk_sft.plot_data_sorted(age_peeps,
    #                          label1="Ages from DTK",
    #                          label2="NA",
    #                          category="result/Age_data_simple_bimodal_{0}".format(self.Hypothesis_count),
    #                          title="faction of param 2 = {0}, param 2 = {1}".format(p1, p2),
    #                          ylabel="age in day", xlabel="data point",
    #                          show=self.showplot)
    #        dtk_sft.plot_probability(age_peeps,
    #                                 label1="ages from model",
    #                                 label2="NA",
    #                                 category="result/Probability_simple_bimodal_{0}".format(self.Hypothesis_count),
    #                                 title="pdf, faction of param 2 = {0}, param 2 = {1}".format(p1, p2),
    #                                 ylabel="age in day", xlabel="prob",
    #                                 show=self.showplot)
    #    if not success:
    #        print ("TEST SIMPLE DISTRIBUTION BIMODAL: failed")
    #        print ("rerun TEST SIMPLE DISTRIBUTION BIMODAL")
    #        age_peeps = self.run_simple_initialization_test(initial_population, flag, p1, p2)
    #        success = dtk_sft.test_bimodal(age_peeps, p1, p2)
    #        if not success:
    #            if not os.path.exists('./failed'):
    #                os.makedirs('./failed')
    #            copy2('demographics.json', 'failed/demographics_simple_bimodal.json')
    #            copy2('result/simple_{0}_age.txt'.format(flag),'failed/simple_{0}_age.txt'.format(flag) )
    #
    #    message = "kstest result for simple bimodal distribution is False, " \
    #              "initial_population = {0}, faction of param 2 = {1}, param 2 = {2}".format(initial_population, p1, p2)
    #    self.assertTrue(success, msg=message)
    #    print ("TEST SIMPLE DISTRIBUTION BIMODAL: pass")
    #    self.Hypothesis_count += 1
    #    pass

if __name__ == "__main__":
    unittest.main()


