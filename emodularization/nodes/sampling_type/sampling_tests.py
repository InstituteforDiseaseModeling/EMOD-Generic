
#!/usr/bin/env python3
# coding=utf-8

from __future__ import print_function
import json
import unittest
import dtk_nodedemog
import numpy as np
import math
from shutil import copyfile
from hypothesis import given, strategies, settings, example, reproduce_failure, assume


class DemographicsParameters:
    KEY_NODES = 'Nodes'
    NODEATTRIBUTES_KEY = 'NodeAttributes'
    INITIALPOPULATION_KEY = 'InitialPopulation'


class SamplingTypes:
    TRACKALL = 'TRACK_ALL'
    FIXED = 'FIXED_SAMPLING'
    ADAPTED_BY_AGE = 'ADAPTED_SAMPLING_BY_AGE_GROUP'
    ADAPTED_BY_POP = 'ADAPTED_SAMPLING_BY_POPULATION_SIZE'
    ADAPTED_BY_AGE_AND_POP = 'ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE'
    ADAPTED_BY_IMMUNE_STATE = 'ADAPTED_SAMPLING_BY_IMMUNE_STATE'


class ConfigurationParameters:
    SAMPLINGTYPE_KEY = 'Individual_Sampling_Type'
    POP_SCALE_TYPE_KEY = 'Population_Scale_Type'
    FIXED_SCALE_TYPE_VALUE = 'FIXED_SCALING'
    SCALEFACTOR_KEY = 'Base_Population_Scale_Factor'
    SAMPLERATE_KEY = 'Base_Individual_Sample_Rate'

    class AgeGroup_Sample_Rate:
        KEY_BIRTH = 'Sample_Rate_Birth'
        KEY_0_18M = 'Sample_Rate_0_18mo'
        KEY_18M_4 = 'Sample_Rate_18mo_4yr'
        KEY_5_9 = 'Sample_Rate_5_9'
        KEY_10_14 = 'Sample_Rate_10_14'
        KEY_15_19 = 'Sample_Rate_15_19'
        KEY_20_PLUS = 'Sample_Rate_20_Plus'
    

class Person:
    def __init__(self, mcw, age, gender):
        self.mcw = mcw
        self.age = age
        self.gender = gender

    def to_string(self):
        gender = 'm' if self.gender == 0 else 'f'
        age_years = int(self.age / 365)
        age_days = int(self.age % 365)
        return f'MCW: {self.mcw}, Age: {age_years}y {age_days}d ({self.age:0.2f}), Gender: {gender}'


class SamplingTests(unittest.TestCase):
    """
    Tests for different sampling types as well as scale factors
    """
    def __init__(self, *args, **kwargs):
        super(SamplingTests, self).__init__(*args, **kwargs)
        self.debug = False
        self.reset()

    #  region setup
    def setUp(self):
        self.debug = False
        self.reset()

    def tearDown(self):
        self.reset()

    def reset(self):
        self.population = 0
        self.male_pop = 0
        self.female_pop = 0
        self.people = []

    def debug_state(self):
        print(f'Population: {self.population}')
        print(f'Males: {self.male_pop}')
        print(f'Females: {self.female_pop}')

    #  endregion setup

    #  region callbacks
    def debug_callback(self, mcw, age, gender):
        print(f'New individual created. MCW: {mcw}\tAge: {age}\tGender: {gender}')

    def count_people_callback(self, mcw, age, gender):
        self.population += 1
        if gender == 0:
            self.male_pop += 1
        else:
            self.female_pop += 1

    def accumulate_people_callback(self, mcw, age, gender):
        self.people.append(Person(mcw, age, gender))

    #  endregion

    #  region config and demographics functions
    def load_config_template_json(self, complex_pop=False):
        if complex_pop:
            config_filename = 'complex_config_template.json'
        else:
            config_filename = 'config_template.json'
        with open(config_filename) as infile:
            return json.load(infile)

    def write_config_file(self, configuration, config_filename='nd.json'):
        with open(config_filename, 'w') as outfile:
            json.dump(configuration, outfile, indent=4, sort_keys=True)

    def load_demographics_template_json(self, complex_pop=False):
        if complex_pop:
            demo_filename = 'complex_demographics_template.json'
        else:
            demo_filename = 'demographics_template.json'
        with open(demo_filename) as infile:
            return json.load(infile)

    def create_demographics_nodes(self, demographics=None):
        if not demographics:
            demographics = self.load_demographics_template_json()
        return demographics[DemographicsParameters.KEY_NODES]

    def change_node_ind_attribute(self, node, attribute_key, new_value):
        node[DemographicsParameters.NODEATTRIBUTES_KEY][attribute_key] = new_value
        return node

    def create_single_demographics_node_init_pop(self, initial_population):
        node = self.create_demographics_nodes()[0]
        return self.change_node_ind_attribute(node, DemographicsParameters.INITIALPOPULATION_KEY, initial_population)

    def create_config_with_overrides(self, overrides, complex_pop=False):
        """
        Create a config from either the regular or complex templates and update some config values using the provided
        list of overrides.

        :param overrides: a dictionary of key names -> new values to override in the config
        :param complex_pop: whether to use the regular or complex config template
        :return: configuration json
        """
        configuration = self.load_config_template_json(complex_pop)
        for key, value in overrides.items():
            configuration[key] = value
        return configuration

    def write_demographics_file(self, demographics, demo_filename='demographics.json'):
        with open(demo_filename, 'w') as outfile:
            json.dump(demographics, outfile, indent=4, sort_keys=True)

    def write_demographics_nodes_file(self, nodes):
        demographics = self.load_demographics_template_json()
        demographics[DemographicsParameters.KEY_NODES] = nodes
        self.demographics = demographics
        self.write_demographics_file(demographics)

    def initialize_with_pop_and_overrides(self, initial_population, overrides, complex_pop=False):
        demographics = self.load_demographics_template_json(complex_pop)
        node = demographics[DemographicsParameters.KEY_NODES][0]
        self.change_node_ind_attribute(node, DemographicsParameters.INITIALPOPULATION_KEY, initial_population)
        self.write_demographics_nodes_file([node])
        config = self.create_config_with_overrides(overrides, complex_pop)
        self.write_config_file(config)

    #  endregion config and demographics functions

    #  region utility methods
    def dump_debug_info(self, method_name=''):
        print(f'Dumping debug info for {method_name}')
        # copy demographics to demographics_broken
        demo_dump_file = f'demographics_broken_{method_name}.json'
        print(f'Dumping demographics info to {demo_dump_file}')
        copyfile('demographics.json', demo_dump_file)
        # copy config to nd_broken
        config_dump_file = f'nd_broken_{method_name}.json'
        print(f'Dumping config info to {config_dump_file}')
        copyfile('nd.json', config_dump_file)
        for person in self.people:
            print(person.to_string())

    def get_age_bucket(self, age):
        """
        Determine age bucket of individual from age in days: birth, 0-18 months, 18m-4, 5-9, 10-14, 15-19, 20+

        :param age: age in days (float)
        :return: age bucket 0-6
        """
        if age == 0:            return 0
        elif age < 30 * 18:     return 1
        elif age < 365 * 5:     return 2
        elif age < 365 * 10:    return 3
        elif age < 365 * 15:    return 4
        elif age < 365 * 20:    return 5
        else:                   return 6

    def get_age_bucket_title(self, age_bucket):
        """
        Short string description of age bucket
        :param age_bucket: int 0-6
        :return: age bucket string
        """
        age_buckets = ['birth', '0-18', '18-4', '5-9', '10-14', '15-19', '20+']
        if 0 <= age_bucket < 7:
            return age_buckets[age_bucket]
        return ''

    def assert_mcw_match(self, total_mcw, population, precision, message):
        """
        Assert match of total monte-carlo weight and the expected population within a given precision

        :param total_mcw: Cummulative monte-carlo weight of all tracked invididuals
        :param population: expected population
        :param precision: fraction of expected population the total mcw is allowed to be off (e.g. 0.01 = 1%, 0.1 = 10%)
        :param message: extra message to print if the assertion fails
        """
        total_mcw_delta = abs((total_mcw - population)) / population
        msg = f'Cummulative mcw weight {total_mcw:0.2f} more than {precision} different from expected: {population} {message}'
        self.assertLess(total_mcw_delta, precision, msg)

    #  endregion utility methods

    #  region track all tests
    @given(initial_population=strategies.integers(min_value=1, max_value=1000))
    @settings(max_examples=50)
    def test_trackall_sampling(self, initial_population):
        """
        Test track-all sampling type

        :param initial_population: population to track
        """
        self.reset()

        self.initialize_with_pop_and_overrides(initial_population, { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.TRACKALL })

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        if len(self.people) != initial_population:
            self.dump_debug_info('test_trackall_sampling')

        self.assertEqual(len(self.people), initial_population)

        total_mcw = 0.0
        expected_mcw = 1.0
        for person in self.people:
            total_mcw += person.mcw
            self.assertEqual(person.mcw, expected_mcw, f'mcw does not match expected for init pop {initial_population}')

        self.assert_mcw_match(total_mcw, initial_population, 0.01, '')

    @given(initial_population=strategies.integers(min_value=1, max_value=1000))
    @settings(max_examples=50)
    def test_trackall_sampling_complex_pop(self, initial_population):
        """
        Test track-all sampling using complex population initialization.

        :param initial_population: population to track
        """
        self.reset()

        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.TRACKALL },
                                               complex_pop=True)

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        if len(self.people) != initial_population:
            self.dump_debug_info('test_trackall_sampling_complex_pop')

        total_mcw = 0.0
        expected_mcw = 1.0
        for person in self.people:
            total_mcw += person.mcw

            self.assertEqual(person.mcw, expected_mcw,
                             f'mcw does not match expected for person: {person.to_string()} (init pop {initial_population})')

        self.assert_mcw_match(total_mcw, initial_population, 0.01, '')

    @given(scale_factor=strategies.floats(min_value=0.1, max_value=1000.0, allow_infinity=False, allow_nan=False),
           expected_individuals=strategies.integers(min_value=100, max_value=1000))
    #@reproduce_failure('3.82.1', b'AAABAAAAeM7bcw==')
    def test_trackall_sampling_x_scale(self, scale_factor, expected_individuals):
        """
        Test track-all sampling using population scale factor.

        :param scale_factor:  population scale factor
        :param expected_individuals: expected number of agents (in a range of low values for speed)
        """
        self.reset()

        initial_population = int(expected_individuals * 1.0 / scale_factor)
        # compensate for rounding in the DTK
        tracked_individuals = int(initial_population * np.float(scale_factor))
        assume(tracked_individuals > 0)

        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.TRACKALL,
                                                 ConfigurationParameters.SCALEFACTOR_KEY: scale_factor } )

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        self.assertEqual(len(self.people), tracked_individuals, f'Number of people mismatch for scale_factor {scale_factor}')

        total_mcw = 0.0
        expected_mcw = 1.0
        for person in self.people:
            total_mcw += person.mcw
            self.assertEqual(person.mcw, expected_mcw, f'mcw does not match expected for init pop {initial_population}')

        self.assert_mcw_match(total_mcw, tracked_individuals, 0.05, '')

    #  endregion track all tests

    #  region fixed sampling tests
    @given(sample_rate=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
            expected_individuals = strategies.integers(min_value=100, max_value=1000))
    @settings(max_examples=50)
    def test_fixed_sampling(self, sample_rate, expected_individuals):
        """
        Test fixed sampling.

        :param sample_rate: fixed sampling rate
        :param expected_individuals: expected number of agents (in a range of low values for speed)
        """
        self.reset()

        initial_population = int(expected_individuals * 1.0 / sample_rate)
        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.FIXED,
                                                 ConfigurationParameters.SAMPLERATE_KEY: sample_rate })

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        total_mcw = 0.0
        expected_mcw = 1.0 / sample_rate
        for person in self.people:
            total_mcw += person.mcw
            self.assertAlmostEqual(person.mcw, expected_mcw, 3, f'mcw does not match expected for sampling rate {sample_rate}')

        # TODO: figure out what the appropriate test is for total MCW in this case
        #self.assert_mcw_match(total_mcw, initial_population, 0.01, f'for sample rate: {sample_rate}')

    @given(initial_population=strategies.integers(min_value=1, max_value=1000))
    @settings(max_examples=50)
    def test_fixed_sampling_rate_1(self, initial_population):
        """
        Use a fixed sampling rate of 1.0, should have identical behavior to track-all sampling

        :param initial_population: initial population
        """
        self.reset()

        sample_rate = 1.0
        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.FIXED,
                                                 ConfigurationParameters.SAMPLERATE_KEY: sample_rate })

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        total_mcw = 0.0
        expected_mcw = 1.0
        for person in self.people:
            total_mcw += person.mcw
            self.assertAlmostEqual(person.mcw, expected_mcw, 3, f'mcw does not match expected for initial pop: {initial_population}')

        self.assert_mcw_match(total_mcw, initial_population, 0.01, f'for sample rate 1.0, initial pop: {initial_population}')

    @given(sample_rate=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
            expected_individuals = strategies.integers(min_value=100, max_value=1000))
    @settings(max_examples=100)
    def test_fixed_sampling_complex_pop(self, sample_rate, expected_individuals):
        """
        Test fixed sampling with complex population initialization.

        :param sample_rate: sample rate
        :param expected_individuals: expected number of agents (in a range of low values for speed)
        """
        self.reset()

        initial_population = int(expected_individuals * 1.0 / sample_rate)
        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.FIXED,
                                                 ConfigurationParameters.SAMPLERATE_KEY: sample_rate },
                                               complex_pop=True)

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        total_mcw = 0.0
        expected_mcw = 1.0 / sample_rate
        for person in self.people:
            total_mcw += person.mcw
            age_bucket = self.get_age_bucket(person.age)

            self.assertAlmostEqual(person.mcw, expected_mcw, 3,
                                   f'mcw does not match expected for sampling rate {sample_rate} of age group {self.get_age_bucket_title(age_bucket)} for person {person.to_string()} with init pop {initial_population}')

        # TODO: figure out what the appropriate test is for total MCW in this case
        #self.assert_mcw_match(total_mcw, initial_population, 0.01, f'for sample rate: {sample_rate}')

    @given(initial_population=strategies.integers(min_value=1, max_value=1e9),
           sample_rate=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
           expected_individuals=strategies.integers(min_value=10, max_value=1000))
    def test_fixed_sampling_with_scaling(self, initial_population, sample_rate, expected_individuals):
        """
        Test fixed sampling plus population scaling.

        :param initial_population: initial population
        :param sample_rate: sample rate
        :param expected_individuals: expected number of agents (in a range of low values for speed)
        """
        self.reset()

        scale_factor = expected_individuals * 1.0 / (initial_population * sample_rate)
        tracked_individuals = int(initial_population * scale_factor * sample_rate)
        assume(tracked_individuals > 0)

        self.initialize_with_pop_and_overrides(initial_population,
                                               { ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.FIXED,
                                                 ConfigurationParameters.SAMPLERATE_KEY: sample_rate,
                                                 ConfigurationParameters.SCALEFACTOR_KEY: scale_factor })

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        total_mcw = 0.0
        expected_mcw = 1.0 / sample_rate
        for person in self.people:
            total_mcw += person.mcw
            self.assertAlmostEqual(person.mcw, expected_mcw, 3, f'mcw does not match expected w/ pop: {initial_population} x{scale_factor} @{sample_rate} = {tracked_individuals}')

        # TODO: figure out what the appropriate test is for total MCW in this case
        #self.assert_mcw_match(total_mcw, tracked_individuals, 0.01, f'w/ pop: {initial_population} x{scale_factor} @{sample_rate} = {tracked_individuals}')

    #  endregion fixed sampling tests

    #  region adapted sampling by age tests
    @given(sample_rate_0_18=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        sample_rate_18_4=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        sample_rate_5_9=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        sample_rate_10_14=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        sample_rate_15_19=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        sample_rate_20=strategies.floats(min_value=0.001, max_value=1.0, allow_infinity=False, allow_nan=False),
        expected_individuals=strategies.integers(min_value=100, max_value=1000))
    def test_adapted_sampling_by_age(self, sample_rate_0_18, sample_rate_18_4, sample_rate_5_9, sample_rate_10_14,
                                     sample_rate_15_19, sample_rate_20, expected_individuals):
        """
        Test adapted sampling by age using complex population initialization.

        :param sample_rate_0_18: 0-18m sample rate
        :param sample_rate_18_4: 18m-4yo sample rate
        :param sample_rate_5_9: 5-9yo sample rate
        :param sample_rate_10_14: 10-14yo sample rate
        :param sample_rate_15_19: 15-19yo sample rate
        :param sample_rate_20: 20+ sample rate
        :param expected_individuals: expected number of agents (in a range of low values for speed)
        """
        self.reset()

        sample_rate_birth = 1.0
        sample_rates = [sample_rate_birth, sample_rate_0_18, sample_rate_18_4, sample_rate_5_9, sample_rate_10_14, sample_rate_15_19, sample_rate_20]

        # pick an initial population which leads to the expected number of tracked invididuals
        initial_population = int(expected_individuals * 1.0 / sum(sample_rates[1:]))
        self.initialize_with_pop_and_overrides(initial_population,
                                               {ConfigurationParameters.SAMPLINGTYPE_KEY: SamplingTypes.ADAPTED_BY_AGE,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_BIRTH: sample_rate_birth,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_0_18M: sample_rate_0_18,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_18M_4: sample_rate_18_4,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_5_9: sample_rate_5_9,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_10_14: sample_rate_10_14,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_15_19: sample_rate_15_19,
                                                ConfigurationParameters.AgeGroup_Sample_Rate.KEY_20_PLUS: sample_rate_20
                                                },
                                               complex_pop=True)

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        age_counts = [0] * 7
        mcw_counts = [0.0] * 7
        total_mcw = 0.0

        for person in self.people:
            total_mcw += person.mcw
            age_bucket = self.get_age_bucket(person.age)
            sample_rate = sample_rates[age_bucket]
            expected_mcw = 1.0 / sample_rate
            age_counts[age_bucket] += 1
            mcw_counts[age_bucket] += person.mcw

            self.assertAlmostEqual(person.mcw, expected_mcw, 3,
                                   f'mcw does not match expected for sampling rate {sample_rate} of age group {self.get_age_bucket_title(age_bucket)} for person {person.to_string()}')

        # TODO: figure out what the appropriate test is for total MCW in this case
        #self.assert_mcw_match(total_mcw, tracked_individuals, 0.01, f'for sample rates: {sample_rates}')

    #  endregion adapted sampling by age tests


if __name__ == '__main__':
    unittest.main()