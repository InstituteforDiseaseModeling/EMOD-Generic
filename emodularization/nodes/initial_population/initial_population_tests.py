import json
import unittest
import dtk_nodedemog

from hypothesis import given, strategies, settings, example

class DemographicsParameters:
    KEY_NODES = "Nodes"
    NODEATTRIBUTES_KEY = "NodeAttributes"
    INITIALPOPULATION_KEY = "InitialPopulation"

class InitialPopulationTest(unittest.TestCase):
    def setUp(self):
        self.debug = False
        self.total_peeps = 0
        self.total_male = 0
        self.total_female = 0
        pass

    def tearDown(self):
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
    # endregion

    def test_single_node(self, initial_node_population=1234, debug=False):
        if debug:
            self.debug = True
        with open("demographics_template.json") as infile:
            demographics = json.load(infile)
        nodes = demographics[DemographicsParameters.KEY_NODES]
        temp_node = nodes[0]
        temp_node[DemographicsParameters.NODEATTRIBUTES_KEY][DemographicsParameters.INITIALPOPULATION_KEY] = initial_node_population
        newnodes = []
        newnodes.append(temp_node)
        demographics[DemographicsParameters.KEY_NODES] = newnodes

        with open("demographics.json", "w") as outfile:
            json.dump(demographics, outfile, indent=4, sort_keys=True)

        dtk_nodedemog.set_callback(self.count_people_callback)
        dtk_nodedemog.populate_from_files()

        self.assertEqual(initial_node_population, self.total_peeps)
        print "Females: {0}".format(self.total_female)
        print "Males: {0}".format(self.total_male)

    def test_mcw_population(self, sample_rate=0.1):
        pass

    def test_single_node2(self, initial_node_population=999, debug=False):
        if debug:
            self.debug = True
        demographics = self.get_demographics_template()
        single_node = self.get_demographics_nodes(demographics)[0]
        single_node = self.change_node_ind_attribute(new_value=initial_node_population,
                                                     att_key=DemographicsParameters.INITIALPOPULATION_KEY)
        demographics[DemographicsParameters.KEY_NODES] = [single_node]
        self.set_demographics_file(demographics)

        dtk_nodedemog.set_callback(self.count_people_callback)
        dtk_nodedemog.populate_from_files()

        self.assertEqual(initial_node_population, self.total_peeps)
        self.print_stats()

    @given(initial_node_population=strategies.integers(min_value=0, max_value=9999))
    @settings(max_examples = 50)
    def test_single_node3(self, initial_node_population):
        print "Hypothesis!"
        demographics = self.get_demographics_template()
        single_node = self.change_node_ind_attribute(initial_node_population, att_key=DemographicsParameters.INITIALPOPULATION_KEY)
        demographics[DemographicsParameters.KEY_NODES] = [single_node]
        self.set_demographics_file(demographics)

        dtk_nodedemog.set_callback(self.count_people_callback)
        dtk_nodedemog.populate_from_files()

        self.assertEqual(initial_node_population, self.total_peeps)
        self.print_stats()

    def test_multi_node(self, initial_node_populations=[1, 10, 100, 1000], debug=False):
        demographics = self.get_demographics_template()
        temp_nodes = []
        for i in initial_node_populations:
            temp_node = self.change_node_ind_attribute(i)
            temp_nodes.append(temp_node)
        demographics[DemographicsParameters.KEY_NODES] = temp_nodes

        dtk_nodedemog.set_callback(self.count_people_callback)
        dtk_nodedemog.populate_from_files()

        self.assertEqual(sum(initial_node_populations), self.total_peeps)
        self.print_stats()



