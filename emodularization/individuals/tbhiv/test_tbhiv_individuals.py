from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest

class TestTbhivModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestTbhivModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.debug = False
        self.expected_methods = None
        self.root_tbhiv_methods = [
            'get_cd4', 'get_infection_state', 'get_infection_state_age', 'get_infection_state_change',
            'get_state_change',  'hapi', 'has_active_infection', 'has_hiv', 'has_latent_infection',
            'has_tb', 'infect_with_hiv', 'initiate_pregnancy', 'is_dead', 'is_fast', 'reset_art_timers'
        ]
        self.required_input_methods = [
            'infect_with_hiv', 'give_intervention',
            'set_param', 'set_enum_param'
        ]
        self.required_root_methods = [
            'has_tb', 'get_cd4', 'has_active_infection', 'has_hiv',
            'is_fast', 'is_dead',
        ]
        self.toby = {}
        self.tina = {}
        import dtk_tbhiv_intrahost as test
        self.namespace_under_test = test
        self.namespace_under_test.reset()

    def tearDown(self):
        self.expected_methods = None

    def test_methods_and_properties_exist(self):
        self.expected_methods = self.root_methods + self.root_tbhiv_methods
        self.report_missing_tbhiv_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=self.namespace_under_test)

    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_tbhiv_toby_tina()
        observed_toby_age = test.get_age(self.toby['observed'])
        observed_tina_age = test.get_age(self.tina['observed'])
        # observed_toby_age = test.get_age(self.toby['observed'])
        # observed_tina_age = test.get_age(self.tina['observed'])
        self.check_property_of_individual('age', self.toby['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.tina['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_tbhiv_toby_tina()
        self.serialize_male_female(male=self.toby, female=self.tina)

        expected_toby = self.toby['expected']
        observed_toby = self.toby['serialized']['individual']
        self.compare_tbhiv_individual_with_expectations(expected_toby, observed_toby,
                                                          individual_label="Toby")

        expected_tina = self.tina['expected']
        observed_tina = self.tina['serialized']['individual']
        self.compare_tbhiv_individual_with_expectations(expected_tina, observed_tina,
                                                          individual_label="Tina")

    def test_update_individual(self):
        # self.debug = True
        test = self.namespace_under_test
        self.create_tbhiv_toby_tina()
        curr_expected_toby_age = self.toby['expected']['age']
        curr_expected_tina_age = self.tina['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        toby = self.toby['observed']
        tina = self.tina['observed']
        tina_became_possible_mother = test.is_possible_mother(tina)
        tina_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(tina_became_possible_mother, "Tina should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_toby_age,
                                              test.get_age(toby),
                                              individual_label="Toby")
            self.check_property_of_individual('age',
                                              curr_expected_tina_age,
                                              test.get_age(tina),
                                              individual_label="Tina")
            if not tina_became_possible_mother:
                tina_became_possible_mother = test.is_possible_mother(tina)
                if tina_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_tina_age
            elif not test.is_possible_mother(tina) and not tina_ceased_potential_motherhood:
                tina_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_tina_age
            test.update(toby)
            test.update(tina)
            curr_expected_tina_age += timestep_size
            curr_expected_toby_age += timestep_size
        self.assertTrue(tina_became_possible_mother, "Tina should have become a possible mother.")
        if self.debug:
            print(f"Tina became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(tina_ceased_potential_motherhood, "Tina should have ceased being a potential mother.")
        if self.debug:
            print(f"Tina ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_tbhiv_toby_tina(self):
        self.create_generic_male_female(male=self.toby, female=self.tina)
        self.toby['label'] = "Toby"
        self.tina['label'] = "Tina"

        for human in [self.toby, self.tina]:
            human['expected']['infectioncount_hiv'] = 0
            human['expected']['infectioncount_tb'] = 0
            human['expected']['m_bool_exogenous'] = False
            human['expected']['m_has_ever_been_onART'] = False
            human['expected']['m_has_ever_tested_positive_for_HIV'] = False

    def compare_tbhiv_individual_with_expectations(self, expected_individual,
                                                   observed_serialized_individual,
                                                   individual_label=None):
        if not individual_label:
            individual_label = f"Individual with id {observed_serialized_individual['suid']['id']}"
        self.compare_generic_individual_with_expectations(expected_individual,
                                                          observed_serialized_individual,
                                                          individual_label)

        tbhiv_properties = [
            'infectioncount_hiv', 'infectioncount_tb', 'm_bool_exogenous', 'm_has_ever_been_onART',
            'm_has_ever_tested_positive_for_HIV'
        ]


    def report_missing_tbhiv_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods(namespace_name='dtk_tbhiv_intrahost',
                                    methodlist=methodlist,
                                    namespace=imported_namespace)


