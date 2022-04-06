#!usr/bin/python

import json
import os.path as path


class ConfigKeys:
    TOTAL_TIMESTEPS = "Simulation_Duration"
    SIMULATION_TIMESTEP = "Simulation_Timestep"
    CONFIG_NAME = "Config_Name"
    RUN_NUMBER = "Run_Number"
    IMMUNITY_ACQUISITION_FACTOR = "Post_Infection_Acquisition_Multiplier"
    CAMPAIGN_NAME = "Campaign_Filename"


class CampaignKeys:
    Start_Day = "Start_Day"
    Infection_Type = "Infection_Type"
    Coverage = "Demographic_Coverage"
    Intervention_Config = "Intervention_Config"
    Event_Coordinator_Config = "Event_Coordinator_Config"
    Prime_Effect = "Prime_Effect"
    Boost_Effect = "Boost_Effect"
    Boost_Threshold = "Boost_Threshold"
    Events = "Events"
    Number_Repetitions = "Number_Repetitions"
    Timesteps_Between_Repetitions = "Timesteps_Between_Repetitions"
    Initial = "Initial"


class Thresholds:
    HIGH, LOW = [0.6, 0.0]


class Effects:
    PRIME, BOOST, INITIAL = [0.25, 0.45, 0.55]


class Vaccine:
    VACCINE, BOOSTER_HIGH, BOOSTER_LOW = [-1, Thresholds.HIGH, Thresholds.LOW]


def apply_vaccine_only(cur_immunity):
    effect = cur_immunity + (1.0 - cur_immunity) * Effects.INITIAL
    return effect


def apply_booster_only(cur_immunity, threshold):
    if cur_immunity > threshold:
        effect = cur_immunity + (1.0 - cur_immunity) * Effects.BOOST
    else:
        effect = cur_immunity + (1.0 - cur_immunity) * Effects.PRIME
    return effect


def apply_vaccines(vaccines):
    effect = 0
    for vaccine in vaccines:
        if vaccine == Vaccine.VACCINE:
            effect = apply_vaccine_only(effect)
        else:
            effect = apply_booster_only(effect, vaccine)
    return effect


def load_emod_parameters(config_filename="config.json"):
    """
    reads config file and populates params_obj
    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with KEY_TOTAL_TIMESTEPS, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    param_obj[ConfigKeys.TOTAL_TIMESTEPS] = cdj[ConfigKeys.TOTAL_TIMESTEPS]
    param_obj[ConfigKeys.IMMUNITY_ACQUISITION_FACTOR] = cdj[ConfigKeys.IMMUNITY_ACQUISITION_FACTOR]
    param_obj[ConfigKeys.CONFIG_NAME] = cdj[ConfigKeys.CONFIG_NAME]
    param_obj[ConfigKeys.RUN_NUMBER] = cdj[ConfigKeys.RUN_NUMBER]
    param_obj[ConfigKeys.CAMPAIGN_NAME] = cdj[ConfigKeys.CAMPAIGN_NAME]
    return param_obj


def parse_json_report(key_new_infections_group, key_statistical_population_group, key_disease_deaths_group,
                      output_folder="output", propertyreport_name="PropertyReport.json", debug=False):
    """
    creates report_data_obj structure with keys
    :param propertyreport_name: PropertyReport.json file with location (output/PropertyReport.json)
    :param key_new_infections_group : defined in post process file
    :returns: report_data_obj structure, dictionary with KEY_NEW_INFECTIONS etc., keys (e.g.)
    """
    propertyreport_path = path.join(output_folder, propertyreport_name)
    with open(propertyreport_path) as infile:
        icj = json.load(infile)["Channels"]
    report_data_obj = {}
    for key in key_new_infections_group:
        new_infections = icj[key]["Data"]
        report_data_obj[key] = new_infections
    for key in key_statistical_population_group:
        statistical_population = icj[key]["Data"]
        report_data_obj[key] = statistical_population
    if key_disease_deaths_group is not None:
        for key in key_disease_deaths_group:
            disease_death = icj[key]["Data"]
            report_data_obj[key] = disease_death
    if debug:
        with open("DEBUG_data_PropertyReport.json", "w") as outfile:
            json.dump(report_data_obj, outfile, indent=4, sort_keys=True)

    return report_data_obj


def parse_vax_event(campaign_filename, prime_group_event):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    vax_event_obj = {}
    vax_event_obj[CampaignKeys.Prime_Effect] = cf[prime_group_event][CampaignKeys.
        Event_Coordinator_Config][CampaignKeys.Intervention_Config][CampaignKeys.Prime_Effect]
    vax_event_obj[CampaignKeys.Boost_Effect] = cf[prime_group_event][CampaignKeys.
        Event_Coordinator_Config][CampaignKeys.Intervention_Config][CampaignKeys.Boost_Effect]
    vax_event_obj[CampaignKeys.Boost_Threshold] = cf[prime_group_event][CampaignKeys.
        Event_Coordinator_Config][CampaignKeys.Intervention_Config][CampaignKeys.Boost_Threshold]

    return vax_event_obj


def parse_outbreak_event(campaign_filename, outbreak_group_event):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    outbreak_event_obj = {}
    outbreak_event_obj[CampaignKeys.Start_Day] = cf[outbreak_group_event][CampaignKeys.Start_Day]
    outbreak_event_obj[CampaignKeys.Number_Repetitions] = cf[outbreak_group_event][CampaignKeys.
        Event_Coordinator_Config].get(CampaignKeys.Number_Repetitions, 1)
    outbreak_event_obj[CampaignKeys.Timesteps_Between_Repetitions] = cf[outbreak_group_event][CampaignKeys.
        Event_Coordinator_Config].get(CampaignKeys.Timesteps_Between_Repetitions, -1)

    return outbreak_event_obj
