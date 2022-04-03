from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest
import unittest
__unittest = True


class TestMalariaModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestMalariaModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.mark = {}
        self.mary = {}
        import dtk_malaria_intrahost as test
        self.namespace_under_test = test
        test.reset()
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
        self.report_missing_malaria_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=test)

    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_malaria_mark_mary()
        observed_toby_age = test.get_age(self.mark['observed'])
        observed_tina_age = test.get_age(self.mary['observed'])
        self.check_property_of_individual('age', self.mark['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.mary['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_malaria_mark_mary()
        self.serialize_male_female(male=self.mark, female=self.mary)

        expected_mark = self.mark['expected']
        observed_mark = self.mark['serialized']['individual']
        self.compare_malaria_individual_with_expectations(expected_mark, observed_mark,
                                                          individual_label="mark")

        expected_mary = self.mary['expected']
        observed_mary = self.mary['serialized']['individual']
        self.compare_malaria_individual_with_expectations(expected_mary, observed_mary,
                                                          individual_label="mary")

    @unittest.skip("Not yet ready: crashing")
    def test_update_individual(self): # TODO: crashing!
        test = self.namespace_under_test
        # self.debug = True
        self.create_malaria_mark_mary()
        curr_expected_mark_age = self.mark['expected']['age']
        curr_expected_mary_age = self.mary['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        mark = self.mark['observed']
        mary = self.mary['observed']
        mary_became_possible_mother = test.is_possible_mother(mary)
        mary_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(mary_became_possible_mother, "mary should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_mark_age,
                                              test.get_age(mark),
                                              individual_label="mark")
            self.check_property_of_individual('age',
                                              curr_expected_mary_age,
                                              test.get_age(mary),
                                              individual_label="mary")
            if not mary_became_possible_mother:
                mary_became_possible_mother = test.is_possible_mother(mary)
                if mary_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_mary_age
            elif not test.is_possible_mother(mary) and not mary_ceased_potential_motherhood:
                mary_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_mary_age
            test.update(mark)
            test.update(mary)
            curr_expected_mary_age += timestep_size
            curr_expected_mark_age += timestep_size
        self.assertTrue(mary_became_possible_mother, "mary should have become a possible mother.")
        if self.debug:
            print(f"mary became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(mary_ceased_potential_motherhood, "mary should have ceased being a potential mother.")
        if self.debug:
            print(f"mary ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_malaria_mark_mary(self):
        self.create_generic_male_female(male=self.mark, female=self.mary)
        self.mark['label'] = "mark"
        self.mary['label'] = "mary"

        for human in [self.mark, self.mary]:
            human['expected']['m_clinical_symptoms'] = [False, False, False]
            human['expected']['m_female_gametocytes'] = 0
            human['expected']['m_female_gametocytes_by_strain'] = []
            human['expected']['m_gametocytes_detected'] = 0
            human['expected']['m_initial_infected_hepatocytes'] = 0
            human['expected']['m_male_gametocytes'] = 0
            human['expected']['m_parasites_detected_by_blood_smear'] = 0
            human['expected']['m_parasites_detected_by_new_diagnostic'] = 0
            human['expected']['m_strain_exposure'] = []
            human['expected']['m_total_exposure'] = 0

    def compare_malaria_individual_with_expectations(self, expected_individual,
                                                     observed_serialized_individual,
                                                     individual_label=None):
        if not individual_label:
            individual_label = f"Individual with id {observed_serialized_individual['suid']['id']}"
        self.compare_generic_individual_with_expectations(expected_individual,
                                                          observed_serialized_individual,
                                                          individual_label)
        malaria_properties = [
            'm_clinical_symptoms','m_female_gametocytes', 'm_female_gametocytes_by_strain',
            'm_gametocytes_detected', 'm_initial_infected_hepatocytes', 'm_male_gametocytes',
            'm_parasites_detected_by_blood_smear', 'm_parasites_detected_by_new_diagnostic',
            'm_strain_exposure', 'm_total_exposure'
        ]

        for m in malaria_properties:
            self.check_property_of_individual(m, expected_individual[m],
                                              observed_serialized_individual[m],
                                              individual_label=individual_label)


    def report_missing_malaria_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods("dtk_malaria_intrahost",
                                    methodlist=methodlist,
                                    namespace=imported_namespace)

