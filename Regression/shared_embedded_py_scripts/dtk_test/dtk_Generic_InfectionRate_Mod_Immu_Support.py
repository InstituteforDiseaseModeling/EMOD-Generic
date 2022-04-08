#!/usr/bin/python

import json
import dtk_test.dtk_sft as dtk_sft
import math
import pandas as pd
import os

"""
This SFT test the Infection Rate with mod_acquire, mod_transsmission and Base_Individual_Sample_Rate. The calculation
for Infection Rate is similar to InfectionRate_Basic_Calculation, except the the multiplication of mod_acquire and 
mod_transsmission.

An individual's infectiousness is based on the disease dynamics as described in that section: infectivity = 
SUM(individual_infectiousness * individual_sample_weight * mod_transsmission)

An individual gets a new infection during a timestep dt with probability 1-exp(-infection_rate*dt). That probability 
is decreased by immunity: actual_probability =  1 - exp(- infection_rate * dt * mod_acquire)
"""


class Config:
    config_name = "Config_Name"
    simulation_timestep = "Simulation_Timestep"
    duration = "Simulation_Duration"
    infectivity = "Base_Infectivity_Constant"
    sample_rate = "Base_Individual_Sample_Rate"
    run_number = "Run_Number"


class Stdout:
    prob = "prob="
    stat_pop = "StatPop: "
    infected = "Infected: "
    contagion = "Contagion"


class InsetChart:
    new_infections = "New Infections"
    stat_pop = "Statistical Population"


class Campaign:
    start_day = 'Start_Day'
    coverage = "Demographic_Coverage"
    initial_effect = "Initial_Effect"
    vaccine_type = "Vaccine_Type"
    acquisition_blocking = "AcquisitionBlocking"
    transmission_blocking = "TransmissionBlocking"


matches = ["Update(): Time: ",
           "total contagion = "
           ]


def load_emod_parameters(config_filename="config.json", debug=False):
    """reads config file and populates params_obj

    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with KEY_CONFIG_NAME, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    keys = [Config.config_name, Config.simulation_timestep, Config.duration, Config.infectivity, Config.run_number,
            Config.sample_rate]
    for key in keys:
        param_obj[key] = cdj[key]

    if debug:
        with open("DEBUG_param_object.json", 'w') as outfile:
            json.dump(param_obj, outfile, indent=4)
    return param_obj


def load_campaign_file(campaign_filename="campaign.json", debug=False):
    """reads campaign file

    :param campaign_filename: campaign.json file
    :returns: campaign_obj structure, dictionary with KEY_START_DAY, etc., keys (e.g.)
    """
    with open(campaign_filename) as infile:
        cf = json.load(infile)
    campaign_obj = {}
    start_day = cf["Events"][0][Campaign.start_day]
    campaign_obj[Campaign.start_day] = int(start_day)
    coverage = cf["Events"][0]["Event_Coordinator_Config"][Campaign.coverage]
    campaign_obj[Campaign.coverage] = float(coverage)
    campaign_obj[Campaign.vaccine_type] = []
    campaign_obj[Campaign.initial_effect] = []
    for event in cf["Events"][1:]:
        if("Acquire_Config" in event["Event_Coordinator_Config"]["Intervention_Config"]):
            vaccine_type = Campaign.acquisition_blocking
            init_effect  = event["Event_Coordinator_Config"]["Intervention_Config"]["Acquire_Config"][Campaign.initial_effect]
            campaign_obj[Campaign.vaccine_type].append(vaccine_type)
            campaign_obj[Campaign.initial_effect].append(init_effect)
        elif("Transmit_Config" in event["Event_Coordinator_Config"]["Intervention_Config"]):
            vaccine_type = Campaign.transmission_blocking
            init_effect  = event["Event_Coordinator_Config"]["Intervention_Config"]["Transmit_Config"][Campaign.initial_effect]
            campaign_obj[Campaign.vaccine_type].append(vaccine_type)
            campaign_obj[Campaign.initial_effect].append(init_effect)
        else:
            raise Exception('Unrecognized vaccine configuration: no Acquire_Config or Transmit_Config')

    if debug:
        with open("DEBUG_campaign_object.json", 'w') as outfile:
            json.dump(campaign_obj, outfile, indent=4)

    return campaign_obj


def parse_stdout_file(stdout_filename="StdOut.txt", simulation_timestep=1, debug=False):
    """
    creates a dataframe to store filtered information for each time step
    :param output_filename: file to parse (StdOut.txt)
    :return:                stdout_df
    """
    filtered_lines = []
    with open(stdout_filename) as logfile:
        for line in logfile:
            if dtk_sft.has_match(line,matches):
                filtered_lines.append(line)
    if debug:
        with open("DEBUG_filtered_lines.txt", "w") as outfile:
            outfile.writelines(filtered_lines)

    # initialize variables
    time_step = index = 0
    stdout_df = pd.DataFrame(columns=[Config.simulation_timestep, Stdout.stat_pop, Stdout.infected,
                                      Stdout.contagion, Stdout.prob])
    stdout_df.index.name = 'index'
    contagion = prob = 0
    for line in filtered_lines:
        if matches[0] in line:
            time_step += simulation_timestep
            stat_pop = int(dtk_sft.get_val(Stdout.stat_pop, line))
            infected = int(dtk_sft.get_val(Stdout.infected, line))
            stdout_df.loc[index] = [time_step, stat_pop, infected, contagion, prob]
            index += 1
        elif matches[1] in line:
            contagion = float(dtk_sft.get_val(matches[1], line))
            prob = float(dtk_sft.get_val(Stdout.prob, line))
    if debug:
        res_path = r'./DEBUG_filtered_from_logging.csv'
        stdout_df.to_csv(res_path)
    return stdout_df


def parse_insetchart_json(insetchart_name="InsetChart.json", output_folder="output", debug=False):
    """
    creates insetchart_df dataframe structure 
    :param insetchart_name: file to parse (InsetChart.json)
    :param output_folder:
    :return: insetchart_df: dataframe structure with KEY_NEW_INFECTIONS etc., keys (e.g.)
    """
    insetchart_path = os.path.join(output_folder, insetchart_name)
    with open(insetchart_path) as infile:
        icj = json.load(infile)["Channels"]

    insetchart_df = pd.DataFrame.from_dict({InsetChart.new_infections: icj[InsetChart.new_infections]['Data'],
                                            InsetChart.stat_pop:       icj[InsetChart.stat_pop]['Data']      })

    if debug:
        insetchart_df.to_csv("DEBUG_data_InsetChart.csv")

    return insetchart_df


def create_report_file(param_obj, campaign_obj, stdout_df, insetchart_df, report_name, debug):
    with open(report_name, "w") as outfile:
        config_name = param_obj[Config.config_name]
        outfile.write("Config_name = {}\n".format(config_name))
        # write information into sft report file for parameter sweeping test in dtk_tools
        sample_rate = param_obj[Config.sample_rate]
        vaccine_types = campaign_obj[Campaign.vaccine_type]
        mod_acquire = mod_transmission = 0
        for i in range(len(vaccine_types)):
            vaccine_type = vaccine_types[i]
            if vaccine_type == Campaign.acquisition_blocking:
                mod_acquire = campaign_obj[Campaign.initial_effect][i]
            elif vaccine_type == Campaign.transmission_blocking:
                mod_transmission = campaign_obj[Campaign.initial_effect][i]
            else:
                outfile.write("Warning: we have vaccine_type = {0} which is not {1} or {2}."
                              "".format(vaccine_type, Campaign.acquisition_blocking, Campaign.transmission_blocking))

        outfile.write("{0} = {1} {2} = {3} {4} = {5}\n".format(Config.infectivity, param_obj[Config.infectivity],
                                                               Config.run_number, param_obj[Config.run_number],
                                                               Config.sample_rate, sample_rate))
        outfile.write("{0} = {1} {2} = {3} {4} = {5}\n".format(Campaign.vaccine_type, vaccine_types,
                                                               Campaign.acquisition_blocking, mod_acquire,
                                                               Campaign.transmission_blocking, mod_transmission))
        success = True

        contagion = stdout_df[Stdout.contagion]
        prob = stdout_df[Stdout.prob]
        stat_pop = stdout_df[Stdout.stat_pop]
        infected = stdout_df[Stdout.infected]
        calculated_contagion_list = []
        calculated_prob_list = []
        outfile.write("Part 1: Testing contagion and probability with calculated values for every time step:\n")
        for t in range(len(contagion)):
            calculated_contagion = param_obj[Config.infectivity] * infected[t] / stat_pop[t] * (1.0 - mod_transmission)
            calculated_contagion_list.append(calculated_contagion)
            if math.fabs(calculated_contagion - contagion[t]) > 5e-2 * calculated_contagion:
                success = False
                outfile.write("    BAD: at time step {0}, the total contagion is {1}, expected {2}.\n".format(
                    t, contagion[t], calculated_contagion
                ))
            # calculated_prob = 1.0 - math.exp(-1 * calculated_contagion * param_obj[config.simulation_timestep])
            # calculated_prob *= (1.0 - mod_acquire)
            calculated_prob = 1.0 - math.exp(-1 * calculated_contagion * param_obj[Config.simulation_timestep] *(1.0 - mod_acquire))

            calculated_prob_list.append(calculated_prob)
            if math.fabs(calculated_prob - prob[t]) > 1e-2 * calculated_prob:
                success = False
                outfile.write("    BAD: at time step {0}, the infected probability is {1}, expected {2}.\n".format(
                    t, prob[t], calculated_prob
                ))
        outfile.write("Part 1: result is: {}.\n".format(success))
        dtk_sft.plot_data(contagion,calculated_contagion_list, label1='contagion from logging', label2="calculated contagion",
                          title="actual vs. expected contagion", xlabel='day',ylabel='contagion',category="contagion",
                          line=True, alpha=0.5, overlap=True)
        dtk_sft.plot_data(prob,calculated_prob_list, label1='probability from logging', label2="calculated probability",
                          title="actual vs. expected probability", xlabel='day',ylabel='probability',category="probability",
                          line=True, alpha=0.5, overlap=True)

        new_infections = [ new_infection_daily * sample_rate for new_infection_daily in insetchart_df[InsetChart.new_infections] ]
        outfile.write("Part 2: Testing new infections based on contagion and infection probability:\n")
        failed_timestep = []
        expected_new_infection_list = []
        start_day = campaign_obj[Campaign.start_day]
        test_stop_day = len(new_infections)
        for t in range(len(new_infections)):
            expected_new_infection = (stat_pop[t] - infected[t]) * calculated_prob_list[t] * sample_rate
            if t == start_day:
                expected_new_infection += int(stat_pop[t] * campaign_obj[Campaign.coverage] * sample_rate) #include new infection from outbreak

            expected_new_infection_list.append(expected_new_infection)
            if expected_new_infection == 0 and t > start_day:
                if test_stop_day == len(new_infections):
                    test_stop_day = t
        if test_stop_day - start_day > 1:
            #chi_squared test
            #print(start_day, test_stop_day)
            if not dtk_sft.test_multinomial(list(new_infections[start_day:test_stop_day]),
                                            expected_new_infection_list[start_day:test_stop_day], outfile, prob_flag=False):
                success = False
                outfile.write("Part 2: Result is False.\n")
            else:
                outfile.write("Part 2: Result is True.\n")
        else: # for infection rate larger than 1, all individual will get infected in one time step.
            if math.fabs(expected_new_infection_list[start_day] - new_infections[start_day]) <= expected_new_infection_list[start_day] * 0.01:
                outfile.write("Part 2: Result is True.\n")
            else:
                success = False
                outfile.write("BAD: at time step {0}, expected new infection is {1}, got {2} from insetchart.json.\n"
                              "".format(start_day, expected_new_infection_list[start_day], new_infections[start_day]))
                outfile.write("Part 2: Result is False.\n")

        #print(len(new_infections), len(expected_new_infection_list))
        dtk_sft.plot_data(new_infections, expected_new_infection_list, label1='new infections from insetchart',
                          label2="calculated new infections",
                          title="actual vs. expected new infections", xlabel='day', ylabel='new_infections',
                          category="new_infections",
                          line=True, alpha=0.5, overlap=True)

        outfile.write(dtk_sft.format_success_msg(success))
    if debug:
        print("SUMMARY: Success={0}\n".format(success))
    return success

