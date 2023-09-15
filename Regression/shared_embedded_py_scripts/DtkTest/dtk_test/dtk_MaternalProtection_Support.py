from __future__ import division    # for division to be zero, required in threshold comparison in create_report_file()
import numpy as np
from scipy.stats import norm, kstest
import json
import dtk_test.dtk_sft as dtk_sft
import pandas as pd
import os

KEY_TOTAL_TIMESTEPS = "Simulation_Duration"
KEY_SIMULATION_TIMESTEP = "Simulation_Timestep"
KEY_BASE_INFECTIVITY_CONSTANT = "Base_Infectivity_Constant"
KEY_CONFIG_NAME = "Config_Name"
KEY_RUN_NUMBER = "Run_Number"
KEY_MATERNAL_ACQUIRE_CONFIG = "Maternal_Acquire_Config"
KEY_BOX_DURATION_GAUSSIAN_MEAN = "Box_Duration_Gaussian_Mean"
KEY_BOX_DURATION_GAUSSIAN_STD_DEV = "Box_Duration_Gaussian_Std_Dev"

# column header for individual susceptibility
KEY_INDIVIDUAL_ID = "id"
KEY_INDIVIDUAL_AGE = "age"
KEY_INDIVIDUAL_MOD_ACQUIRE = "mod_acquire"

"""
A waning effect with a box duration distributed from a Gaussian reproduces the previous 'sigmoid binary'
implementation of maternal immunity. Agents individually have complete immunity until that immunity ends
as a step-function.
"""


# region post_process support
def load_emod_parameters(config_filename="config.json"):
    """
    reads config file and populates params_obj
    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with KEY_TOTAL_TIMESTEPS, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    param_obj[KEY_TOTAL_TIMESTEPS] = cdj[KEY_TOTAL_TIMESTEPS]
    param_obj[KEY_BASE_INFECTIVITY_CONSTANT] = cdj[KEY_BASE_INFECTIVITY_CONSTANT]
    param_obj[KEY_SIMULATION_TIMESTEP] = cdj[KEY_SIMULATION_TIMESTEP]
    param_obj[KEY_CONFIG_NAME] = cdj[KEY_CONFIG_NAME]
    param_obj[KEY_RUN_NUMBER] = cdj[KEY_RUN_NUMBER]
    param_obj[KEY_BOX_DURATION_GAUSSIAN_MEAN] = cdj[KEY_MATERNAL_ACQUIRE_CONFIG][KEY_BOX_DURATION_GAUSSIAN_MEAN]
    param_obj[KEY_BOX_DURATION_GAUSSIAN_STD_DEV] = cdj[KEY_MATERNAL_ACQUIRE_CONFIG][KEY_BOX_DURATION_GAUSSIAN_STD_DEV]
    with open("DEBUG_param_obj.json", "w") as outfile:
        json.dump(param_obj, outfile, indent=4, sort_keys=True)
    return param_obj


def parse_output_file(output_filename="test.txt", debug=False):
    """
    creates a dataframe of time step and infected,  infectiouness and stat populations
    :param output_filename: file to parse (test.txt)
    :param debug: if True then print debug_info and write output_df to disk as './individual_susceptibility.csv'
    :return: output_df:  # of infected population and infectiousness per person at each time step
    """
    filtered_lines = []
    with open(output_filename) as logfile:
        for line in logfile:
            # search for "Update(): time" | Susceptibility update
            if dtk_sft.has_match(line, ["Update(): Time: ", KEY_INDIVIDUAL_MOD_ACQUIRE]):
                filtered_lines.append(line)
    if debug:
        with open("DEBUG_filtered_lines.txt", "w") as outfile:
            outfile.writelines(filtered_lines)

    # initialize variables
    time_step = 0
    output_df = pd.DataFrame(columns=[KEY_SIMULATION_TIMESTEP, KEY_INDIVIDUAL_ID, KEY_INDIVIDUAL_AGE, KEY_INDIVIDUAL_MOD_ACQUIRE])
    output_df.index.name = "index"
    with open("DEBUG_individual_susceptibility.txt", "w") as suscept_file:
        for i in range(0, len(filtered_lines)):
            if "Update(): Time:" in filtered_lines[i]:
                if debug:
                    print("working on... " + filtered_lines[i])
                    print("time_step = " + str(time_step))
                time_step += 1
                break
            # DEVNOTE: we just validate time_step 1 and all individuals
            else:
                a = parse_name_value_pair(filtered_lines[i])
                suscept_file.write("individual susceptibility row is id: {0}, age: {1}, mod_acquire: {2}.\n".
                format(a[KEY_INDIVIDUAL_ID], a[KEY_INDIVIDUAL_AGE], a[KEY_INDIVIDUAL_MOD_ACQUIRE]))
                if i > 0 and output_df.loc[i-1].id == int(a[KEY_INDIVIDUAL_ID]):
                    continue
                output_df.loc[i] = [int(time_step), int(a[KEY_INDIVIDUAL_ID]), float(a[KEY_INDIVIDUAL_AGE]), float(a[KEY_INDIVIDUAL_MOD_ACQUIRE])]
        # drop duplicates as DTK will call getModAcquire() multiple times per individual per timestep
        output_df = output_df.drop_duplicates(subset=[KEY_SIMULATION_TIMESTEP, KEY_INDIVIDUAL_ID])

        if debug:
            res_path = r'./DEBUG_individual_susceptibility.csv'
            if not os.path.exists(os.path.dirname(res_path)):
                os.makedirs(os.path.dirname(res_path))
            output_df.to_csv(res_path)
    return output_df


def parse_name_value_pair(text):
    """
    parse comma separated name value pair and return a dictionary
    :param text: line of text to parse
    # e.g. text = '00:00:00 [0] [D] [Individual] id = 1, age = 594.748291, mod_acquire = 1.000000'
    :returns: d: dictionary with (name=value)
    """
    del_chars = ','
    scrunched_text = text.replace(del_chars, '')
    text_list = scrunched_text.split()
    keys = [key for key in text_list if key.isalpha() or '_' in key]
    values = [value for value in text_list if '.' in value or value.isnumeric()]
    d = dict(zip(keys, values))
    return d


def create_report_file(param_obj, output_df, report_name):
    """
    Do the actual validation as follows and create the actual reports:
    1) validating each individual's mod_acquire value using the function calculate_initial_mod_acquire(),
    2) examine the set of immune failage acquire values collectively - binary type only. These values are assigned according
        to a random process, so it's possible to identify threshold values from that process
    :param param_obj: parameter object(read from config.json)
    :param output_df: output dataframe from output file parsing (stdout/test.txt)
    :param report_name: report file name to write to disk
    :param debug:
    :return: success
    """

    simulation_duration = int(param_obj[KEY_TOTAL_TIMESTEPS])
    simulation_timestep = float(param_obj[KEY_SIMULATION_TIMESTEP])
    base_infectivity = float(param_obj[KEY_BASE_INFECTIVITY_CONSTANT])
    gaussian_mu = float(param_obj[KEY_BOX_DURATION_GAUSSIAN_MEAN])
    gaussian_std = float(param_obj[KEY_BOX_DURATION_GAUSSIAN_STD_DEV])

    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj["Config_Name"]) + ", Run number: " + str(param_obj["Run_Number"])
                      + "\n# Compares the threshold immunity values with the actual values.\n")
        outfile.write("Simulation parameters: simulation_duration={0}, simulation_timestep={1}, base_infectivity={2}\n".
                      format(simulation_duration, simulation_timestep, base_infectivity))

        success = True

        # Lists of mod_acquire and age
        mod_acq = list()
        ages    = list()
        for index, row in output_df.iterrows():
            current_individual = row
            ages.append(current_individual[KEY_INDIVIDUAL_AGE])
            mod_acq.append(current_individual[KEY_INDIVIDUAL_MOD_ACQUIRE])

        # Histograms of ages and ages weighted by mod_acquire (mod_acquire is 0 or 1)
        age_bins = np.arange(0,401,20)
        bin_cent = np.diff(age_bins)/2 + age_bins[:-1]
        (age_pyr,_)  = np.histogram(ages, bins=age_bins)
        (imm_num,_)  = np.histogram(ages, bins=age_bins, weights=mod_acq)

        # Difference of histograms is cdf for susceptibility
        sigmoid_calc = imm_num/age_pyr
        sigmoid_ref  = norm.cdf(bin_cent, loc=gaussian_mu, scale=gaussian_std)

        # Random draws based on simulated cdf
        sim_data = np.interp(np.random.random(size=10000), sigmoid_calc, bin_cent)

        # Test random draws using sim data against reference distribution
        ks_result = kstest(sim_data, cdf='norm', args=(gaussian_mu, gaussian_std))

        if ks_result.pvalue > 0.05:
            success = False
            outfile.write("BAD: ks-test pvalue: "+str(ks_result.pvalue) + " expected < 0.05\n")

        if success:
            outfile.write("GOOD: actual mod_acquire for all individuals was within 0.0001 of the expected.\n")

        title = "Individual_Mod_Acquire"
        dtk_sft.plot_data(sigmoid_ref, sigmoid_calc, label1="Expected", label2="Actual",
                          title=title, ylabel="Fraction Susceptible", category=title, show=True)

        outfile.write(dtk_sft.format_success_msg(success))

    return success

