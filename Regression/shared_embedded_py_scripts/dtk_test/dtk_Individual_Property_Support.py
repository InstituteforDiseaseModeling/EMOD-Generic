import json
import os.path as path
import pandas as pd
import dtk_test.dtk_sft as sft


class CampaignKeys:
    START_DAY = "Start_Day"
    EVENTS_KEY = "Events"
    EVENT_COORDINATOR_CONFIG_KEY = "Event_Coordinator_Config"
    PROPERTY_RESTRICTIONS = "Property_Restrictions"


class ConfigKeys:
    PARAMETERS_KEY = "parameters"
    CAMPAIGN_FILENAME = "Campaign_Filename"
    ENABLE_INTERVENTIONS = "Enable_Interventions"
    CONFIG_NAME = "Config_Name"
    DEMOGRAPHICS_FILENAMES = "Demographics_Filenames"
    ENABLE_HINT = "Enable_Heterogeneous_Intranode_Transmission"
    ENABLE_PROPERTY_OUTPUT = "Enable_Property_Output"
    START_TIME = "Start_Time"
    TIMESTEP_SIZE = "Simulation_Timestep"


class DemographicsKeys:
    DEFAULTS_KEY = "Defaults"
    INDIVIDUAL_PROPERTIES_KEY = "IndividualProperties"

    class PropertyKeys:
        PROPERTY_NAME = "Property"
        PROPERTY_VALUES = "Values"
        INITIAL_DISTRIBUTION = "Initial_Distribution"
        TRANSITIONS_KEY = "Transitions"


class JsonReport:
    CHANNELS_KEY = "Channels"
    class Channels:
        DEATHS = "Disease Deaths"
        INFECTED = "Infected"
        NEW_INFECTIONS = "New Infections"
        STATISTICAL_POPULATION = "Statistical Population"
        CAMPAIGN_COST = "Campaign Cost"


def load_config_file(filename="config.json", debug=False):
    with open (filename) as infile:
        config_json = json.load(infile)[ConfigKeys.PARAMETERS_KEY]

    config_object = {}
    config_object[ConfigKeys.CONFIG_NAME] = config_json[ConfigKeys.CONFIG_NAME]
    config_object[ConfigKeys.DEMOGRAPHICS_FILENAMES] = config_json[ConfigKeys.DEMOGRAPHICS_FILENAMES]
    config_object[ConfigKeys.ENABLE_PROPERTY_OUTPUT] = config_json[ConfigKeys.ENABLE_PROPERTY_OUTPUT]
    config_object[ConfigKeys.ENABLE_HINT] = config_json[ConfigKeys.ENABLE_HINT]
    config_object[ConfigKeys.CAMPAIGN_FILENAME] = config_json[ConfigKeys.CAMPAIGN_FILENAME]
    config_object[ConfigKeys.START_TIME] = config_json[ConfigKeys.START_TIME]
    config_object[ConfigKeys.TIMESTEP_SIZE] = config_json[ConfigKeys.TIMESTEP_SIZE]
    config_object[ConfigKeys.ENABLE_INTERVENTIONS] = config_json[ConfigKeys.ENABLE_INTERVENTIONS]

    if debug:
        with open("DEBUG_config_object.json","w") as outfile:
            json.dump(config_object, outfile, indent=4)

    return config_object

def load_campaign_file(filename="campaign.json", debug=False):
    '''
    Loads up the campaign file as a dictionary of lists of events by day

    :param filename: Name of the campaign file (campaign.json)
    :param debug: Enabling debugging will drop the dictionary to list
    :return: campaign_dictionary
    '''

    with open(filename) as infile:
        camp_json = json.load(infile)

    camp_events = camp_json[CampaignKeys.EVENTS_KEY]
    camp_dict = {}
    for event in camp_events:
        start_day = event[CampaignKeys.START_DAY]
        if start_day not in camp_dict:
            camp_dict[start_day] = [event]
        else:
            camp_dict[start_day].append(event)
    return camp_dict


def channels_from_jsonreport(channels_list, report_folder="output",
                             json_report_name="InsetChart.json",
                             debug=False):
    json_report_location = path.join(report_folder, json_report_name)
    with open(json_report_location) as infile:
        channels_obj = json.load(infile)[JsonReport.CHANNELS_KEY]
    selection_dict = {}
    for c in channels_list:
        selection_dict[c] = channels_obj[c]['Data']

    if debug:
        clue = json_report_name.split('.')[0]
        with open("DEBUG_{0}_channels.json".format(clue),"w") as outfile:
            json.dump(selection_dict, outfile, indent=4)
    return selection_dict

def dataframe_from_channeldict(channel_dictionary, debug=False):
    timesteps_df = pd.DataFrame.from_dict(channel_dictionary)

    if debug:
        with open("DEBUG_channel_df.csv","w") as outfile:
            outfile.write(timesteps_df.to_csv())
    return timesteps_df

def dataframe_join_on_day(df1, df2, debug=False):
    combined_df = pd.concat([df1, df2], axis=1)

    if debug:
        with open("DEBUG_df_joined_on_day.csv","w") as outfile:
            outfile.write(combined_df.to_csv())
    return combined_df

def get_properties_from_demographics(filename="individualproperties_overlay.json", debug=False):
    """
    Loads a demographics file and gets all individual properties specified in the defaults section.
    :param filename: demographics filename to load.  (individualproperties_overlay.json)
    :param debug:
    :return: array of individual properties
    """
    overlay_json = None
    with open(filename) as infile:
        overlay_json = json.load(infile)

    returned_properties = []
    properties_array = overlay_json[DemographicsKeys.DEFAULTS_KEY][DemographicsKeys.INDIVIDUAL_PROPERTIES_KEY]
    for property in properties_array:
        property_object = {}
        property_object[DemographicsKeys.PropertyKeys.PROPERTY_NAME] = property[DemographicsKeys.PropertyKeys.PROPERTY_NAME]
        property_object[DemographicsKeys.PropertyKeys.PROPERTY_VALUES] = property[DemographicsKeys.PropertyKeys.PROPERTY_VALUES]
        property_object[DemographicsKeys.PropertyKeys.INITIAL_DISTRIBUTION] = property[DemographicsKeys.PropertyKeys.INITIAL_DISTRIBUTION]
        property_object[DemographicsKeys.PropertyKeys.TRANSITIONS_KEY] = property[DemographicsKeys.PropertyKeys.TRANSITIONS_KEY]
        returned_properties.append(property_object)

    if debug:
        with open("DEBUG_Properties_Array.json","w") as outfile:
            json.dump(properties_array, outfile, indent=4)
    return returned_properties

def get_property_channels(inset_channel_names, properties_array, debug=False):
    ps_lists = build_strings_for_all_properties(properties_array)
    prop_channels = []
    for ic in inset_channel_names:
        channel_matrix = build_channel_matrix_for_properties(ps_lists,
                                                             channel_name=ic)
        prop_channels += channel_matrix
    if debug:
        with open("DEBUG_property_channels.json","w") as outfile:
            json.dump(prop_channels, outfile, indent=4)
    return prop_channels

#region property strings
def build_strings_for_all_properties(properties_array, debug=False):
    propertystring_lists = []
    for property in properties_array:
        propertystring_lists.append(build_strings_for_property(property))
    if debug:
        with open("DEBUG_property_strings.json","w") as outfile:
            json.dump(propertystring_lists, outfile)
    return propertystring_lists

def build_strings_for_property(property, debug=False):
    property_strings = []
    property_name = property["Property"]
    for property_value in property["Values"]:
        property_strings.append("{0}:{1}".format(property_name,property_value))
    return property_strings

def build_channel_matrix_for_properties(propertystring_lists,
                                        channel_name="Statistical Population",
                                        debug = False):
    mix_list = []
    for pstring in propertystring_lists[0]:
        mix_list.append(pstring)

    for x in range(1, len(propertystring_lists)):
        tmp_list = []
        curr_list = propertystring_lists[x]
        for ps in curr_list:
            for ms in mix_list:
                tmp_list.append("{0},{1}".format(ms, ps))
        mix_list = tmp_list

    channel_matrix=[]
    property_channnel_template = "{0}:{1}"
    for ps in mix_list:
        prop_channnel = property_channnel_template.format(channel_name, ps)
        channel_matrix.append(prop_channnel)
    if debug:
        with open("DEBUG_{0}_matrix.json".format(channel_name),"w") as outfile:
            json.dump(channel_matrix, outfile)
    return channel_matrix

def build_channel_matrix_from_scratch(channel_base_names,
                                      demographics_filename,
                                      debug=False):
    property_array = get_properties_from_demographics(demographics_filename, debug=debug)
    propertystring_lists = build_strings_for_all_properties(properties_array=property_array, debug=debug)
    total_matrix = []
    for channel in channel_base_names:
        tmp_matrix = build_channel_matrix_for_properties(propertystring_lists,
                                                         channel_name=channel,
                                                         debug=debug)
        total_matrix = total_matrix + tmp_matrix
    return total_matrix

#endregion

def verify_channel_sum(channel_df, channel_root_name, messages_array=[]):
    '''
    Verifies that in the dataframe given, the value for the channel root name
    is equal to the sums of all of the channels that have that root name and
    contain that property name. For example, statistical population should equal
    statistical population for
    :param channel_df:
    :param channel_root_name:
    :param messages_array:
    :return:
    '''
    sum_column = "{0}:sum".format(channel_root_name)

    for c in channel_df.columns.values:
        if channel_root_name in c and c != channel_root_name and not "sum" in c:
            if sum_column in channel_df.columns.values:
                channel_df[sum_column] += channel_df[c]
            else:
                channel_df[sum_column] = channel_df[c]

    message_template = "{0}! Channel {1} should be equal to sum of all child channels"
    if channel_df[channel_root_name].equals(channel_df[sum_column]):
        messages_array.append(message_template.format("GOOD", channel_root_name))
        return True
    else:
        messages_array.append(message_template.format("BAD", channel_root_name))
        return False

def verify_channel_split(channel_df, channel_root_name, property, messages_array,
                         sft_output_file,
                         debug=False):
    '''
    Verifies that the channels are split according to the ratios for the property
    :param channel_df:
    :param channel_root_name:
    :param property:
    :param messages_array:
    :param sft_output_file:
    :return:
    '''

    # First, look for the property values to find
    value_map = {}
    property_name = property["Property"]
    property_values = property["Values"]
    property_distro = property["Initial_Distribution"]
    for v in property_values:
        property_string = "{0}:{1}".format(property_name, v)
        value_map[property_string] = []

    # Now find all of the channel names that contain the root and property string
    # and add them to the map

    shorter_channel_list = []
    for c in channel_df.columns.values:
        if channel_root_name in c and not "sum" in c:
            shorter_channel_list.append(c)
            for property_string in value_map.keys():
                if property_string in c: # Right channel, right value
                    value_map[property_string].append(c)

    local_sums = []
    total_pop = 0
    for v in value_map.keys():
        sum_name = "{0}:sum:{1}".format(channel_root_name, v)
        local_sums.append(sum_name)
        channel_df[sum_name] = channel_df[value_map[v][0]]
        total_pop += channel_df[value_map[v][0]][0]
        for x in range(1, len(value_map[v])):
            channel_df[sum_name] = channel_df[sum_name] + channel_df[value_map[v][x]]
            total_pop += channel_df[value_map[v][x]][0]

    # Let's plot this. NOTE: this will compare sum to first two values,
    # if there are three or more values, won't show all of them.
    sft.plot_data_3series(dist1=channel_df[channel_root_name],
                          dist2=channel_df[local_sums[0]],
                          dist3=channel_df[local_sums[1]],
                          label1=channel_root_name,
                          label2=local_sums[0],
                          label3=local_sums[1],
                          title="Property Split {0}".format(property_name), xlabel="timesteps",
                          ylabel="people", category="Property Split {0}".format(property_name),
                          show=True, sort=False)

    # now we have a channel like Statistical Population:sum:Risk:High
    # now see if the first timestep local sum splits are proportional to the property distro
    # print("Value map keys: {0} \n".format(value_map.keys()))
    success = True
    for x in range(0, len(value_map.keys())):
        it_worked = sft.test_binomial_95ci(channel_df[local_sums[x]][0], total_pop, property_distro[x],
                                           report_file=sft_output_file, category=channel_root_name)

        message_template = "{0}!: for channel {1}, found {2} with property value {3} out of total {4}" + \
                           " with a probability of {5}"
        message = None
        if it_worked:
            message= message_template.format("GOOD", channel_root_name, channel_df[local_sums[x]][0],
                                             property_values[x], total_pop, property_distro[x])
        else:
            success = False
            message= message_template.format("BAD", channel_root_name, channel_df[local_sums[x]][0],
                                             property_values[x], total_pop, property_distro[x])
        messages_array.append(message)

    if debug:
        with open("DEBUG_{0}_{1}_sums_df.csv".format(channel_root_name, property_name),"w") as outfile:
            outfile.write(channel_df.to_csv())
    return success, messages_array
