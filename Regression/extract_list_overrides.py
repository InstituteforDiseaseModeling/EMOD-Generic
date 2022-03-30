# import extract_overrides as eo #NOTE: import the script and it runs.
import json
import sys
from copy import deepcopy


def read_config_json(filename):
    with open(filename) as config_file:
        config_json = json.load(config_file)
    return config_json


def recursive_node_purger( default_reference_json, specific_flat_json, override_config_json ):
    for node in default_reference_json:
        if json.dumps( default_reference_json[ node ] ).startswith( "{" ):
            recursive_node_purger( default_reference_json[ node ], specific_flat_json, override_config_json[ node ] )
            #print( len( default_reference_json[ node ] ) )
            if len( override_config_json[ node ] ) == 0:
                del override_config_json[ node ]
        else:
            # leaf node, compare values
            if node not in specific_flat_json["parameters"] or node in specific_flat_json["parameters"] and default_reference_json[ node ] == specific_flat_json["parameters"][node]:
                del override_config_json[ node ]
            else:
                override_config_json[ node ] = specific_flat_json["parameters"][node]


def extract_list_of_overrides(file_config_list):
    specific_config_json_list = []
    config_path_list = file_config_list["science"]
    for config_path in config_path_list:
        specific_config_json = config_path["path"]
        specific_config_json_list.append(specific_config_json)
    return specific_config_json_list


def main():
    # file_config_list = json.loads(open(sys.argv[1]).read())
    file_config_list = None
    with open("generic_science.json") as suitefile:
        file_config_list = json.load(suitefile)
    specific_config_json_list = extract_list_of_overrides(file_config_list)
    default_config_filename = "defaults/generic_science_default_config.json"
    default_reference_json = read_config_json(default_config_filename)
    for specific_config in specific_config_json_list:
        specific_config_filename = specific_config + "/config.json"
        override_config_filename = specific_config + "/param_overrides.json"
        print(specific_config_filename)
        specific_config_json = read_config_json(specific_config_filename)
        override_config_json = read_config_json(default_config_filename)
        # override_config_json = deepcopy(default_reference_json)
        recursive_node_purger(default_reference_json, specific_config_json, override_config_json)
        override_config_json["Default_Config_Path"] = default_config_filename
        #TODO: write this object to disk. NOTE: you'll need to know the filename you want
        with open(override_config_filename, 'w') as outfile:
            json.dump(override_config_json, outfile, indent=4)


if __name__ == "__main__":
    # run
    main()
