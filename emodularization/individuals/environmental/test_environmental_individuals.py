from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest
__unittest = True


class TestEnvironmentalModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestEnvironmentalModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.edgar = {}
        self.elsie = {}
        import dtk_environmental_intrahost as test
        test.reset()
        self.namespace_under_test = test
        self.debug = False

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
        test = self.namespace_under_test
        self.expected_methods = self.root_methods
        self.report_missing_environmental_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=test)

    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_environmental_edgar_elsie()
        observed_toby_age = test.get_age(self.edgar['observed'])
        observed_tina_age = test.get_age(self.elsie['observed'])
        self.check_property_of_individual('age', self.edgar['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.elsie['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_environmental_edgar_elsie()
        self.serialize_male_female(male=self.edgar, female=self.elsie)

        expected_edgar = self.edgar['expected']
        observed_edgar = self.edgar['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_edgar, observed_edgar,
                                                          individual_label="edgar")

        expected_elsie = self.elsie['expected']
        observed_elsie = self.elsie['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_elsie, observed_elsie,
                                                          individual_label="elsie")

    def test_update_individual(self):
        test = self.namespace_under_test
        # self.debug = True
        self.create_environmental_edgar_elsie()
        curr_expected_edgar_age = self.edgar['expected']['age']
        curr_expected_elsie_age = self.elsie['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        edgar = self.edgar['observed']
        elsie = self.elsie['observed']
        elsie_became_possible_mother = test.is_possible_mother(elsie)
        elsie_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(elsie_became_possible_mother, "elsie should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_edgar_age,
                                              test.get_age(edgar),
                                              individual_label="edgar")
            self.check_property_of_individual('age',
                                              curr_expected_elsie_age,
                                              test.get_age(elsie),
                                              individual_label="elsie")
            if not elsie_became_possible_mother:
                elsie_became_possible_mother = test.is_possible_mother(elsie)
                if elsie_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_elsie_age
            elif not test.is_possible_mother(elsie) and not elsie_ceased_potential_motherhood:
                elsie_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_elsie_age
            test.update(edgar)
            test.update(elsie)
            curr_expected_elsie_age += timestep_size
            curr_expected_edgar_age += timestep_size
        self.assertTrue(elsie_became_possible_mother, "elsie should have become a possible mother.")
        if self.debug:
            print(f"elsie became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(elsie_ceased_potential_motherhood, "elsie should have ceased being a potential mother.")
        if self.debug:
            print(f"elsie ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_environmental_edgar_elsie(self):
        self.create_generic_male_female(male=self.edgar, female=self.elsie)
        self.edgar['label'] = "edgar"
        self.elsie['label'] = "elsie"
        pass

    def report_missing_environmental_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods("dtk_environmental_intrahost",
                                    methodlist=methodlist,
                                    namespace=imported_namespace)

