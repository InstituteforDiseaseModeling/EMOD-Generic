#!usr/bin/python
from __future__ import division
import json
import dtk_test.dtk_sft as sft
import pandas as pd
import os.path as path


KEY_NEW_INFECTIONS = "New Infections"
KEY_STATISTICAL_POPULATION = "Statistical Population"


class CampaignKeys:
    VACCINE_TAKE = "Vaccine_Take"
    DEMOGRAPHIC_COVERAGE = "Demographic_Coverage"
    INTERVENTION_CONFIG = "Intervention_Config"
    START_DAY = "Start_Day"
    EVENT_COORDINATOR_CONFIG = "Event_Coordinator_Config"
    NUMBER_REPETITIONS = "Number_Repetitions"
    TIMESTEPS_BETWEEN_REPETITIONS = "Timesteps_Between_Repetitions"
    ACQUIRE_CONFIG = "Acquire_Config"
    INITIAL_EFFECT = "Initial_Effect"


class ConfigKeys:
    CAMPAIGN_FILENAME = "Campaign_Filename"
    CONFIG_NAME = "Config_Name"
    RUN_NUMBER = "Run_Number"
    SIMULATION_TIMESTEP = "Simulation_Timestep"
    TOTAL_TIMESTEPS = "Simulation_Duration"
    START_TIME = "Start_Time"


"""
MultiEffectVaccine allows a vaccine to have Acquisition, Transmission, and/or Mortality blocking effects.  In this test,
we are only testing Acquisition (Mortality and Transmission parameters in the config file aren't affecting test
behavior).  We expect the MultiEffectVaccine's efficacy to be the InitialEffect multiplied by the VaccineTake, and the
number of new infections to reflect this efficacy rate, and test this.  
From this, that means there are individuals whose VaccineTake does not match whether or not they were uninfected if 
InitialEffect is not 1, so we test if this number of individuals is equal to the expected number.
"""


def load_emod_parameters(config_filename, debug=False):
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    param_obj[ConfigKeys.CONFIG_NAME] = cdj[ConfigKeys.CONFIG_NAME]
    param_obj[ConfigKeys.RUN_NUMBER] = cdj[ConfigKeys.RUN_NUMBER]
    param_obj[ConfigKeys.SIMULATION_TIMESTEP] = cdj[ConfigKeys.SIMULATION_TIMESTEP]
    param_obj[ConfigKeys.TOTAL_TIMESTEPS] = cdj[ConfigKeys.TOTAL_TIMESTEPS]
    param_obj[ConfigKeys.START_TIME] = cdj[ConfigKeys.START_TIME]
    param_obj[ConfigKeys.CAMPAIGN_FILENAME] = cdj[ConfigKeys.CAMPAIGN_FILENAME]
    if debug:
        with open("DEBUG_param_obj.json","w") as outfile:
            json.dump(param_obj, outfile, indent=4)
    return param_obj


def parse_data(filtered_lines, debug=False):
    if len(filtered_lines) == 0:
        raise ValueError("There's no people!!")
    uninfected_dict = {}
    columns = ['take', 'id', 'uninfected']
    outputdf = pd.DataFrame(columns=columns)
    take_list = []
    identity_list = []
    for line in filtered_lines:
        if "Vaccine did " in line:
            identity = int(float(sft.get_val("for individual ", line)))
            uninfected_dict[identity] = 0  # initializing everyone to infected by default
            identity_list.append(identity)
            if "not" not in line:
                take_list.append(1)
            else:
                take_list.append(0)
        elif "We didn't infect individual" in line:
            identity_2 = int(sft.get_val("We didn't infect individual ", line))
            uninfected_dict[identity_2] = 1
    outputdf['take'] = take_list
    outputdf['id'] = identity_list
    outputdf = outputdf.sort_values(by='id')
    uninfected_list = []
    for key in sorted(uninfected_dict.keys()):
        uninfected_list.append(uninfected_dict[key])
    outputdf['uninfected'] = uninfected_list
    if debug:
        print(type(uninfected_dict))
        print(uninfected_dict)
        with open("InfectedAndTakeDataframe.csv", "w") as outfile:
            outputdf.to_csv(outfile)
    return outputdf


def load_campaign_file(campaign_filename, debug=False):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    campaign_obj = {}
    campaign_obj[CampaignKeys.VACCINE_TAKE] = cf[0][CampaignKeys.EVENT_COORDINATOR_CONFIG][CampaignKeys.
        INTERVENTION_CONFIG][CampaignKeys.VACCINE_TAKE]
    campaign_obj[CampaignKeys.START_DAY] = [cf[0][CampaignKeys.START_DAY]]
    campaign_obj[CampaignKeys.INITIAL_EFFECT] = cf[0][CampaignKeys.EVENT_COORDINATOR_CONFIG][CampaignKeys.
        INTERVENTION_CONFIG][CampaignKeys.ACQUIRE_CONFIG][CampaignKeys.INITIAL_EFFECT]
    if debug:
        with open("DEBUG_campaign_obj.json", "w") as outfile:
            json.dump(campaign_obj, outfile, indent=4)
    return campaign_obj


def parse_output_file(stdout_filename, debug=False):
    filtered_lines = []
    with open(stdout_filename) as infile:
        for line in infile:
            if "Vaccine did " in line or "We didn't infect individual" in line:
                filtered_lines.append(line)
    if debug:
        with open("DEBUG_filtered_lines.txt", "w") as debug_file:
            debug_file.writelines(filtered_lines)
    return filtered_lines


def create_report_file(param_obj, campaign_obj, report_name, stdout_filename,
                       debug=False, report_data_obj=None, outbreak_event_obj=None):
    success = True
    filtered_lines = parse_output_file(stdout_filename, debug)
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + param_obj[ConfigKeys.CONFIG_NAME] + ", Run number: " +
                      str(param_obj[ConfigKeys.RUN_NUMBER]) + "\n# Tests the expected vaccine take versus"
                                                              " the actual vaccine take for acquisition with infection"
                                                              " data from the InsetChart.\n")
        outputdf = parse_data(filtered_lines)
        if report_data_obj is not None and outbreak_event_obj is not None: # testing particular take rate
            success = False \
                if not test_infection_rate(report_data_obj, outbreak_event_obj, campaign_obj, outfile) else success
        if report_data_obj is not None:
            success = False \
                if not test_take_vs_infected(report_data_obj, outbreak_event_obj, campaign_obj,
                                             outputdf, outfile, debug) else success
        outfile.write(sft.format_success_msg(success))
    if debug:
        print("SUMMARY: Success={0}\n".format(success))
    return success


def test_infection_rate(report_data_obj, outbreak_event_obj, campaign_obj, outfile):
    success = True
    outbreak_start = outbreak_event_obj[CampaignKeys.START_DAY]
    stat_pop = report_data_obj[KEY_STATISTICAL_POPULATION][outbreak_start]
    new_infect = report_data_obj[KEY_NEW_INFECTIONS][outbreak_start]
    outfile.write(f"NEUTRAL: The number of new infections is {new_infect}.\n")
    take_rate = campaign_obj[CampaignKeys.VACCINE_TAKE] * campaign_obj[CampaignKeys.INITIAL_EFFECT]
    theory_new_infect = (1 - take_rate) * stat_pop
    sft.plot_data(theory_new_infect, new_infect, label1="theory_new_infect", label2="actual_new_infect",
                  title="theoretical vs. actual new infections", category="take_infect_plot", ymin=0,
                  ymax=stat_pop)
    # now doing a binomial test
    if not sft.test_binomial_95ci(new_infect, stat_pop, 1-take_rate, outfile, "new infections test"):
        success = False
    else:
        outfile.write("GOOD: 95% confidence interval binomial test for number of infections at 1-take*initial_effect "
                      "probability passes.\n")
    return success


def test_take_vs_infected(report_data_obj, outbreak_event_obj, campaign_obj, outputdf, outfile, debug=False):
    # now to test if the take rate is equal to the uninfected rate.  if take = 1, uninfected = 1, and if
    # take = 0, uninfected should = 0.
    success = True
    outbreak_start = outbreak_event_obj[CampaignKeys.START_DAY]
    stat_pop = report_data_obj[KEY_STATISTICAL_POPULATION][outbreak_start]
    if report_data_obj is not None:
        fail_count = 0
        initial_ineffectiveness = 1 - campaign_obj[CampaignKeys.INITIAL_EFFECT]
        ineffective_pop_prob = initial_ineffectiveness * campaign_obj[CampaignKeys.VACCINE_TAKE]
        pop_w_ineffective_vaccine = stat_pop * ineffective_pop_prob
        for index, row in outputdf.iterrows():
            if row['take'].astype(float) != row['uninfected'].astype(float):
                fail_count += 1
        if fail_count > 0:  # sometimes we expect take to fail because the initial effect is less than 1.
            # time for another binomial test here looking at when take does not match infection status
            if sft.test_binomial_95ci(fail_count, stat_pop, ineffective_pop_prob, outfile,
                                      "ineffective vaccine test"):
                outfile.write("GOOD: 95% confidence interval binomial test for number of people whose vaccine was"
                              " ineffective because of initial effectiveness rate passes.\n")
            else:
                outfile.write(f"BAD: The number of individuals whose vaccine take did not match their infection"
                              f" status was"
                              f" {fail_count} and should have been {pop_w_ineffective_vaccine}."
                              f" Binomial test on this failed.\n")
                success = False
        else:
            if initial_ineffectiveness is 0.0:
                outfile.write(f"GOOD: All individuals' infection status matched their vaccine take and the vaccine"
                              f" has a 100% initial effect.\n")
            else:
                outfile.write(f"BAD: Initial ineffectiveness is {initial_ineffectiveness} and should have been 0.0."
                              f"  Something wonky is going on with your initial effect.\n")
                success = False
        if debug:
            print(f"Number of individuals infected was {stat_pop - sum(outputdf['uninfected'])}.\n")
    return success


def parse_json_report(output_folder="output", insetchart_name="InsetChart.json", debug=False):
    """creates report_data_obj structure with keys

    :param insetchart_name: InsetChart.json file with location (output/InsetChart.json)
    :returns: report_data_obj structure, dictionary with KEY_NEW_INFECTIONS etc., keys (e.g.)
    """
    insetchart_path = path.join(output_folder, insetchart_name)
    with open(insetchart_path) as infile:
        icj = json.load(infile)["Channels"]

    report_data_obj = {}

    new_infections = icj[KEY_NEW_INFECTIONS]["Data"]
    report_data_obj[KEY_NEW_INFECTIONS] = new_infections
    statistical_population = icj[KEY_STATISTICAL_POPULATION]["Data"]
    report_data_obj[KEY_STATISTICAL_POPULATION] = statistical_population

    if debug:
        with open("DEBUG_data_InsetChart.json", "w") as outfile:
            json.dump(report_data_obj, outfile, indent=4)

    return report_data_obj


def parse_outbreak_event(campaign_filename, outbreak_group_event, debug=False):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    outbreak_event_obj = {}
    outbreak_event_obj[CampaignKeys.START_DAY] = cf[outbreak_group_event][CampaignKeys.START_DAY]
    outbreak_event_obj[CampaignKeys.NUMBER_REPETITIONS] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.NUMBER_REPETITIONS, 1)
    outbreak_event_obj[CampaignKeys.TIMESTEPS_BETWEEN_REPETITIONS] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.TIMESTEPS_BETWEEN_REPETITIONS, -1)
    outbreak_event_obj[CampaignKeys.DEMOGRAPHIC_COVERAGE] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.DEMOGRAPHIC_COVERAGE, 1)

    if debug:
        print("outbreak_event_obj: " + str(outbreak_event_obj) + "\n")

    return outbreak_event_obj

