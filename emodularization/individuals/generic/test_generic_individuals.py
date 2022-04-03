from pathlib import Path
import sys
sys.path.append( str(Path('..').resolve().absolute()) )
from dtkModuleTest import DtkModuleTest
__unittest = True


class TestGenericModules(DtkModuleTest):
    def __init__(self, *args, **kwargs):
        super(TestGenericModules, self).__init__(*args, **kwargs)

    def setUp(self):
        super().setUp()
        self.gene = {}
        self.gina = {}
        import dtk_generic_intrahost as test
        self.namespace_under_test = test
        test.reset()

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
        self.report_missing_generic_intrahost_methods(methodlist=self.expected_methods,
                                                      imported_namespace=test)

    def test_create_individual(self):
        test = self.namespace_under_test
        self.create_generic_gene_gina()
        observed_toby_age = test.get_age(self.gene['observed'])
        observed_tina_age = test.get_age(self.gina['observed'])
        self.check_property_of_individual('age', self.gene['expected']['age'],
                                          observed_toby_age, individual_label="Toby")
        self.check_property_of_individual('age', self.gina['expected']['age'],
                                          observed_tina_age, individual_label="Tina")

    def test_serialize_individual(self):
        # self.debug = True  # uncomment to drop serialized people to disk
        self.create_generic_gene_gina()
        self.serialize_male_female(male=self.gene, female=self.gina)

        expected_gene = self.gene['expected']
        observed_gene = self.gene['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_gene, observed_gene,
                                                          individual_label="gene")

        expected_gina = self.gina['expected']
        observed_gina = self.gina['serialized']['individual']
        self.compare_generic_individual_with_expectations(expected_gina, observed_gina,
                                                          individual_label="gina")

    def test_update_individual(self):
        test = self.namespace_under_test
        # self.debug = True
        self.create_generic_gene_gina()
        curr_expected_gene_age = self.gene['expected']['age']
        curr_expected_gina_age = self.gina['expected']['age']
        timestep_size = 1 # TODO: figure out how to set / configure this
        gene = self.gene['observed']
        gina = self.gina['observed']
        gina_became_possible_mother = test.is_possible_mother(gina)
        gina_ceased_potential_motherhood = False
        age_of_potential_motherhood = None
        age_of_ceased_motherhood_potential = None
        self.assertFalse(gina_became_possible_mother, "gina should not be a possible mother at creation.")
        for x in range(14600):
            self.check_property_of_individual('age',
                                              curr_expected_gene_age,
                                              test.get_age(gene),
                                              individual_label="gene")
            self.check_property_of_individual('age',
                                              curr_expected_gina_age,
                                              test.get_age(gina),
                                              individual_label="gina")
            if not gina_became_possible_mother:
                gina_became_possible_mother = test.is_possible_mother(gina)
                if gina_became_possible_mother:
                    age_of_potential_motherhood = curr_expected_gina_age
            elif not test.is_possible_mother(gina) and not gina_ceased_potential_motherhood:
                gina_ceased_potential_motherhood = True
                age_of_ceased_motherhood_potential = curr_expected_gina_age
            test.update(gene)
            test.update(gina)
            curr_expected_gina_age += timestep_size
            curr_expected_gene_age += timestep_size
        self.assertTrue(gina_became_possible_mother, "gina should have become a possible mother.")
        if self.debug:
            print(f"gina became a possible mother at age {age_of_potential_motherhood}.\nm")
        self.assertTrue(gina_ceased_potential_motherhood, "gina should have ceased being a potential mother.")
        if self.debug:
            print(f"gina ceased potential motherhood at age {age_of_ceased_motherhood_potential}.\n")

    def create_generic_gene_gina(self):
        self.create_generic_male_female(male=self.gene, female=self.gina)
        self.gene['label'] = "Gene"
        self.gina['label'] = "Gina"
        pass

    def report_missing_generic_intrahost_methods(self, methodlist, imported_namespace):
        self.report_missing_methods("dtk_generic_intrahost",
                                    methodlist=methodlist,
                                    namespace=imported_namespace)

