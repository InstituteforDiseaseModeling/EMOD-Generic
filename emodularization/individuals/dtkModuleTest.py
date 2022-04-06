import unittest
__unittest = True
import json

class DtkModuleTest(unittest.TestCase):
    def setUp(self):
        self.root_methods = [
            'create', 'get_age','get_immunity', 'get_individual_for_iv', 'get_infection_age',
            'get_infectiousness', 'get_schema', 'give_intervention', 'is_infected',
            'is_possible_mother', 'is_pregnant', 'my_set_callback', 'reset','serialize',
            'set_deposit_callback', 'set_enum_param', 'set_mortality_callback', 'set_param',
            'should_infect', 'update', 'update1', 'update2', 'update_pregnancy'
        ]
        self.debug = False
        self.namespace_under_test = None


    def create_generic_male_female(self, male, female):
        expected_male = {}
        expected_male['age'] = 7300.0
        expected_male['sex'] = 0
        expected_male['mcw'] = 1.1

        expected_female = {}
        expected_female['age'] = 3650.0
        expected_female['sex'] = 1
        expected_female['mcw'] = 0.9



        for human in [expected_male, expected_female]:
            human['cumulativeInfs'] = 0
            human['family_migration_is_destination_new_home'] = False
            human['family_migration_time_at_destination'] = 0
            human['family_migration_time_until_trip'] = 0
            human['family_migration_type'] = 0
            human['Inf_Sample_Rate'] = 0
            human['infections'] = []
            human['infectiousness'] = 0
            human['is_on_family_trip'] = False
            human['is_pregnant'] = False
            human['leave_on_family_trip'] = False
            human['m_daily_mortality_rate'] = 0
            human['m_is_infected'] = False
            human['m_new_infection_state'] = 0
            human['m_newly_symptomatic'] = False
            human['max_waypoints'] = 0
            human['migration_is_destination_new_home'] = False
            human['migration_mod'] = 0
            human['migration_outbound'] = True
            human['migration_time_at_destination'] = 0
            human['migration_time_until_trip'] = 0
            human['migration_type'] = 0
            human['migration_will_return'] = True
            human['pregnancy_timer'] = 0
            human['Properties'] = []
            human['StateChange'] = 0
            human['waiting_for_family_trip'] = False
            human['waypoints'] = []
            human['waypoints_trip_type'] = []


        male['expected'] = expected_male  # Male, 23 years, monte carlo weight 1.0
        female['expected'] = expected_female  # Female, 10 years, monte carlo weight 1.0

        male_human = self.create_individual_hook(expected_male['sex'],
                                                 expected_male['age'],
                                                 expected_male['mcw'])
        female_human = self.create_individual_hook(expected_female['sex'],
                                                   expected_female['age'],
                                                   expected_female['mcw'])
        male['observed'] = male_human
        female['observed'] = female_human

    def serialize_male_female(self, male, female):
        self.assertTrue(male, "Male should be populated before serializing.")
        self.assertTrue(female, "Female should be populated before serializing.")
        male['serialized'] = self.serialize_individual_hook(male['observed'], male['label'])
        female['serialized'] = self.serialize_individual_hook(female['observed'], female['label'])

    def create_individual_hook(self, sex_int, age_float, mcw_float):
        if self.debug:
            print(f'Create human with sex: {sex_int} age: {age_float} mcw: {mcw_float}\n')
        return self.namespace_under_test.create((sex_int, age_float, mcw_float))

    def serialize_individual_hook(self, individual, individual_label=None):
        if self.debug:
            print(f'Serialize human: {individual} label: {individual_label}\n')
        individual_text = self.namespace_under_test.serialize(individual)
        individual_json = json.loads(individual_text)
        if self.debug:
            if not individual_label:
                individual_label = f"Individual with id {individual_json['individual']['suid']['id']}"
            with open(f'{individual_label}.json', 'w') as outfile:
                json.dump(individual_json, outfile, indent=4, sort_keys=True)
        return individual_json

    def check_property_of_individual(self, property_under_test, expected_value,
                                     observed_value, individual_label=None):
        self.assertEqual(expected_value, observed_value, msg=f'{individual_label} should have {property_under_test} '
                                                             f'of {expected_value}, observed: {observed_value}.')

    def compare_generic_individual_with_expectations(self, expected_individual,
                                                     observed_serialized_individual,
                                                     individual_label=None):
        if not individual_label:
            individual_label = f"Individual with id {observed_serialized_individual['suid']['id']}"
        self.check_property_of_individual('age', expected_individual['age'],
                                          observed_serialized_individual['m_age'],
                                          individual_label=individual_label)
        self.check_property_of_individual("sex", expected_individual['sex'],
                                          observed_serialized_individual['m_gender'],
                                          individual_label=individual_label)
        self.check_property_of_individual("monte carlo weight", expected_individual['mcw'],
                                          observed_serialized_individual['m_mc_weight'],
                                          individual_label=individual_label)

        serialized_properties = [
            'cumulativeInfs', 'family_migration_is_destination_new_home',
            'family_migration_time_at_destination', 'family_migration_time_until_trip',
            'family_migration_type', 'Inf_Sample_Rate', 'infections', 'infectiousness',
            'is_on_family_trip', 'is_pregnant', 'leave_on_family_trip', 'm_daily_mortality_rate',
            'm_is_infected', 'm_new_infection_state', 'm_newly_symptomatic', 'max_waypoints',
            'migration_is_destination_new_home', 'migration_outbound',
            'migration_time_at_destination', 'migration_time_until_trip', 'migration_type',
            'migration_will_return', 'pregnancy_timer', 'Properties', 'StateChange',
            'waiting_for_family_trip', 'waypoints', 'waypoints_trip_type'
        ]

        for sp in serialized_properties:
            self.check_property_of_individual(sp, expected_individual[sp],
                                              observed_serialized_individual[sp],
                                              individual_label=individual_label)

    def report_missing_methods(self, namespace_name, methodlist, namespace, package_name=None):
        '''

        :param namespace_name: label for logging messages
        :param methodlist: array of properties to check for
        :param namespace: the actual thing that you imported, we'll call dir() on this
        :param package_name: defaults to None, doesn't seem to do anything yet
        :return:
        '''
        self.assertIsNotNone(namespace)
        listing = dir(namespace)
        missing_properties = []
        for em in methodlist:
            if em not in listing:
                missing_properties.append(em)
        self.assertEqual(0, len(missing_properties), msg=f"Expected no missing properties in {namespace_name},"
                                                         f" missing: {str(missing_properties)}")
