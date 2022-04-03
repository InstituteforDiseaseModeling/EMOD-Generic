import random
import math
import json
from string import Template

def generate_ages(max_age, num_buckets):
    ages = [0]
    # ages = [] # first age bucket can be any value
    while num_buckets > 1: # num_buckets > 0 if first age bucket is not defined yet
        random_age = random.random() * max_age
        random_age = 1 if random_age == 0 else random_age # make sure no duplicate age
        ages.append(random_age)
        num_buckets -= 1
    final_ages = norm_list_to_max(ages, max_age, 7)
    return final_ages

def norm_list_to_max(list, max_value, precision):
    total_weight = sum(list)
    normed_buckets = []
    for item in list:
        temp = float(item)/total_weight
        # temp = round_places(temp, precision)
        normed_buckets.append(temp)

    for x in range(1, len(normed_buckets)):
        normed_buckets[x] += normed_buckets[x-1]
    normed_buckets = [x * max_value for x in normed_buckets]

    final_buckets = []
    for n in normed_buckets:
        final_buckets.append(round_places(n, precision))
    final_buckets[-1] = max_value
    return final_buckets


def generate_distribution(num_buckets):
    curr_bucket = 0
    buckets = []
    while curr_bucket < num_buckets:
        weight = int(random.random() * 100)
        buckets.append(weight)
        curr_bucket += 1

    total_weight = sum(buckets)
    normed_buckets = []
    for i in range(0, len(buckets) - 1):
        temp = float(buckets[i]) / total_weight
        normed_buckets.append(temp)
    normed_buckets.append(1 - sum(normed_buckets))

    return normed_buckets

def round_places(unrounded, places=3):
    temp = unrounded
    multiplier = float(10**places)
    temp *= multiplier
    temp = int(temp)
    temp /=(multiplier)
    return temp

class DemographicsParameters:
    NODES_KEY = "Nodes"
    NODEATTRIBUTES_KEY = "NodeAttributes"
    INITIALPOPULATION_KEY = "InitialPopulation"
    AGEDISTRIBUTION_KEY = "AgeDistribution"
    DISTRIBUTIONVALUES_KEY = "DistributionValues"
    RESULTVALUES_KEY = "ResultValues"
    METADATA_KEY = "Metadata"
    INDIVIDUALATTRIBUTES_KEY = "IndividualAttributes"
    AGEDISTRIBUTIONFLAG_KEY = "AgeDistributionFlag"
    AGEDISTRIBUTION1_KEY = "AgeDistribution1"
    AGEDISTRIBUTION2_KEY = "AgeDistribution2"
    NUMDISTRIBUTIONAXES_KEY = "NumDistributionAxes"
    RESULTSCALEFACTOR_KEY = "ResultScaleFactor"
    RESULTUNITS_KEY = "ResultUnits"

class ConfigParameters:
    AGEDISTRIBUTIONTYPE_KEY = "Age_Initialization_Distribution_Type"
    SIMPLEDISTRO = "DISTRIBUTION_SIMPLE"
    COMPLEXDISTRO = "DISTRIBUTION_COMPLEX"
    DISTROOFF = "DISTRIBUTION_OFF"
    ENABLEBUILTINDEMO_KEY = "Enable_Demographics_Builtin"
    DEFAULTDEMO_NODEPOP_KEY = "Default_Geography_Initial_Node_Population"
    DEFAULTDEMO_TORUSSIZE_KEY = "Default_Geography_Torus_Size"

class Constants:
    DAYS_IN_YEAR = 365
    FLAG_UNIFORM = 1
    FLAG_GAUSSIAN = 2
    FLAG_EXPONENTIAL = 3
    FLAG_BIMODAL = 6

def get_json_template(json_filename="demographics_template.json"):
    with open(json_filename) as infile:
        j_file_obj = json.load(infile)
    return j_file_obj

# region demographics editing
def get_demographics_nodes( demographics=None):
    if not demographics:
        demographics = get_json_template()
    nodes = demographics[DemographicsParameters.NODES_KEY]
    return nodes

def change_node_ind_attribute(new_value, node=None, att_key=DemographicsParameters.INITIALPOPULATION_KEY):
    if not node:
        node = get_demographics_nodes()[0]
    temp_node = node
    temp_node[DemographicsParameters.NODEATTRIBUTES_KEY][att_key] = new_value
    return temp_node

def set_demographics_file(demographics, demo_filename="demographics.json"):
    with open(demo_filename, 'w') as outfile:
        json.dump(demographics, outfile, indent=4, sort_keys=True)
# endregion

# region config editing
def set_config_file(config, config_filename="nd.json"):
    with open(config_filename, 'w') as outfile:
        json.dump(config, outfile, indent=4, sort_keys=True)
# endregion

# region create complex distro
def create_complex_agedistro_node(portions=[0.1, 0.2, 0.0, 0.4, 0.3], ages=[0, 5, 10, 20, 40]):
    '''
    :param portions: array of fractions that add up to one [0.1, 0.2, 0.0, 0.4, 0.3]
    :param ages: array of ascending ages in years, same length as portions [0, 5, 10, 20, 40]
    :return: json object representing node
    '''
    temp_node = {}
    temp_node[DemographicsParameters.NUMDISTRIBUTIONAXES_KEY] = 0
    temp_node[DemographicsParameters.RESULTSCALEFACTOR_KEY] = Constants.DAYS_IN_YEAR
    temp_node[DemographicsParameters.RESULTUNITS_KEY] = 'years'

    distribution_values = []
    distribution = 0
    for p in portions:
        distribution += p
        distribution_values.append(distribution)

    temp_node[DemographicsParameters.DISTRIBUTIONVALUES_KEY] = distribution_values

    temp_node[DemographicsParameters.RESULTVALUES_KEY] = ages
    return temp_node

def create_standard_complex_distro_node():
    standard_portions = [0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.0]
    standard_ages =     [0.0, 1.01, 9.0,  15,  20,  25,  30,  35,  40,  45,  50, 55]
    temp_node = create_complex_agedistro_node(portions=standard_portions, ages=standard_ages)
    return temp_node

def configure_complex_age_initialization( distro, initial_population = 10000):
    demographics = get_json_template()
    single_node = change_node_ind_attribute(initial_population,
                                                 att_key=DemographicsParameters.INITIALPOPULATION_KEY)
    single_node[DemographicsParameters.INDIVIDUALATTRIBUTES_KEY][DemographicsParameters.AGEDISTRIBUTION_KEY] = distro
    demographics[DemographicsParameters.NODES_KEY] = [single_node]
    set_demographics_file(demographics)

    config = get_json_template(json_filename="nd_template.json")
    config[ConfigParameters.AGEDISTRIBUTIONTYPE_KEY] = ConfigParameters.COMPLEXDISTRO
    config[ConfigParameters.ENABLEBUILTINDEMO_KEY] = 0
    set_config_file(config)
# endregion

# region create simple node
def configure_simple_age_initialization(initial_population = 10000, flag = 1, AgeDistribution1 = 0, AgeDistribution2 = 100):
    demographics = get_json_template()
    single_node = change_node_ind_attribute(initial_population,
                                                 att_key=DemographicsParameters.INITIALPOPULATION_KEY)

    # individual_attributes_template = Template(r'{"AgeDistributionFlag": $flag,"AgeDistribution1": $AgeDistribution1,"AgeDistribution2": $AgeDistribution2}')
    # individual_attributes = individual_attributes_template.substitute(flag = flag, AgeDistribution1 = AgeDistribution1, AgeDistribution2 = AgeDistribution2)
    single_node[DemographicsParameters.INDIVIDUALATTRIBUTES_KEY][DemographicsParameters.AGEDISTRIBUTIONFLAG_KEY] = flag
    single_node[DemographicsParameters.INDIVIDUALATTRIBUTES_KEY][DemographicsParameters.AGEDISTRIBUTION1_KEY] = AgeDistribution1
    single_node[DemographicsParameters.INDIVIDUALATTRIBUTES_KEY][DemographicsParameters.AGEDISTRIBUTION2_KEY] = AgeDistribution2
    demographics[DemographicsParameters.NODES_KEY] = [single_node]
    set_demographics_file(demographics)

    config = get_json_template(json_filename="nd_template.json")
    config[ConfigParameters.AGEDISTRIBUTIONTYPE_KEY] = ConfigParameters.SIMPLEDISTRO
    config[ConfigParameters.ENABLEBUILTINDEMO_KEY] = 0
    set_config_file(config)
# endregion
