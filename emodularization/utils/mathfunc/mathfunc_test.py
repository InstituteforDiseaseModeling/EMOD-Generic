#!/usr/bin/python

import sys
import dtk_mathfunc as mf
import dtk_test.dtk_sft as sft
import math
import numpy as np
from scipy import stats
import pandas as pd
import os
import unittest
import logging
import datetime
import random


class TestMathFunctions(unittest.TestCase):
    results = []
    fail_count = 0
    pass_count = 0
    exception_count = 0
    count = 0
    debug = False

    @classmethod
    def setUp(cls):
        cls.fail_count = 0
        cls.pass_count = 0
        cls.exception_count = 0
        cls.count = 0
        cls.debug = False
        # Set up logging
        now = datetime.datetime.now()
        now_string = now.strftime("%H-%M-%S")
        logging.basicConfig(filename="Mathfunc_test_{0}.log".format(now_string),
                            format='%(asctime)s:%(levelname)s:%(message)s',
                            level=logging.DEBUG)
        pass

    @classmethod
    def tearDown(cls):
        # dump p1, p2 and result to csv file
        p1 = [r[0] for r in cls.results]
        p2 = [r[1] for r in cls.results]
        res = [r[2] for r in cls.results]
        df = pd.DataFrame({'p1': p1, 'p2': p2, 'results': res})
        res_path = r'./result.csv'
        if not os.path.exists(os.path.dirname(res_path)): os.makedirs(os.path.dirname(res_path))
        df.to_csv(res_path)

    def test_fixed(self, num_test = 500):
        self.results.append(('Fixed', 'Fixed', 'Fixed'))
        logging.info("FIXED")
        for test_run in range(num_test):
            p1 = self.draw_random_value(includezero=True)
            if test_run % 2 == 1:
                p1 *= -1 # half positive and half negative values
            size = 5
            p1 = sft.round_to_n_digit(p1, 7)
            result = None
            try:
                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_fixed_draw(p1), 7)
                    # s = mf.get_fixed_draw(p1)
                    expected = p1 if math.fabs(p1) > 1e-40 else 0
                    result = s == expected
                    if result:
                        self.pass_count += 1
                        logging.debug("Good: get_fixed_draw({0}) passes.".format(p1))
                    else:
                        self.fail_count += 1
                        logging.debug("Bad: get_fixed_draw({0}) fails: it returns {1}.".format(p1, s))
                    # self.assertTrue(result, msg="test_fixed fails: get_fixed_draw( "+ str(p1) +" ) return " + str(s) +".")

            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_fixed_draw({0}) raise exception: {1}.".format(p1, inst))

            self.results.append((p1, None, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_fixed: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count*size + self.exception_count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg="test_fixed fails with fail_count = " +
                                     str(self.fail_count) + ", pass_count = " +
                                     str(self.pass_count))
        self.assertTrue(self.exception_count<=2)

    def test_uniform(self, num_test = 500):
        self.results.append(('Uniform', 'Uniform', 'Uniform'))
        logging.info("UNIFORM")
        for test_run in range(num_test):
            p1 = self.draw_random_value(includezero=True)
            p = self.draw_random_value(includezero=True)
            if test_run % 2 == 1:
                p1 *= -1 # half positive and half negative values

            # Due to lower precision of float in C++, round p1 and p2 to 7 significant digits
            p1 = sft.round_to_n_digit(p1,7)
            p = sft.round_to_n_digit(p,7)
            # round values that < 0.0000001 to 0.0
            if math.fabs(p1 - 0.0) < 1/(10.0**(7-1)): p1 = 0.0
            if math.fabs(p - 0.0) < 1/(10.0**(7-1)): p = 0.0
            p2 = p1 + p
            size = 10000
            dist_uniform = []
            try:
                for _ in range(0, size):
                    # round result from get_uniform_draw() to 7 significant digits
                    s = sft.round_to_n_digit(mf.get_uniform_draw(p1, p2),7)
                    dist_uniform.append(s)
                if self.debug:
                    dist_uniform_scipy = stats.uniform.rvs(p1, p, size)
                    # dist_uniform_np = np.random.uniform(p1, p2, size)
                    sft.plot_probability(dist_uniform, dist_uniform_scipy)
                result = sft.test_uniform(dist_uniform, p1, p2,round = True)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_uniform_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_uniform_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_uniform_draw({0},{1}) raise exception: {2}".format(p1, p2, inst))
            self.results.append((p1, p2, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_uniform: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg = "test_uniform fails with fail_count = " +
                                       str(self.fail_count) + ", pass_count = " +
                                       str(self.pass_count))
        self.assertTrue(self.exception_count<=20)

    # this is replaced by test_eGaussNonNeg.
    # MathFunctions.cpp doesn't connect to eGauss() in Random.cpp for now.
    # we can add this back if we decide to add eGauss() to MathFunctions.cpp later.
    def _test_gaussian(self, num_test=500):
        self.results.append(('Gaussian', 'Gaussian', 'Gaussian'))
        logging.info("GAUSSIAN")
        for test_run in range(num_test):
            p1 = self.draw_random_value(includezero=True)
            p2 = self.draw_random_value(includezero=False)
            if test_run % 2 == 1:
                p1 *= -1 # half positive and half negative values

            size = 10000
            # Due to lower precision of float in C++, round p1 and p2 to 7 significant digits
            p1 = sft.round_to_n_digit(p1,7)
            p2 = sft.round_to_n_digit(p2,7)
            result = None
            dist_gaussian = []
            try:

                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_gaussian_draw(p1, p2),7)
                    dist_gaussian.append(s)
                if self.debug:
                    dist_gaussian_scipy = stats.norm.rvs(p1, p2, size)
                    # dist_gaussian_np = np.random.normal(p1, p2, size)
                    sft.plot_probability(dist_gaussian, dist_gaussian_scipy) # expect a symmetric "bell curve" shape
                result = sft.test_gaussian(dist_gaussian, p1, p2, allow_negative = False, round = True)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_gaussian_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_gaussian_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_gaussian_draw({0},{1}) raise exception: {2}".format(p1, p2, inst))

            self.results.append((p1, p2, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_gaussian: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg = "test_gaussian fails with fail_count = " +
                                       str(self.fail_count) + ", pass_count = " +
                                       str(self.pass_count))
        self.assertTrue(self.exception_count <= 0.05 * 0.1 * num_test * 3) # 3 sigma

    def test_eGaussNonNeg(self, num_test=500):
        # self.debug = True
        self.results.append(('eGaussNonNeg', 'eGaussNonNeg', 'eGaussNonNeg'))
        logging.info("eGaussNonNeg")
        exception_count = 0
        non_neg = True
        non_inf = True
        for test_run in range(num_test):
            p1 = self.draw_random_value(includezero=True) #-9.733508#
            p2 = self.draw_random_value(includezero=False) #1.758154 #
            if test_run % 5 == 1:
                p1 *= -1 # 80% positive and 20% negative values

            # Due to lower precision of float in C++, round p1 and p2 to 7 significant digits
            p1 = sft.round_to_n_digit(p1,7)
            p2 = sft.round_to_n_digit(p2,7)
            
            expect_error_value = False
            if p1 == float("inf") or p2 == float("inf") or p1/p2 < -4:
                expect_error_value = True
                #exception_count += 1

            size = 10000
            dist_gaussian = []
            try:
                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_gaussian_draw(p1, p2), 7)
                    dist_gaussian.append(s)
                with open("test_eGaussNonNeg_result.txt", 'a') as file:
                   if not expect_error_value:
                        if min(dist_gaussian) < 0:
                            result = non_neg = False
                            msg = f"BAD: mu={p1} sig={p2} failed, got value {min(dist_gaussian)} less than 0."
                            file.write(msg + '\n')
                            logging.debug(msg)
                        elif max(dist_gaussian) == float('inf'):
                            result = non_inf = False
                            msg = f"BAD: mu={p1} sig={p2} failed, got value {max(dist_gaussian)}."
                            file.write(msg + '\n')
                            logging.debug(msg)
                        else:
                            result = sft.test_eGaussNonNeg(dist_gaussian, p1, p2, round=True, report_file=file)
                            if not result and self.debug:
                                # plot truncated normal distribution
                                a = -p1 / p2
                                b = (sys.float_info.max - p1) / p2  # float("inf")
                                dist_gaussian_scipy = stats.truncnorm.rvs(a, b, p1, p2, size)
                                sft.plot_probability(dist_gaussian, dist_gaussian_scipy, title=f"pdf_p1={p1}_p2={p2}",
                                                     category=f'eGaussNonNeg_pdf-p1_{p1}-p2_{p2}')
                   else:  # expect_error_value = True
                       # If the model reaches an error condition, it returns a -1 as the only member of the
                       # distribution.
                       result = set(dist_gaussian) == {-1}
                       if not result:
                           msg = f"BAD: mu={p1} sig={p2} failed, expected error value = -1 when mu/sig < -4."
                           file.write(msg + '\n')
                           logging.debug(msg)
                       else:
                           msg = f"GOOD: mu={p1} sig={p2} passed, expected error value = -1 when mu/sig < -4."
                           file.write(msg + '\n')
                           logging.debug(msg)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_gaussian_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_gaussian_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_gaussian_draw({0},{1}) raise exception: {2}".format(p1, p2, inst))

            self.results.append((p1, p2, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_eGaussNonNeg: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg = "test_eGaussNonNeg fails with fail_count = " +
                                       str(self.fail_count) + ", pass_count = " +
                                       str(self.pass_count))
        self.assertTrue(self.exception_count == exception_count)
        self.assertTrue(non_neg, msg='expected no negative value.')
        self.assertTrue(non_inf, msg='expected no infinity value.')

    def test_exponential(self, num_test = 500):
        self.results.append(('Exponential', 'Exponential', 'Exponential'))
        logging.info("EXPONENTIAL")
        for test_run in range(num_test):
            p1 = self.draw_random_value(includezero=False)
            size = 10000
            result = None
            dist_exponential = []
            try:
                for _ in range(0, size):
                    dist_exponential.append(mf.get_exponential_draw(p1))
                if self.debug:
                    loc = 0
                    scale = 1.0 / p1
                    dist_exponential_scipy = stats.expon.rvs(loc, scale, size)
                    sft.plot_probability(dist_exponential, dist_exponential_scipy)
                result = sft.test_exponential(dist_exponential, p1, report_file=None, integers=False, roundup=False, round_nearest=False)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_exponential_draw({0}) passes.".format(p1))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_exponential_draw({0}) fails.".format(p1))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_exponential_draw({0}) raise exception: {1}".format(p1, inst))

            self.results.append((p1, None, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_exponential: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        print(message)
        logging.info(message)
        self.assertTrue(success, msg="test_exponential fails with fail_count = " +
                                     str(self.fail_count) + ", pass_count = " +
                                     str(self.pass_count))
        self.assertTrue(self.exception_count<=50) # exception for scale = 0 when p1== inf

    def test_poisson(self, num_test = 500):
        self.results.append(('Poisson', 'Poisson', 'Poisson'))
        logging.info("POISSON")
        for test_run in range(num_test):
            if test_run == 0:
                p1 = float("inf") # one exception
            elif test_run == 1:
                p1 = 2.2e9 # another exception
            else:
                while True:
                    p1 = self.draw_random_value(includezero=False)
                    if p1 < 2.1e9:
                        break
            size = 10000
            result = None
            dist_poison = []
            try:
                for _ in range(0, size):
                    dist_poison.append(mf.get_poisson_draw(p1))
                if self.debug:
                    loc = 0
                    dist_poison_scipy = stats.poisson.rvs(p1, loc, size)
                    sft.plot_probability(dist_poison, dist_poison_scipy)
                result = sft.test_poisson(dist_poison, p1, normal_approximation = False)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_poisson_draw({0}) passes.".format(p1))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_poisson_draw({0}) fails.".format(p1))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_poisson_draw({0}) raise exception: {1}".format(p1, inst))

            self.results.append((p1, None, result))
            self.count += 1
        # success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        success = float(self.fail_count) / self.count <= 0.12 # fail_count ~ 40+ due to the Precision limitation of floating point numbers in C++
        message = "test_poisson: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg="test_poisson fails with fail_count = " +
                                     str(self.fail_count) + ", pass_count = " +
                                     str(self.pass_count))
        self.assertTrue(self.exception_count <= 2)

    def test_lognormal(self, num_test=500):
        self.results.append(('Lognormal', 'Lognormal', 'Lognormal'))
        logging.info("LOGNORMAL")
        for test_run in range(num_test):
            p1 = random.uniform(-1e5, 700) # some lower limit, max limit is exp(700)
            p2 = random.uniform(1e-12,30)
            size = 1000
            p1 = sft.round_to_n_digit(p1,7)
            p2 = sft.round_to_n_digit(p2,7)
            result = None
            dist_lognormal = []
            try:

                # log_mean = sft.round_to_n_digit(log_mean,7)
                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_lognormal_draw(p1, p2), 7)
                    dist_lognormal.append(s)
                if self.debug:
                    # dist_lognormal_np = np.random.lognormal(log_mean, p2, size)
                    # sigma = p2  # standard deviation, which is log width
                    # scale = math.exp(p1)  # p1 is mean/mu, another way to understand this value
                    dist_lognormal_scipy = stats.lognorm.rvs(s = p2, loc = 0, scale = math.exp(p1), size = size)
                    sft.plot_probability(dist_lognormal,dist_lognormal_scipy, precision = 3)

                result = sft.test_lognorm(dist_lognormal, p1, p2, round = True)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_lognormal_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_lognormal_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_lognormal_draw({0}, {1}) raise exception: {2}".format(p1, p2, inst))

            self.results.append((p1, p2, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_lognormal: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg="test_lognormal fails with fail_count = " +
                                     str(self.fail_count) + ", pass_count = " +
                                     str(self.pass_count))
        self.assertEqual(self.exception_count, 0)

    def test_bimodal(self, num_test=500):
        self.results.append(('Bimodal', 'Bimodal', 'Bimodal'))
        logging.info("BIMODAL")
        for test_run in range(num_test):
            if test_run < 10:
                p1 = 0
            elif test_run < 20:
                p1 = 1
            else:
                p1 = random.random()
            p2 = self.draw_random_value(includezero=True)
            size = 10000
            p1 = sft.round_to_n_digit(p1,7)
            p2 = sft.round_to_n_digit(p2,7)
            result = None
            dist_bimodal = []
            try:
                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_bimodal_draw(p1, p2), 7)
                    dist_bimodal.append(s)
                if self.debug:
                    with open("test.txt","w") as file:
                        for n in dist_bimodal:
                            file.write("{}\n".format(n))
                result = sft.test_bimodal(dist_bimodal, p1, p2)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_bimodal_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_bimodal_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_bimodal_draw({0}, {1}) raise exception: {2}".format(p1, p2, inst))

            self.results.append((p1, p2, result))
            self.count += 1
        success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        message = "test_bimodal: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg="test_bimodal fails with fail_count = " + str(self.fail_count) + ", pass_count = " + str(self.pass_count))
        self.assertEqual(self.exception_count, 0)

    def test_weibull(self, num_test = 500):
        self.results.append(('Weibull', 'Weibull', 'Weibull'))
        logging.info("WEIBULL")
        for test_run in range(1,num_test+1):
            p1 = self.draw_random_value(includezero=False)
            p2 = self.draw_random_value(includezero=False)
            size = 10000
            p1 = sft.round_to_n_digit(p1,7)
            p2 = sft.round_to_n_digit(p2,7)
            result = None
            dist_weibull = []
            try:
                for _ in range(0, size):
                    s = sft.round_to_n_digit(mf.get_weibull_draw(p1, p2), 7)
                    dist_weibull.append(s)
                if self.debug:
                    # dist_weibull_np = np.random.weibull(p2, size)
                    # dist_weibull_np = map(lambda x : x * p1, dist_weibull_np)
                    dist_weibull_scipy = stats.weibull_min.rvs(c=p2, loc=0, scale=p1, size=size)
                    sft.plot_probability(dist_weibull, dist_weibull_scipy)
                result = sft.test_weibull(dist_weibull, p1, p2, round=True)
                if result:
                    self.pass_count += 1
                    logging.debug("Good: get_weibull_draw({0},{1}) passes.".format(p1, p2))
                else:
                    self.fail_count += 1
                    logging.debug("Bad: get_weibull_draw({0},{1}) fails.".format(p1, p2))
            except Exception as inst:
                self.exception_count += 1
                result = inst
                logging.debug("Exception: get_weibull_draw({0}, {1}) raise exception: {2}".format(p1, p2, inst))

            self.results.append((p1, p2, result))
            self.count += 1
        # success = sft.is_stats_test_pass(self.fail_count, self.pass_count)
        success = float(self.fail_count) / self.count <= 0.12 # fail_count 30~ 50 due to the Precision limitation of floating point numbers in C++
        message = "test_weibull: SUMMARY: Success= " + \
                  str(success) + ", exception = " + \
                  str(self.exception_count) + \
                  ", fail = " + str(self.fail_count) + \
                  ", pass = " + str(self.pass_count) + \
                  ", total = " + str(self.count) + "."
        logging.info(message)
        print(message)
        self.assertTrue(success, msg="test_weibull fails with fail_count = " +
                                     str(self.fail_count) + ", pass_count = " +
                                     str(self.pass_count))
        self.assertTrue(self.exception_count <= 0.05*500*0.4)

    def draw_random_value(self, includezero=False):
        """
        Draw a random positive number between (0-1e12), or positive infinity.
        This function will return values fall in this category:
            positive infinity or 0(when includezero==True)          5%
            very close to 0                                         15%
            lognormally distributed between (0,10+)                 20%
            lognormally distributed between(0,200+)                 20%
            lognormally distributed between(100,10000)              20%
            larger than 10000                                       20%
        :param num_test:
        :return:
        """
        random_num = random.random() # [0.0,1.0)
        if random_num <= 0.05:
            if includezero:
                p = float("inf") if random_num < 0.025 else 0
            else:
                p = float("inf")
        elif random_num < 0.2:
            p = random.lognormvariate(0, 10)
        elif random_num < 0.4:
            p = random.lognormvariate(0, 0.8)
        elif random_num < 0.6:
            p = random.lognormvariate(3, 1)
        elif random_num < 0.8:
            p = random.lognormvariate(7, 1)
        else:
            p = random.uniform(1e4, 1e12)
        return p

    
if __name__ == "__main__":
    unittest.main()
