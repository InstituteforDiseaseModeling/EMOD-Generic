from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest
__unittest = True

class TestHIVModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestHIVModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.harry = {}
        self.helen = {}
        import dtk_hiv_intrahost as test
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
            try:
                jack_json = self.serialize_individual_hook(jack)
                self.assertNotIn(jack_json['individual']['suid']['id'], initial_ids, msg="Each id should be unique before reset.")
            except Exception as ex:
                print( str( ex ) )
        test.reset()
        for x in range(0,10):
            jack = self.create_individual_hook(0, 7300, 1.0)
            jack_json = self.serialize_individual_hook(jack)
            print( "jack = {0}, initial_ids = {1}".format( jack, initial_ids ) )
            self.assertIn(jack_json['individual']['suid']['id'], initial_ids, msg="Each id should be reused after reset.")

    def test_methods_and_properties_exist(self):
        test = self.namespace_under_test
        self.expected_methods = self.root_methods
        self.report_missing_hiv_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=test)

    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_hiv_harry_helen()
        observed_toby_age = test.get_age(self.harry['observed'])
        observed_tina_age = test.get_age(self.helen['observed'])
        self.check_property_of_individual('age', self.harry['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.helen['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_hiv_harry_helen()
        self.serialize_male_female(male=self.harry, female=self.helen)

        expected_harry = self.harry['expected']
        observed_harry = self.harry['serialized']['individual']
        self.compare_hiv_individual_with_expectations(expected_harry, observed_harry,
                                                      individual_label="harry")

        expected_helen = self.helen['expected']
        observed_helen = self.helen['serialized']['individual']
        self.compare_hiv_individual_with_expectations(expected_helen, observed_helen,
                                                      individual_label="helen")

    def test_update_individual(self):
        test = self.namespace_under_test
        # self.debug = True
        self.create_hiv_harry_helen()
        curr_expected_harry_age = self.harry['expected']['age']
        curr_expected_helen_age = self.helen['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        harry = self.harry['observed']
        helen = self.helen['observed']
        helen_became_possible_mother = test.is_possible_mother(helen)
        helen_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(helen_became_possible_mother, "helen should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_harry_age,
                                              test.get_age(harry),
                                              individual_label="harry")
            self.check_property_of_individual('age',
                                              curr_expected_helen_age,
                                              test.get_age(helen),
                                              individual_label="helen")
            if not helen_became_possible_mother:
                helen_became_possible_mother = test.is_possible_mother(helen)
                if helen_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_helen_age
            elif not test.is_possible_mother(helen) and not helen_ceased_potential_motherhood:
                helen_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_helen_age
            test.update(harry)
            test.update(helen)
            curr_expected_helen_age += timestep_size
            curr_expected_harry_age += timestep_size
        self.assertTrue(helen_became_possible_mother, "helen should have become a possible mother.")
        if self.debug:
            print(f"helen became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(helen_ceased_potential_motherhood, "helen should have ceased being a potential mother.")
        if self.debug:
            print(f"helen ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_hiv_harry_helen(self):
        self.create_generic_male_female(male=self.harry, female=self.helen)
        self.harry['label'] = "harry"
        self.helen['label'] = "helen"

        for human in [self.harry, self.helen]:
            human['expected']['active_relationships'] = [0,0,0,0]
            # 'co_infective_factor': 721141000000000000
            human['expected']['delay_between_adding_relationships_timer'] = 0
            human['expected']['has_other_sti_co_infection'] = False
            # TODO: figure out how to handle non-obvious (max_relationships) properties
            human['expected']['last_12_month_relationships'] = []
            human['expected']['last_6_month_relationships'] = []
            human['expected']['m_AssortivityIndex'] = [-1,-1,-1,-1]
            human['expected']['m_TotalCoitalActs'] = 0
            # 'max_relationships' = [16835440, 538976288, 538976288, 538976288]
            human['expected']['migrating_because_of_partner'] = False
            human['expected']['neg_num_partners_while_CD4500plus'] = 0
            human['expected']['num_lifetime_relationships'] = [0,0,0,0]
            human['expected']['num_unique_partners'] = [
                [[],[],[],[]],
                [[],[],[],[]],
                [[],[],[],[]],
                [[],[],[],[]]
            ]
            human['expected']['pos_num_partners_while_CD4500plus'] = 0
            human['expected']['potential_exposure_flag'] = False
            human['expected']['promiscuity_flags'] = 0
            human['expected']['queued_relationships'] = [0,0,0,0]
            human['expected']['relationship_properties'] = []
            human['expected']['relationships'] = []
            human['expected']['relationshipSlots'] = 0
            human['expected']['slot2RelationshipDebugMap'] = []
            human['expected']['transmissionInterventionsDisabled'] = False

    def compare_hiv_individual_with_expectations(self, expected_individual,
                                                 observed_serialized_individual,
                                                 individual_label=None):
        if not individual_label:
            individual_label = f"Individual with id {observed_serialized_individual['suid']['id']}"
        self.compare_generic_individual_with_expectations(expected_individual,
                                                          observed_serialized_individual,
                                                          individual_label)
        hiv_properties = [
            'active_relationships', 'delay_between_adding_relationships_timer', 
            'has_other_sti_co_infection', 'last_12_month_relationships', 'last_6_month_relationships',
            'm_AssortivityIndex', 'm_TotalCoitalActs', 'migrating_because_of_partner',
            'neg_num_partners_while_CD4500plus', 'num_lifetime_relationships', 'num_unique_partners',
            'pos_num_partners_while_CD4500plus', 'potential_exposure_flag', 'promiscuity_flags',
            'queued_relationships', 'relationship_properties', 'relationships',
            'relationshipSlots', 'slot2RelationshipDebugMap', 'transmissionInterventionsDisabled'
        ]

        for h in hiv_properties:
            self.check_property_of_individual(h, expected_individual[h],
                                              observed_serialized_individual[h],
                                              individual_label=individual_label)
        pass

    def report_missing_hiv_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods("dtk_hiv_intrahost",
                                    methodlist=methodlist,
                                    namespace=imported_namespace)

