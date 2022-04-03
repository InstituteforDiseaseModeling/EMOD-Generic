import sys
import json
import math
from scipy import stats
import unittest
import age_structure_support as age_support
import dtk_nodedemog
import dtk_test.dtk_sft as dtk_sft
import random
from shutil import rmtree
from shutil import copy2
import datetime
import os

class SimulatedPerson:
    def __init__(self, mcw, age, gender):
        self.weight = mcw
        self.age = age
        self.gender = gender

class ComplexDistributionTest(unittest.TestCase):
    # region class method
    @classmethod
    def setUpClass(self):
        directories = ["./failed", "./result"]
        self.deleteAndCreateFolder(directories)

    @classmethod
    def deleteAndCreateFolder(self, directories):
        for directory in directories:
            if os.path.exists(directory):
                rmtree(directory)
                print("Delete folder: {}".format(directory))
            os.makedirs(directory)
            print("Create folder: {}".format(directory))

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

    def tearDown(self):
        pass

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

    # region testcase
    def _test_complex_initialization_node(self):
        print("\nTEST COMPLEX INITIALIZATION NODE: begin")
        a_d_key = age_support.DemographicsParameters.AGEDISTRIBUTION_KEY
        d_v_key = age_support.DemographicsParameters.DISTRIBUTIONVALUES_KEY
        r_v_key = age_support.DemographicsParameters.RESULTVALUES_KEY
        nodeText = '{"' + a_d_key + '": {"'+ age_support.DemographicsParameters.NUMDISTRIBUTIONAXES_KEY + '": 0,"'\
                   + age_support.DemographicsParameters.RESULTSCALEFACTOR_KEY +'": "'\
                   + str(age_support.Constants.DAYS_IN_YEAR) +'","'\
                   + age_support.DemographicsParameters.RESULTUNITS_KEY + '": "years",' + \
                   '"' + d_v_key + '": [0.0, 0.2, 0.2, 0.4, 0.4, 0.6, 0.6, 0.8, 0.8, 1.0, 1.0, 1.0],' + \
                   '"' + r_v_key + '": [0.0, 5.0,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55]}}'
        nodeJson = json.loads(nodeText)

        testNode = age_support.create_complex_agedistro_node(
            portions=[0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.0],
            ages=    [0.0, 5.0,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55])

        if self.debug:
            print(json.dumps(nodeJson, indent=4))
            print(json.dumps(testNode, indent=4))
        for i in range(len(testNode[d_v_key])):
            diff_1 = math.fabs(nodeJson[a_d_key][d_v_key][i] - testNode[d_v_key][i])
            diff_2 = math.fabs(nodeJson[a_d_key][r_v_key][i] - testNode[r_v_key][i])
            self.assertTrue(diff_1 < 1e-5)
            self.assertTrue(diff_2 < 1e-5)
        print("TEST COMPLEX INITIALIZATION NODE: pass")
        pass

    def test_standard_distribution(self):
        print("\nTEST COMPLEX INITIALIZATION WITH STANDARD DISTRIBUTION AND AGES: begin")
        title = "complex_standard_test"
        initial_population = 10000
        directories = ["./failed/staging/standard", "./result/standard"]
        distro = age_support.create_standard_complex_distro_node()

        age_support.configure_complex_age_initialization(distro, initial_population)

        success, message = self.run_test(distro, initial_population, directories)

        if not success:
            now = datetime.datetime.now()
            now_string = now.strftime("%H-%M-%S")
            # if not os.path.exists('./failed'):
            #     os.makedirs('./failed')
            if not os.path.exists(directories[0]):
                os.makedirs(directories[0])
            copy2('demographics.json', '{0}/demographics.json'.format(directories[0]))
            copy2('{0}/nodes_bucket.json'.format(directories[1]), '{0}/nodes_bucket_{1}.json'.format(directories[0], now_string))
            with open ('{0}/error_complex_standard.txt'.format(directories[1]),'w') as outfile:
                outfile.write(str(message))
            copy2('{0}/error_complex_standard.txt'.format(directories[1]), '{0}/error_complex_standard.txt'.format(directories[0], now_string))
            print("TEST COMPLEX INITIALIZATION WITH STANDARD DISTRIBUTION AND AGES: Failed")
            print(message)
            print("RERUN FAILED TEST")
            directories2 = ["./failed/final/standard", "./result/standard"]
            success, message2 = self.run_failed_test(directories2, demographics_filename="{0}/demographics.json".format(directories[0]))

        if not success:
            print("Compare error messages from both runs")
            success, message= self.compareResult(message, message2)
            print("Compare result: {0}: {1}".format(success, message))

        self.assertTrue(success, msg=message)
        print("TEST COMPLEX INITIALIZATION WITH STANDARD DISTRIBUTION AND AGES: Pass")
        pass

    def test_random_distribution(self, num_buckets = None, max_age = 100, run = 0):
        print("\nTEST COMPLEX INITIALIZATION WITH RANDOM DISTRIBUTION AND AGES_RUN_{0}: begin".format(run))
        initial_population = 10000
        directories = ["./failed/staging/random_run_{0}".format(run), "./result/random_run_{0}".format(run)]
        # Make portions
        if not num_buckets:
            num_buckets = int(random.random()*8) + 2 # at least 2 buckets
        portions = age_support.generate_distribution(num_buckets)
        if self.debug:
            print("buckets: {0}\n".format(portions))

        # Generate ages
        ages_random = age_support.generate_ages(max_age, num_buckets)
        if self.debug:
            print("ages: {0}\n".format(ages_random))

        # create distribution
        self.assertEqual(sum(portions), 1.0, msg="Portions must add up to 1")
        for i in portions:
            self.assertTrue(i >= 0, msg="Portion must be larger than 0")
        # portions = sorted(portions)

        current_age = 0
        for i in ages_random:
            self.assertTrue(i >= current_age, msg="ages must in ascending order")
            current_age = i
        distro = age_support.create_complex_agedistro_node(portions, ages_random)
        if self.debug:
            print("distro: {0}\n".format(distro))
            print("initial population: {0}".format(initial_population))

        # set demographics and config
        age_support.configure_complex_age_initialization(distro, initial_population)

        # run test
        success, message = self.run_test(distro, initial_population, directories)

        if not success:
            now = datetime.datetime.now()
            now_string = now.strftime("%H-%M-%S")
            if not os.path.exists(directories[0]):
                os.makedirs(directories[0])
            copy2('demographics.json', '{0}/demographics.json'.format(directories[0]))
            # copy2('demographics.json', 'failed/demographics{0}.json'.format(now_string))
            copy2('{0}/nodes_bucket.json'.format(directories[1]),
                  '{0}/nodes_bucket_{1}.json'.format(directories[0], now_string))
            copy2('{0}/age_buckets.png'.format(directories[1]),
                  '{0}/age_buckets.png'.format(directories[0]))
            with open ('{0}/error_complex_random.txt'.format(directories[1]),'w') as outfile:
                outfile.write(str(message))
            copy2('{0}/error_complex_random.txt'.format(directories[1]),
                  '{0}/error_complex_random.txt'.format(directories[0]))
            print(message)
            print("TEST COMPLEX INITIALIZATION WITH RANDOM DISTRIBUTION AND AGES_RUN_{0}: failed".format(run))
            print("RERUN FAILED TEST")
            directories2 = ["./failed/final/random_run_{0}".format(run), "./result/random_run_{0}".format(run)]
            success, message2 = self.run_failed_test(directories2, demographics_filename="{0}/demographics.json".format(directories[0]))
            # print "run failed test result: {0}, {1}".format(success, message2)

        if not success:
            print("Compare error messages from both runs")
            success, message= self.compareResult(message, message2)
            print("Compare result: {0}: {1}".format(success, message))

        self.assertTrue(success, msg = message)
        print("TEST COMPLEX INITIALIZATION WITH RANDOM DISTRIBUTION AND AGES_RUN_{0}: pass".format(run))
        pass
    # endregion

    def compareResult(self, msg1, msg2):
        message = []
        for msg in msg1:
            if msg in msg2:
                message.append(msg)
        if message:
            return False, message
        else:
            return True, "Pass"

    def run_test(self, distro, initial_population, directories, precision=7):
        # reset list of people before each test run
        self.peeps = []
        # place holder for result and error message
        success = True
        message = []

        # set callback
        dtk_nodedemog.set_callback(self.accumulate_people_callback)

        # run sim
        dtk_nodedemog.populate_from_files()

        buckets = {}  # dictionary for age buckets and ages
        peep_ages = []  # array to store all ages
        bucket_ranges = distro[age_support.DemographicsParameters.RESULTVALUES_KEY]  # age buckets
        distribution_values = distro[
            age_support.DemographicsParameters.DISTRIBUTIONVALUES_KEY]  # distribution value used to calculated each portion

        for i in bucket_ranges:
            buckets[i] = []
        for person in self.peeps:
            age = round(person.age, precision)
            # age = person.age
            peep_ages.append(age)
        x = 0
        peep_ages = sorted(peep_ages)
        for age in peep_ages:  # iterate througe all ages ascendingly
            while (x < len(bucket_ranges)):
                if age <= round(bucket_ranges[x] * age_support.Constants.DAYS_IN_YEAR, precision):
                    buckets[bucket_ranges[x]].append(age)  # group ages into buckets
                    break
                else:
                    x += 1

        if not os.path.exists(directories[1]):
            os.makedirs(directories[1])
        with open("{0}/nodes_bucket.json".format(directories[1]), "w") as outfile:
            json.dump(buckets, outfile, indent=4)

        # test portion and uniform distribution
        sizes=[] # for plot pie
        labels=[] # for plot pie
        previous_portion = 0
        total_peeps_count = 0
        previous_year = bucket_ranges[0]
        for i in range(len(bucket_ranges)):
            r = bucket_ranges[i]  # max limit of each age bucket
            ages = buckets[r]  # array contains ages in this bucket
            num_of_people = len(ages)  # number of people in this bucket
            expected_portion = distribution_values[i] - previous_portion
            calculate_portion = num_of_people / float(initial_population)
            if self.debug:
                print("Bucket {0} has {1} people".format(r, num_of_people))
                print("Age bucket: {0} - {1}, calculate portion: {2}, " \
                      "expected portion: {3}".format(previous_year, r, calculate_portion, expected_portion))
            # for plot pie
            sizes.append(num_of_people)
            labels.append("{0} - {1}".format(previous_year, r))
            # test portion
            if expected_portion == 0:
                if calculate_portion != 0:
                    success = False
                    message.append("Age bucket: {0} - {1}, calculate_portion is {2},"
                                   " expected_portion is 0".format(previous_year, r, calculate_portion))
                    # self.assertEqual(calculate_portion, 0, msg="calculate_portion is {0} expected_portion is 0".format(calculate_portion))
            else:
                diff = math.fabs(calculate_portion - expected_portion)
                if diff > 2e-2:
                    success = False
                    message.append("Age bucket: {0} - {1}, calculate portion: {2}, " \
                                   "expected portion: {3}".format(previous_year, r, calculate_portion,
                                                                  expected_portion))
                    # self.assertTrue(diff < 2e-2, msg="calculate_portion is {0} expected_portion is {1}".format(calculate_portion, expected_portion))
            # test uniform distribution
            if num_of_people > 0:  # if there is people
                total_peeps_count += num_of_people
                min_age = ages[0]
                max_age = ages[-1]
                min_years = age_support.round_places(min_age / age_support.Constants.DAYS_IN_YEAR, precision)
                max_years = age_support.round_places(max_age / age_support.Constants.DAYS_IN_YEAR, precision)
                if self.debug:
                    print("Total ages for bucket {0} - {1}: {2}".format(previous_year, r, num_of_people))
                    print("Min age: {0} or {1} years".format(min_age, min_years))
                    print("Max age: {0} or {1} years".format(max_age, max_years))

                if i > 0:
                    # ignore the first age bucket since the kstest uniform test is too senstive when min = max,
                    # we will test it by comparing the max_diff and min_diff
                    bucket_result = dtk_sft.test_uniform(ages,
                                                         min_age,
                                                         max_age,
                                                         round=True,
                                                         significant_digits=precision - 2)

                    if self.debug or self.saveplot or not bucket_result:
                        scipy_distro = stats.uniform.rvs(min_age,
                                                         max_age - min_age,
                                                         num_of_people)
                        dtk_sft.plot_data_sorted(ages, scipy_distro,
                                          label1="Ages from bucket {0}".format(r),
                                          label2="Uniform distro from scipy",
                                          category="{0}/Age_data_bucket_{1} - {2}".format(directories[1], previous_year, r),
                                          show=self.showplot)
                        dtk_sft.plot_probability(ages, scipy_distro,
                                                 label1="ages from model",
                                                 label2="ages from scipy distro",
                                                 category="{0}/Probability_bucket_{1} - {2}".format(directories[1],previous_year,r),
                                                 show=self.showplot)
                        if self.debug:
                            with open("{0}/nodes_{1}_{2}_bucket.json".format(directories[1], i - 1, i), "w") as outfile:
                                json.dump(ages, outfile, indent=4)
                            with open("{0}/scipy_{1}_{2}_bucket.json".format(directories[1], i - 1, i), "w") as outfile:
                                json.dump(sorted(scipy_distro), outfile, indent=4)
                    if not bucket_result:
                        # print "ks test result for bucket {0} - {1} : {2}".format(previous_year, r, bucket_result)
                        success = False
                        message.append(
                            "ks test result for bucket {0} - {1} : {2}".format(previous_year, r, bucket_result))
                        if not os.path.exists(directories[0]):
                            os.makedirs(directories[0])
                        if self.debug:
                            copy2('{0}/Age_data_bucket_{1} - {2}.png'.format(directories[1], previous_year,r),
                                  '{0}/Age_data_bucket_{1} - {2}.png'.format(directories[0], previous_year, r))
                            copy2('{0}/Probability_bucket_{1} - {2}.png'.format(directories[1], previous_year, r),
                                  '{0}/Probability_bucket_{1} - {2}.png'.format(directories[0], previous_year, r))
                    # self.assertTrue(bucket_result)

                # test ages are close enough to max and min limit
                max_diff = r - max_years
                # make sure we have a reasonable tolerance based on number of people in the bucket and the width of the bucket
                expected_diff = 10 * (r - previous_year) / (num_of_people * 1.0)
                expected_diff = 1e-2 if expected_diff <= 1e-2 else expected_diff
                if max_diff > expected_diff or max_diff < 0:
                    success = False
                    message.append("max age is {0} in bucket {1} - {2}, "
                                   "expected difference is less than {3}, but larger than or equal to 0"
                                   .format(max_years, previous_year, r, expected_diff))
                # self.assertTrue( max_diff <= expected_diff and max_diff >= 0,
                #                   msg = "max age is {0} in bucket {1}, "
                #                   "expected difference is less than {2} "
                #                   "and larger than or equal to 0".format(max_years, r, expected_diff))
                min_diff = min_years - previous_year
                if min_diff > expected_diff or min_diff < 0:
                    success = False
                    message.append("min age is {0} in bucket {1} - {2}, "
                                   "expected difference is less than {3}, but larger than or equal to 0"
                                   .format(min_years, previous_year, r, expected_diff))
                    # self.assertTrue( min_diff <= expected_diff and min_diff >= 0,
                    #                 msg = "min age is {0} in bucket {1}, "
                    #                       "expected difference is less than {2} "
                    #                       "and larger than or equal to 0".format(min_years, r, expected_diff))

            previous_year = r
            previous_portion = distribution_values[i]

        # plot a pie chart for number of people in each age buckets
        dtk_sft.plot_pie(sizes, labels, category = '{0}/age_buckets'.format(directories[1]), show = False)

        # test total population
        if total_peeps_count != initial_population:
            success = False
            message.append("total_peeps_count = {0}, expected initial_population = {1}".format(total_peeps_count,
                                                                                               initial_population))
        # self.assertEqual(total_peeps_count, initial_population,
        #                 msg = "total_peeps_count = {0}, expected initial_population = {1}".format(total_peeps_count, initial_population))
        if self.debug:
            print("Total people count = {0}, initial population = {1}".format(total_peeps_count, initial_population))
        return success, message

    def run_failed_test(self, directories, demographics_filename='failed/demographics.json',  title="complex_test"):
        print("RUN FAILED TEST: begin")
        # read demographics from failed demographics file
        demographics = age_support.get_json_template(json_filename=demographics_filename)

        # get distribution and initial population from demographics
        distro = demographics[age_support.DemographicsParameters.NODES_KEY][0][
            age_support.DemographicsParameters.INDIVIDUALATTRIBUTES_KEY][
            age_support.DemographicsParameters.AGEDISTRIBUTION_KEY]
        initial_population = demographics[age_support.DemographicsParameters.NODES_KEY][0][
            age_support.DemographicsParameters.NODEATTRIBUTES_KEY][
            age_support.DemographicsParameters.INITIALPOPULATION_KEY]
        if self.debug:
            print("distro : {0}".format(distro))
            print("initial population: {0}".format(initial_population))

        # set demographics
        age_support.set_demographics_file(demographics)

        # set config
        config = age_support.get_json_template(json_filename="nd_template.json")
        config[age_support.ConfigParameters.AGEDISTRIBUTIONTYPE_KEY] = age_support.ConfigParameters.COMPLEXDISTRO
        config[age_support.ConfigParameters.ENABLEBUILTINDEMO_KEY] = 0
        age_support.set_config_file(config)

        # run test
        success, message = self.run_test(distro, initial_population, directories)

        if not success:
            now = datetime.datetime.now()
            now_string = now.strftime("%H-%M-%S")
            if not os.path.exists(directories[0]):
                os.makedirs(directories[0])
            if self.debug:
                copy2("{0}/nodes_bucket.json".format(directories[1]),
                      '{0}/nodes_bucket_{1}.json'.format(directories[0], now_string))
                copy2("{0}/age_buckets.png".format(directories[1]),
                      '{0}/age_buckets.png'.format(directories[0]))
                with open('{0}/error_rerun.txt'.format(directories[1]), 'w') as outfile:
                    outfile.write(str(message))
                copy2("{0}/error_rerun.txt".format(directories[1]),
                      '{0}/error_rerun.txt'.format(directories[0]))
            print(message)
            print("RUN FAILED TEST: Failed")
        else:
            print("RUN FAILED TEST: Pass")
        return success, message

    def run_test_n_times(self, run = 10):
        for i in range(run):
            self.test_random_distribution(run=i + 1)

if __name__ == "__main__":
    unittest.main()


