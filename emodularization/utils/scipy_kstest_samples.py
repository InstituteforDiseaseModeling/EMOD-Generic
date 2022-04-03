#!/usr/bin/python

from scipy import stats
import math
import unittest
from datetime import datetime

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
#import dtk_mathfunc

class KSTestUnitTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(KSTestUnitTest, self).__init__(*args, **kwargs)
        rawdate = datetime.now()
        tempdate = str(rawdate)
        badchars = [' ',':','.']
        for char in badchars:
            tempdate = tempdate.replace(char, '-')

    def setUp(self):
        self.mean = 10.0
        self.sig = 2.0
        self.num_draws = 100
        self.norm_10_2 = []
        self.expon_10_2 = []
        n = stats.norm(loc=self.mean, scale=self.sig)
        x = stats.expon(loc=self.mean, scale=self.sig)
        for i in range(0, self.num_draws):
            self.norm_10_2.append(n.rvs())
            self.expon_10_2.append(x.rvs())

        self.sig_1_range = 0.6827 # 68-95-99.7 rule
        self.sig_2_range = 0.9545 # https://en.wikipedia.org/wiki/68%E2%80%9395%E2%80%9399.7_rule
        self.sig_3_range = 0.9973 # http://statweb.stanford.edu/~naras/jsm/NormalDensity/NormalDensity.html
        self.ks_result = None
        self.debug = False

    def tearDown(self):
        pass

    @unittest.skip("Don't want, don't need.\n")
    def test_normal_distribution_passes_fit(self):
        loc, scale = stats.norm.fit(self.norm_10_2)
        print "Norm Loc: {0} Expected: {1}\n".format(loc, self.mean)
        print "Norm Scale: {0} Expected: {1}\n\n".format(scale, self.sig)

        log_norms = []
        for n in self.norm_10_2:
            log_norms.append(math.log(n))

        shape, loc, scale = stats.lognorm.fit(log_norms)
        print "Lognorm Shape: {0} Expected: {1}\n".format(shape, "Something")
        print "Lognorm loc: {0} Expected:{1}\n".format(loc, self.mean)
        print "Lognorm scale: {0} Expected: {1}\n\n".format(scale, self.sig)

    def test_dtk_normal_distribution_passes_fit(self):
        import dtk_mathfunc
        my_mean = 10.0
        my_sigma = 2.0
        sample_size = 100
        print "\nRunning fit test on dtk generated norm with mean: {0} sigma: {1} samples: {2}\n".format(my_mean,
                                                                                                        my_sigma,
                                                                                                        sample_size)

        my_norm = []
        for x in range(0, sample_size):
            my_norm.append(dtk_mathfunc.get_gaussian_draw(my_mean, my_sigma))
        loc, scale = stats.norm.fit(my_norm)
        print "Norm Loc: {0} Expected: {1}\n".format(loc, my_mean)
        print "Norm Scale: {0} Expected: {1}\n\n".format(scale, my_sigma)

        log_norms = []
        for n in my_norm:
            log_norms.append(math.log(n))

        shape, loc, scale = stats.lognorm.fit(log_norms)
        print "Lognorm Shape: {0} Expected: {1}\n".format(shape, "Something")
        print "Lognorm loc: {0} Expected:{1}\n".format(loc, my_mean)
        print "Lognorm scale: {0} Expected: {1}\n\n".format(scale, my_sigma)

    def test_normal_distribution_kstest(self):
        print "\nRunning KS test on norm distribution with mean {0} and sigma {1}\n".format(self.mean, self.sig)
        ks_result = stats.kstest(self.norm_10_2, 'norm', args=(self.mean, self.sig))
        p_val = ks_result[1]
        ks_stat = ks_result[0]
        if self.debug:
            print "Norm Statistic: {0}\n".format(str(ks_stat))
            print "Norm P-Value: {0}\n".format(str(p_val))
        passes = True
        pval_ok = True
        ks_stat_ok = True
        if not p_val > 0.05:
            pval_ok = False
            passes = False
        if not ks_stat < 0.15:
            ks_stat_ok = False
            passes = False
        if not passes:
            import matplotlib.pyplot as plt
            fig, ax = plt.subplots(1,1)
            ax.hist(self.norm_10_2, normed=False, histtype='stepfilled', alpha=0.2, label='normal 10 2 data')
            ax.legend(loc='best', frameon=False)
            plt.show()
        self.assertTrue(pval_ok, "PVal should be greater than 0.05, was {0}\n".format(p_val))
        self.assertTrue(ks_stat_ok, "KS_statistic should be less than 0.15, was {0}\n".format(ks_stat))
        print "Test complete\n"

    def test_dtk_normal_distribution_kstest(self):
        import dtk_mathfunc
        my_mean = 10.0
        my_sigma = 2.0
        sample_size = 100
        print "\nRunning KS test on dtk generated norm with mean: {0} sigma: {1} samples: {2}\n".format(my_mean, my_sigma, sample_size)

        my_norm = []
        for x in range(0, sample_size):
            my_norm.append(dtk_mathfunc.get_gaussian_draw(my_mean, my_sigma))

        ks_result = stats.kstest(my_norm, 'norm', args=(my_mean, my_sigma))
        p_val = ks_result[1]
        ks_stat = ks_result[0]
        if self.debug:
            print "Norm Statistic: {0}\n".format(str(ks_stat))
            print "Norm P-Value: {0}\n".format(str(p_val))
        passes = True
        pval_ok = True
        ks_stat_ok = True
        if not p_val > 0.05:
            pval_ok = False
            passes = False
        if not ks_stat < 0.15:
            ks_stat_ok = False
            passes = False
        if not passes:
            import matplotlib.pyplot as plt
            fig, ax = plt.subplots(1,1)
            ax.hist(self.norm_10_2, normed=False, histtype='stepfilled', alpha=0.2, label='normal 10 2 data')
            ax.legend(loc='best', frameon=False)
            plt.show()
        self.assertTrue(pval_ok, "PVal should be greater than 0.05, was {0}\n".format(p_val))
        self.assertTrue(ks_stat_ok, "KS_statistic should be less than 0.15, was {0}\n".format(ks_stat))
        print "Test complete\n"

    def test_dtk_lognorm_distribution_kstest(self):
        import dtk_mathfunc
        my_mean = 10.0
        my_sigma = 2.0
        sample_size = 100
        print "\nRunning KS test on dtk generated lognorm with mean: {0} sigma: {1} samples: {2}\n".format(my_mean,
                                                                                                        my_sigma,
                                                                                                        sample_size)

        my_norm = []
        for x in range(0, sample_size):
            my_norm.append(dtk_mathfunc.get_lognormal_draw(my_mean, my_sigma))

        ks_result = stats.kstest(my_norm, 'lognorm', args=(my_mean, my_sigma))
        p_val = ks_result[1]
        ks_stat = ks_result[0]
        if self.debug:
            print "Norm Statistic: {0}\n".format(str(ks_stat))
            print "Norm P-Value: {0}\n".format(str(p_val))
        passes = True
        pval_ok = True
        ks_stat_ok = True
        if not p_val > 0.05:
            pval_ok = False
            passes = False
        if not ks_stat < 0.15:
            ks_stat_ok = False
            passes = False
        if not passes:
            import matplotlib.pyplot as plt
            fig, ax = plt.subplots(1, 1)
            ax.hist(my_norm, normed=False, histtype='stepfilled', alpha=0.2, label='lognormal 10 2 data')
            ax.legend(loc='best', frameon=False)
            plt.show()
        self.assertTrue(pval_ok, "PVal should be greater than 0.05, was {0}\n".format(p_val))
        self.assertTrue(ks_stat_ok, "KS_statistic should be less than 0.15, was {0}\n".format(ks_stat))
        print "Test complete\n"

    def test_plot_histogram_of_normal(self, plot=False):
        print "\nPlotting a histogram of normal with mean {0} and sigma {1}\n".format(self.mean, self.sig)
        ks_result = stats.kstest(self.norm_10_2, 'norm', args=(self.mean, self.sig))
        if self.debug:
            print "Norm Statistic: \n" + str(ks_result[0])
            print "Norm P-Value: \n" + str(ks_result[1])
        sorts = sorted(self.norm_10_2)
        sig_1_count = 0
        sig_2_count = 0
        sig_3_count = 0
        sig_3_low = self.mean - (3 * self.sig)
        sig_3_high = self.mean + (3 * self.sig)
        sig_2_low = self.mean - (2 * self.sig)
        sig_2_high = self.mean + (2 * self.sig)
        sig_1_low = self.mean - self.sig
        sig_1_high = self.mean + self.sig
        for draw in sorts:
            if sig_3_low < draw < sig_3_high:
                sig_3_count += 1
                if sig_2_low < draw < sig_2_high:
                    sig_2_count += 1
                    if sig_1_low < draw < sig_1_high:
                        sig_1_count += 1
        fudge_factor = 1.0 / (self.num_draws/10)
        within_sig1 = float(sig_1_count) / len(self.norm_10_2)
        within_sig2 = float(sig_2_count) / len(self.norm_10_2)
        within_sig3 = float(sig_3_count) / len(self.norm_10_2)
        print ("Within sig1: " + str(within_sig1))
        print ("Within sig2: " + str(within_sig2))
        print ("Within sig3: " + str(within_sig3))
        sig1_err_msg = "Expected sig1 range within {0} of {1}, got {2}".format(fudge_factor, self.sig_1_range, within_sig1)
        sig2_err_msg = "Expected sig2 range within {0} of {1}, got {2}".format(fudge_factor, self.sig_2_range, within_sig2)
        sig3_err_msg = "Expected sig3 range within {0} of {1}, got {2}".format(fudge_factor, self.sig_3_range, within_sig3)
        self.assertTrue(abs(within_sig1 - self.sig_1_range) < fudge_factor, msg=sig1_err_msg)
        self.assertTrue(abs(within_sig2 - self.sig_2_range) < fudge_factor, msg=sig2_err_msg)
        self.assertTrue(abs(within_sig3 - self.sig_3_range) < fudge_factor, msg=sig3_err_msg)
        if plot:
            import matplotlib.pyplot as plt
            fig, ax = plt.subplots(1,1)
            ax.hist(self.norm_10_2, normed=False, histtype='stepfilled', alpha=0.2, label='normal 10 2 data')
            ax.legend(loc='best', frameon=False)
            plt.show()
        print "Test complete\n"

if __name__ == '__main__':
    unittest.main()
