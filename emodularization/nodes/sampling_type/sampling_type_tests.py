import json
import unittest
import dtk_nodedemog

from hypothesis import given, strategies, settings, example

class DemographicsParameters:
    KEY_NODES = "Nodes"
    NODEATTRIBUTES_KEY = "NodeAttributes"
    INITIALPOPULATION_KEY = "InitialPopulation"

class SamplingTypes:
    TRACKALL = "TRACK_ALL"
    FIXED = "FIXED_SAMPLING"

class ConfigurationParameters:
    SAMPLINGTYPE_KEY = "Individual_Sampling_Type"
    SCALEFACTOR_KEY = "x_Base_Population"
    SAMPLERATE_KEY = "Base_Individual_Sample_Rate"

class Person:
    def __init__(self, mcw, age, gender):
        self.mcw = mcw
        self.age = age
        self.gender = gender

class SamplingTypeTest(unittest.TestCase):
    def setUp(self):
        self.debug = False
        self.total_peeps = 0
        self.total_male = 0
        self.total_female = 0
        self.people_objects = []
        pass

    def tearDown(self):
        self.total_peeps = 0
        self.total_male = 0
        self.total_female = 0
        self.people_objects = []
        pass

    # region debugging
    def print_stats(self):
        print "Total peeps: {0}".format(self.total_peeps)
        print "Total males: {0}".format(self.total_male)
        print "Total females: {0}".format(self.total_female)

    # endregion

    # region callbacks
    def console_callback(self, mcw, age, gender):
        message = "New individual created. MCW: {0}\tAge: {1}\tGender: {2}"
        print message.format(mcw, age, gender)

    def count_people_callback(self, mcw, age, gender):
        self.total_peeps +=1
        if gender == 0:
            self.total_male += 1
        else:
            self.total_female += 1

    def accumulate_people_callback(self, mcw, age, gender):
        temp_person = Person(mcw, age, gender)
        self.people_objects.append(temp_person)

    # endregion

    # region configuration editing
    def get_config_template(self, config_filename="nd_template.json"):
        with open(config_filename) as infile:
            configuration = json.load(infile)
        return configuration

    def change_config_parameter(self, new_value, configuration=None, param_key=ConfigurationParameters.SCALEFACTOR_KEY):
        if not configuration:
            configuration = self.get_config_template()
        configuration[param_key] = new_value
        return configuration

    def set_config_file(self, configuration, config_filename="nd.json"):
        with open (config_filename, 'w') as outfile:
            json.dump(configuration, outfile, indent=4, sort_keys=True)
    # endregion

    # region demographics editing
    def get_demographics_template(self, demo_filename="demographics_template.json"):
        with open(demo_filename) as infile:
            demographics = json.load(infile)
        return demographics

    def get_demographics_nodes(self, demographics=None):
        if not demographics:
            demographics = self.get_demographics_template()
        nodes = demographics[DemographicsParameters.KEY_NODES]
        return nodes

    def change_node_ind_attribute(self, new_value, node=None, att_key=DemographicsParameters.INITIALPOPULATION_KEY):
        if not node:
            node = self.get_demographics_nodes()[0]
        temp_node = node
        temp_node[DemographicsParameters.NODEATTRIBUTES_KEY][att_key] = new_value
        return temp_node

    def set_demographics_file(self, demographics, demo_filename="demographics.json"):
        with open(demo_filename, 'w') as outfile:
            json.dump(demographics, outfile, indent=4, sort_keys=True)

    def set_demographics_singlenode_file(self, node):
        demographics = self.get_demographics_template()
        nodes = []
        nodes.append(node)
        demographics[DemographicsParameters.KEY_NODES] = nodes
        self.demographics = demographics
        self.set_demographics_file(demographics)

    # endregion

    @given(sample_rate=strategies.floats(min_value=0.1, max_value=100.0, allow_infinity=None, allow_nan=None))
    @settings(max_examples=50)
    def test_fixed_sampling(self, sample_rate):
        node = self.change_node_ind_attribute(1000)
        self.set_demographics_singlenode_file(node)

        config = self.change_config_parameter(sample_rate,
                                              param_key=ConfigurationParameters.SAMPLERATE_KEY)
        config = self.change_config_parameter(SamplingTypes.FIXED,
                                              configuration=config,
                                              param_key=ConfigurationParameters.SAMPLINGTYPE_KEY)
        self.set_config_file(config)

        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        expected_mcw = 1.0 / sample_rate
        err_msg = "With sample rate {0}, expected mcw {1} got {2}"
        print "People: {0} Sample rate {1}".format(len(self.people_objects), sample_rate)
        errs = []
        for guy in self.people_objects:
            if abs(guy.mcw - expected_mcw) > 0.001:
                errs.append(err_msg.format(sample_rate, expected_mcw, guy.mcw))
        if len(errs) > 0:
            self.assertEqual(0, len(errs), "Expected zero errors, got: {0} like this: {1}".format(len(errs), errs[0]))

    @given(initial_population= strategies.integers(min_value=1, max_value=1000))
    @settings(max_examples=50)
    def test_trackall_sampling(self, initial_population):
        self.people_objects = []
        node = self.change_node_ind_attribute(initial_population)
        self.set_demographics_singlenode_file(node)

        config = self.change_config_parameter(SamplingTypes.TRACKALL,
                                              param_key=ConfigurationParameters.SAMPLINGTYPE_KEY)
        self.set_config_file(config)

        print "InitPop: {0}".format(initial_population)
        dtk_nodedemog.set_callback(self.accumulate_people_callback)
        dtk_nodedemog.populate_from_files()

        if len(self.people_objects) != initial_population:
            with open("demographics_broken.json","w") as outfile:
                json.dump(self.demographics, outfile, indent=4, sort_keys=True)
            with open("nd_broken.json","w") as outfile:
                json.dump(config, outfile, indent=4, sort_keys=True)
            for guy in self.people_objects:
                print(guy.mcw, guy.age, guy.gender)
        self.assertEqual(len(self.people_objects), initial_population)
        err_msg_template = "With Track All, expected mcw 1.0 got {0}"

        errs = []
        for guy in self.people_objects:
            if guy.mcw != 1.0:
                errs.append(err_msg.format(guy.mcw))
        self.assertEqual(0, len(errs))
        if len(errs) > 0:
            print "Expected 0 errors, got {0} like {1}".format(len(errs), errs[0])




