#!/usr/bin/python

import json
import math
import os
from enum import Enum
import pandas           as pd
import numpy            as np
import scipy.special    as sps
import dtk_test.dtk_sft as dtk_sft

"""
Basic calculations
Each node contains a total amount of infectivity that is equal to the sum of the infectivities of each individual 
present in that node. An individual's infectivity is the product of that individual's infectiousness and sampling 
weight. An individual's infectiousness is based on the disease dynamics as described in that section: infectivity = 
SUM(individual_infectiousness * individual_sample_weight)

The probability of an individual in a node receiving an infection (absent structured transmission HINT as described in 
that section) is determined by an infection rate that is equal to the total infectivity in that node divided by the 
total population in that node: infection_rate = infectivity / stat_pop

An individual gets a new infection during a timestep dt with probability 1-exp(-infection_rate*dt). That probability 
is decreased by immunity: actual_probability =  1 - exp(- infection_rate * dt * mod_acquire)

NOTE: AN INDIVIDUAL'S PROBABILITY OF INFECTION IS INDEPENDENT OF THAT INDIVIDUAL'S SAMPLING WEIGHT

The number of unweighted (individual_sample_weight = 1) immunologically naïve (mod_acquire = 1) individuals infected 
during a timestep should be distributed as a binomial draw with probability 1-exp(-infection_rate*dt).

The expression for probability of infection derives from the Poisson distribution. The Poisson rate parameter for number 
of infections occurring in a node during a timestep dt is infectivity*dt, with exp(-infection_rate*dt) equal to the 
probability of a single individual receiving zero infections, and 1-exp(-infection rate*dt) equal to the probability of 
a single individual receiving one or more infections.

This infection process is exactly equivalent to randomly selecting (with replacement) a number of indivduals to receive 
infections, where the number of individuals selected during a timestep is distributed as a Poisson draw with rate 
parameter equal to infectivity*dt.
"""


class Config:
    config_name = "Config_Name"
    simulation_timestep = "Simulation_Timestep"
    duration = "Simulation_Duration"
    infectivity_distribution = "Base_Infectivity_Distribution"
    exponential_mean = "Base_Infectivity_Exponential"
    gaussian_mu = "Base_Infectivity_Gaussian_Mean"
    gaussian_sig = "Base_Infectivity_Gaussian_Std_Dev"
    uniform_max = "Base_Infectivity_Max"
    uniform_min = "Base_Infectivity_Min"
    constant_mean = "Base_Infectivity_Constant"
    infectivity_mean = "Infectivity_Mean"
    sample_rate = "Base_Individual_Sample_Rate"
    run_number = "Run_Number"


class Distribution(Enum):
    CONSTANT = "CONSTANT_DISTRIBUTION"
    EXPONENTIAL = "EXPONENTIAL_DISTRIBUTION"
    GAUSSIAN = "GAUSSIAN_DISTRIBUTION"
    UNIFORM = "UNIFORM_DISTRIBUTION"


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


matches = ["Update(): Time: ",
           "total contagion = "
           ]

# provided by Kurt
def gaussNonNegMean(mu=0.0, sig=1.0):
    """
    Calculates the mean of a truncated normal distribution with left limit at 0 and right limit at INF.

    In the DTK, for the Gaussian distribution, parameters mean and std_dev are poorly named; those values
    correspond to the mu and sigma parameters, and are not actually mean and standard deviation

    'XXXX_Gaussian_Mean'    SHOULD be 'XXXX_Gaussian_Mu'
    'XXXX_Gaussian_Std_Dev' SHOULD be 'XXXX_Gaussian_Sigma'

    Example below:
        mu   = 1.0
        sig  = 0.1
        print('mu = {:5.3f}, sig = {:5.3f}, mean = {:5.3f}'.format(mu,sig,gaussNonNegMean(mu,sig)))

        output: mu = 1.000, sig = 0.100, mean = 1.000

        mu   = 0.0
        sig  = 1.0
        print('mu = {:5.3f}, sig = {:5.3f}, mean = {:5.3f}'.format(mu,sig,gaussNonNegMean(mu,sig)))
        output: mu = 0.000, sig = 1.000, mean = 0.798
    Args:
        mu:
        sig:

    Returns:

    """
    mos  = mu / sig
    Z    = 1 - 0.5 * (1 + sps.erf( -mos / np.sqrt(2)))
    mean = mu + np.exp(-0.5 * mos * mos) * sig / Z / np.sqrt(2 * np.pi)

    return mean


def load_emod_parameters(config_filename="config.json", debug=False):
    """reads config file and populates params_obj

    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with KEY_CONFIG_NAME, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    keys = [Config.config_name, Config.simulation_timestep, Config.duration, Config.infectivity_distribution, Config.run_number,
            Config.sample_rate]
    for key in keys:
        param_obj[key] = cdj[key]

    # get mean of infectivity distribution from config
    if param_obj[Config.infectivity_distribution] == Distribution.CONSTANT.value:
        param_obj[Config.infectivity_mean] = cdj[Config.constant_mean]
    elif param_obj[Config.infectivity_distribution] == Distribution.EXPONENTIAL.value:
        param_obj[Config.infectivity_mean] = cdj[Config.exponential_mean]
    elif param_obj[Config.infectivity_distribution] == Distribution.GAUSSIAN.value:
        param_obj[Config.infectivity_mean] = gaussNonNegMean(mu=cdj[Config.gaussian_mu], sig=cdj[Config.gaussian_sig])
    elif param_obj[Config.infectivity_distribution] == Distribution.UNIFORM.value:
        param_obj[Config.infectivity_mean] = (cdj[Config.uniform_max] + cdj[Config.uniform_min]) / 2.0
    else:
        param_obj[Config.infectivity_mean] = None

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
    creates insetchart_df dataframe
    :param insetchart_name: file to parse (InsetChart.json)
    :param output_folder:
    :return: insetchart_df: dataframe structure with "New Infections" etc., keys (e.g.)
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
        sample_rate = param_obj[Config.sample_rate]
        outfile.write("Config_name = {}\n".format(config_name))
        # write information into sft report file for parameter sweeping test in dtk_tools
        outfile.write("{0} = {1}\n {2} = {3}\n {4} = {5}\n {6} = {7}\n".format(
            Config.infectivity_distribution, param_obj[Config.infectivity_distribution],
            Config.infectivity_mean, param_obj[Config.infectivity_mean],
            Config.run_number, param_obj[Config.run_number],
            Config.sample_rate, sample_rate))
        success = True
        if not param_obj[Config.infectivity_mean]:
            success = False
            outfile.write(f"BAD: {Config.infectivity_distribution} must be one from the following distributions:\n"
                          f"{Distribution.CONSTANT.value}, {Distribution.UNIFORM.value}, "
                          f"{Distribution.GAUSSIAN.value} and {Distribution.EXPONENTIAL.value}, please fix the test.\n")
        else:
            contagion = stdout_df[Stdout.contagion]
            prob = stdout_df[Stdout.prob]
            stat_pop = stdout_df[Stdout.stat_pop]
            infected = stdout_df[Stdout.infected]
            calculated_contagion_list = []
            calculated_prob_list = []
            outfile.write("Part 1: Testing contagion and probability with calculated values for every time step:\n")
            for t in range(len(contagion)):
                calculated_contagion = param_obj[Config.infectivity_mean] * infected[t] / stat_pop[t]
                calculated_contagion_list.append(calculated_contagion)
                if math.fabs(calculated_contagion - contagion[t]) > 5e-2 * calculated_contagion:
                    success = False
                    outfile.write("    BAD: at time step {0}, the total contagion is {1}, expected {2}.\n".format(
                        t, contagion[t], calculated_contagion
                    ))
                calculated_prob = 1.0 - math.exp(-1 * calculated_contagion * param_obj[Config.simulation_timestep])
                calculated_prob_list.append(calculated_prob)
                if math.fabs(calculated_prob - prob[t]) > 5e-2 * calculated_prob:
                    success = False
                    outfile.write("    BAD: at time step {0}, the infected probability is {1}, expected {2}.\n".format(
                        t, prob[t], calculated_prob
                    ))
            outfile.write("Part 1: result is: {}.\n".format(success))
            dtk_sft.plot_data(contagion,calculated_contagion_list,
                              label1='contagion from logging', label2="calculated contagion",
                              title="actual vs. expected contagion",
                              xlabel='day',ylabel='contagion',category="contagion",
                              line=True, alpha=0.5, overlap=True)
            dtk_sft.plot_data(prob,calculated_prob_list,
                              label1='probability from logging', label2="calculated probability",
                              title="actual vs. expected probability",
                              xlabel='day',ylabel='probability',category="probability",
                              line=True, alpha=0.5, overlap=True)

            new_infections = [ new_infection_daily * sample_rate for
                               new_infection_daily in
                               insetchart_df[InsetChart.new_infections] ]
            outfile.write("Part 2: Testing new infections based on contagion and infection probability:\n")
            failed_timestep = []
            expected_new_infection_list = []
            start_day = campaign_obj[Campaign.start_day]
            test_stop_day = len(new_infections)
            for t in range(len(new_infections)):
                expected_new_infection = (stat_pop[t] - infected[t]) * calculated_prob_list[t] * sample_rate
                if t == start_day:
                    expected_new_infection += int(stat_pop[t] * #include new infection from outbreak
                                                  campaign_obj[Campaign.coverage] * sample_rate)
            # the following statistical tests are too stick, replaced with chi-squared test
            #     if t == campaign_obj[campaign.start_day] or expected_new_infection < 5 or
            #        (stat_pop[t] - infected[t]) * (1 - prob[t]) < 5:
            #         if math.fabs(expected_new_infection - new_infections[t]) > math.ceil(expected_new_infection * 5e-2):
            #             failed_timestep.append(t)
            #             outfile.write("BAD: at timestep {0}, new infection is {1},
            #                           expected {2}.\n".format(t, new_infections[t], expected_new_infection))
            #     elif not dtk_sft.test_binomial_95ci(new_infections[t], (stat_pop[t] - infected[t]), prob[t], outfile,
            #                                       'New Infections at timestep ' + str(t)):
            #         failed_timestep.append(t)
            #     expected_new_infection_list.append(expected_new_infection)
            # message = "binomial test for 95% confidence interval failed {0} times during {1} time steps totally." \
            #           "Expected less than {2} failures.\n".format(len(failed_timestep), len(new_infections),
            #                                                      math.ceil(len(new_infections) * 5e-2))
            # if len(failed_timestep) > math.ceil(len(new_infections) * 5e-2):
            #     success = False
            #     outfile.write("BAD: " + message)
            #     outfile.write("Part 2: Result is False.\n")
            # else:
            #     outfile.write(
            #         "GOOD: " + message)
            #     outfile.write("Part 2: Result is True.\n")

                expected_new_infection_list.append(expected_new_infection)
                if expected_new_infection == 0 and t > start_day:
                    if test_stop_day == len(new_infections):
                        test_stop_day = t
            if test_stop_day - start_day > 1:
                #chi_squared test
                #print(start_day, test_stop_day)
                if not dtk_sft.test_multinomial(list(new_infections[start_day: test_stop_day]),
                                                expected_new_infection_list[start_day: test_stop_day],
                                                outfile, prob_flag=False):
                    success = False
                    outfile.write("Part 2: Result is False.\n")
                else:
                    outfile.write("Part 2: Result is True.\n")
            else: # for infection rate larger than 1, all individual will get infected in one time step.
                if math.fabs(expected_new_infection_list[start_day] - new_infections[start_day]) <= \
                        expected_new_infection_list[start_day] * 0.01:
                    outfile.write("Part 2: Result is True.\n")
                else:
                    success = False
                    outfile.write("BAD: at time step {0}, expected new infection is {1}, got {2} from "
                                  "insetchart.json.\n".format(start_day, expected_new_infection_list[start_day],
                                                              new_infections[start_day]))
                    outfile.write("Part 2: Result is False.\n")

            #print(len(new_infections), len(expected_new_infection_list))
            dtk_sft.plot_data(new_infections, expected_new_infection_list, label1='new infections from insetchart',
                              label2="calculated new infections",
                              title="actual vs. expected new infections", xlabel='day', ylabel='new_infections',
                              category="new_infections",
                              line=True, alpha=0.5, overlap=True)

        outfile.write(dtk_sft.format_success_msg(success))
    if debug:
        print( "SUMMARY: Success={0}\n".format(success) )
    return success

