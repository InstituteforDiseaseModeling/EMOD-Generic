# import extract_overrides as eo #NOTE: import the script and it runs.
import json
import sys
from copy import deepcopy


def read_config_json(filename):
    with open(filename) as config_file:
        config_json = json.load(config_file)
    return config_json


def recursive_node_purger( default_reference_json, specific_flat_json, override_config_json, test_suite_file_name ):
    for node in default_reference_json:
        if json.dumps( default_reference_json[ node ] ).startswith( "{" ):
            # for tbhiv param_overrides only
            if 'tbhiv_science.json' == test_suite_file_name:
                if node in specific_flat_json:
                    recursive_node_purger(default_reference_json[node], specific_flat_json[node],
                                          override_config_json[node], test_suite_file_name)
                    if len( override_config_json[ node ] ) == 0:
                        del override_config_json[ node ]
            else:
                recursive_node_purger(default_reference_json[node], specific_flat_json, override_config_json[node],
                                      test_suite_file_name)
                if len(override_config_json[node]) == 0:
                    del override_config_json[node]
        else:
            # for tbhiv param_overrides only
            if 'tbhiv_science.json' == test_suite_file_name:
                # leaf node, compare values
                if node not in specific_flat_json or (node in specific_flat_json and default_reference_json[node] == specific_flat_json[node]):
                    del override_config_json[ node ]
                else:
                    override_config_json[ node ] = specific_flat_json[node]
            else:
                # leaf node, compare values
                if node not in specific_flat_json["parameters"] or node in specific_flat_json["parameters"] and \
                        default_reference_json[node] == specific_flat_json["parameters"][node]:
                    del override_config_json[node]
                else:
                    override_config_json[node] = specific_flat_json["parameters"][node]


def extract_list_of_overrides(file_config_list):
    specific_config_json_list = []
    config_path_list = file_config_list["science"]
    for config_path in config_path_list:
        specific_config_json = config_path["path"]
        specific_config_json_list.append(specific_config_json)
    return specific_config_json_list


def main(test_suite_file_name):
    # file_config_list = json.loads(open(sys.argv[1]).read())
    file_config_list = None
    with open(test_suite_file_name) as suitefile:
        file_config_list = json.load(suitefile)
    specific_config_json_list = extract_list_of_overrides(file_config_list)
    default_config_filename = ''
    if 'typhoid_science.json' == test_suite_file_name:
        default_config_filename = "defaults/typhoid_science_default_config.json"
    elif 'tbhiv_science.json' == test_suite_file_name:
        default_config_filename = "defaults/tbhiv_science_default_config.json"
    elif 'environmental_science.json' == test_suite_file_name:
        default_config_filename = "defaults/environmental_science_default_config.json"
    elif 'generic.json' == test_suite_file_name:
        default_config_filename = "defaults/generic_science_default_config.json"
    default_reference_json = read_config_json(default_config_filename)
    for specific_config in specific_config_json_list:
        specific_config_filename = specific_config + "/config.json"
        override_config_filename = specific_config + "/param_overrides.json"
        print(specific_config_filename)
        specific_config_json = read_config_json(specific_config_filename)
        override_config_json = read_config_json(default_config_filename)
        # check if any parameter is nested
        if any(isinstance(i,dict) for i in specific_config_json['parameters'].values()):
            print('There is nested parameters in this config file')
        recursive_node_purger(default_reference_json, specific_config_json, override_config_json, test_suite_file_name)
        # # for tbhiv param_overrides only
        if 'tbhiv_science.json' == test_suite_file_name:
            list_TBHIV_drug_types = override_config_json['parameters']['TBHIV_Drug_Types']
            list_TBHIV_drug_params = list(override_config_json['parameters']['TBHIV_Drug_Params'].keys())
            for drug_params in list_TBHIV_drug_params:
                if drug_params not in list_TBHIV_drug_types:
                    del override_config_json['parameters']['TBHIV_Drug_Params'][drug_params]
        override_config_json["Default_Config_Path"] = default_config_filename
        # TODO: write this object to disk. NOTE: you'll need to know the filename you want
        with open(override_config_filename, 'w') as outfile:
            json.dump(override_config_json, outfile, indent=4)
            outfile.write('\n')


if __name__ == "__main__":
    # run
    # test_suite_file_name = 'environmental_science.json'
    # test_suite_file_name = 'generic.json'
    # test_suite_file_name = 'tbhiv_science.json'
    test_suite_file_name = 'typhoid_science.json'
    main(test_suite_file_name)
