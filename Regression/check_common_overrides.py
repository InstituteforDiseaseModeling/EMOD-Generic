import regression_utils as ru
from itertools import groupby

def main(test_suite_file_name):
    list_configjson_file = ru.load_json(test_suite_file_name)
    list_configjson_path = list_configjson_file['science']
    config_tup_list = []
    for configjson_path in list_configjson_path:
        configjson_flat = {}
        #print( "configjson_path = " + configjson_path )
        path = configjson_path['path'] + '/param_overrides.json'
        configjson = ru.load_json(path)
        ru.recursive_json_overrider( configjson, configjson_flat )
        # specifically for TBHIV
        if 'tbhiv_science.json' == test_suite_file_name:
            if len(configjson_flat['TBHIV_Drug_Params']) != 0:
                for k in configjson_flat['TBHIV_Drug_Params'].keys():
                    config_tbhiv_drug_params_tup = list(map(lambda i: ((k+'_'+i[0], i[1]), 1),
                                                            configjson_flat['TBHIV_Drug_Params'][k].items()))
                    config_tup_list.extend(config_tbhiv_drug_params_tup)
                del configjson_flat['TBHIV_Drug_Params']
        config_tup = list(map(lambda i: ((i[0], i[1]), 1), configjson_flat.items()))
        config_tup_list.extend(config_tup)
    config_tup_list.sort(key=lambda i: i[0])
    config_tup_reduced = [(key, sum([i[1] for i in values])) for key, values in groupby(config_tup_list, lambda i: i[0])]
    config_tup_reduced = sorted(config_tup_reduced, key = lambda i: -i[1])
    with open("Common_Overrides.txt", "w") as f:
        for item in config_tup_reduced:
            f.write(str(item)+'\n')
    # print(config_tup_reduced)


if __name__ == "__main__":
    # run
    test_suite_file_name = 'environmental_science.json'
    # test_suite_file_name = 'generic.json'
    # test_suite_file_name = 'tbhiv_science.json'
    # test_suite_file_name = 'typhoid_science.json'
    main(test_suite_file_name)
