import json

path_config = "config_create_sp.json"

with open(path_config, "r") as file:
    po = json.load(file)
    del po['parameters']['Serialized_Population_Filenames']
    del po['parameters']['Enable_Random_Generator_From_Serialized_Population']
    po['parameters']['Serialization_Time_Steps'] = [15]
    po['parameters']['Serialization_Type'] = 'TIMESTEP'

with open(path_config, "w") as file:
    json.dump(po, file, indent=4)