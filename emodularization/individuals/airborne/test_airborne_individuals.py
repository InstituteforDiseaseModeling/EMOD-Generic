from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest
import unittest
__unittest = True


class TestAirborneModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestAirborneModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.andy = {}
        self.anna = {}
        import dtk_airborne_intrahost as test
        test.reset()
        self.namespace_under_test = test
        self.debug = True # TODO: set to false after DtkTrunk 3849 closed

    @unittest.skip("NYR: Not ready for prime time DtkTrunk 3849")
    def test_reset_method(self):
        test = self.namespace_under_test
        initial_ids = []
        for x in range(0,10):
            joe = self.create_individual_hook(0, 7300, 1.0)
            joe_json = self.serialize_individual_hook(joe)
            initial_ids.append(joe_json['individual']['suid']['id'])
        for x in range(0,10):
            jack = self.create_individual_hook(0, 7300, 1.0)
            jack_json = self.serialize_individual_hook(jack)
            self.assertNotIn(jack_json['individual']['suid']['id'], initial_ids, msg="Each id should be unique before reset.")
        test.reset()
        for x in range(0,10):
            jack = self.create_individual_hook(0, 7300, 1.0)
            jack_json = self.serialize_individual_hook(jack)
            self.assertIn(jack_json['individual']['suid']['id'], initial_ids, msg="Each id should be reused after reset.")

    def test_methods_and_properties_exist(self):
        # TODO: retest after DtkTrunk 3850 resolved
        test = self.namespace_under_test
        self.expected_methods = self.root_methods
        self.report_missing_airborne_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=test)

    @unittest.skip("NYR: Not ready for prime time DtkTrunk 3849")
    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_airborne_andy_anna()
        observed_toby_age = test.get_age(self.andy['observed'])
        observed_tina_age = test.get_age(self.anna['observed'])
        self.check_property_of_individual('age', self.andy['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.anna['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    #@unittest.skip("NYR: Not ready for prime time DtkTrunk 3849")
    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_airborne_andy_anna()
        self.serialize_male_female(male=self.andy, female=self.anna)

        expected_andy = self.andy['expected']
        observed_andy = self.andy['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_andy, observed_andy,
                                                          individual_label="andy")

        expected_anna = self.anna['expected']
        observed_anna = self.anna['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_anna, observed_anna,
                                                          individual_label="anna")

    @unittest.skip("NYR: Not ready for prime time DtkTrunk 3849")
    def test_update_individual(self):
        test = self.namespace_under_test
        # self.debug = True
        self.create_airborne_andy_anna()
        curr_expected_andy_age = self.andy['expected']['age']
        curr_expected_anna_age = self.anna['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        andy = self.andy['observed']
        anna = self.anna['observed']
        anna_became_possible_mother = test.is_possible_mother(anna)
        anna_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(anna_became_possible_mother, "anna should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_andy_age,
                                              test.get_age(andy),
                                              individual_label="andy")
            self.check_property_of_individual('age',
                                              curr_expected_anna_age,
                                              test.get_age(anna),
                                              individual_label="anna")
            if not anna_became_possible_mother:
                anna_became_possible_mother = test.is_possible_mother(anna)
                if anna_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_anna_age
            elif not test.is_possible_mother(anna) and not anna_ceased_potential_motherhood:
                anna_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_anna_age
            test.update(andy)
            test.update(anna)
            curr_expected_anna_age += timestep_size
            curr_expected_andy_age += timestep_size
        self.assertTrue(anna_became_possible_mother, "anna should have become a possible mother.")
        if self.debug:
            print(f"anna became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(anna_ceased_potential_motherhood, "anna should have ceased being a potential mother.")
        if self.debug:
            print(f"anna ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_airborne_andy_anna(self):
        self.create_generic_male_female(male=self.andy, female=self.anna)
        self.andy['label'] = "andy"
        self.anna['label'] = "anna"
        pass

    def report_missing_airborne_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods("dtk_airborne_intrahost",
                                    methodlist=methodlist,
                                    namespace=imported_namespace)

