from __future__ import division    # for division to be zero, required in threshold comparison in create_report_file()
import numpy as np
import json
import dtk_test.dtk_sft as dtk_sft
import pandas as pd
import os

KEY_TOTAL_TIMESTEPS = "Simulation_Duration"
KEY_SIMULATION_TIMESTEP = "Simulation_Timestep"
KEY_BASE_INFECTIVITY_CONSTANT = "Base_Infectivity_Constant"
KEY_ENABLE_IMMUNITY = "Enable_Immunity"
KEY_SUSCEPTIBILITY_TYPE = "Susceptibility_Type"  # ("FRACTIONAL", "BINARY")
KEY_MATERNAL_PROTECTION_TYPE = "Maternal_Protection_Type"  # ("LINEAR, "SIGMOID")
KEY_CONFIG_NAME = "Config_Name"
KEY_RUN_NUMBER = "Run_Number"

# following is for LINEAR type
KEY_MATERNAL_LINEAR_SLOPE = "Maternal_Linear_Slope"
KEY_MATERNAL_LINEAR_SUSZERO = "Maternal_Linear_SusZero"

# following is for SIGMOID type
KEY_MATERNAL_SIGMOID_STEEPFAC = "Maternal_Sigmoid_SteepFac"
KEY_MATERNAL_SIGMOID_HALFMAXAGE = "Maternal_Sigmoid_HalfMaxAge"
KEY_MATERNAL_SIGMOID_SUSINIT = "Maternal_Sigmoid_SusInit"

# column header for individual susceptibility
KEY_INDIVIDUAL_ID = "id"
KEY_INDIVIDUAL_AGE = "age"
KEY_INDIVIDUAL_MOD_ACQUIRE = "mod_acquire"
KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE = "m_immune_failage_acquire"


class Constants:
    DAYS_IN_YEAR = 365


"""
Enable maternal protection in the simulation configuration using the "Enable_Maternal_Protection" Boolean.
If enabled, the desired form of maternal protection is specified using the "Maternal_Protection_Type" enumeration.
Additional input parameters depend on the selected enumeration.
Maternal_Protection_Type == LINEAR
      Expected Parameters: {Maternal_Linear_Slope, Maternal_Linear_SusZero}
Maternal_Protection_Type == SIGMOID
      Expected Parameters: {Maternal_Sigmoid_SteepFac, Maternal_Sigmoid_HalfMaxAge, Maternal_Sigmoid_SusInit}
*** Appendix ***
Both "FRACTIONAL" implementations assign an equal susceptibility to all agents. This value is a function of age only:
Linear:      Susceptibility = Slope * Age + SusZero
Sigmoid:     Susceptibility = SusInit + (1.0 - SusInit) / (1.0 + EXP( (HalfMaxAge - Age) / SteepFac) )
Both "BINARY" implementations assign an age cutoff to each agent. If the agent age is less than the cutoff age, then the
 agent has susceptibility reduced to zero.
If the agent age is greater than or equal to the cutoff age, the agent does not have reduced susceptibility. The age 
assigned to each agent is randomly assigned:
Linear:      AgeCutoff = (RAND - SusZero) / Slope
Sigmoid:     AgeCutoff = HalfMaxAge - SteepFac * LOG( (1.0 - SusInit) / (RAND - SusInit) - 1.0 + 0.001)
Negative age cutoffs are not physically relevant, but are also not numerically problematic. The small positive value 
(0.001) in Sigmoid_Binary serves to prevent LOG(ZERO).
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
    param_obj[KEY_ENABLE_IMMUNITY] = cdj[KEY_ENABLE_IMMUNITY]
    param_obj[KEY_SUSCEPTIBILITY_TYPE] = cdj[KEY_SUSCEPTIBILITY_TYPE]
    param_obj[KEY_MATERNAL_PROTECTION_TYPE] = cdj[KEY_MATERNAL_PROTECTION_TYPE]
    param_obj[KEY_CONFIG_NAME] = cdj[KEY_CONFIG_NAME]
    param_obj[KEY_RUN_NUMBER] = cdj[KEY_RUN_NUMBER]
    if param_obj[KEY_MATERNAL_PROTECTION_TYPE] == "LINEAR":
        param_obj[KEY_MATERNAL_LINEAR_SLOPE] = cdj[KEY_MATERNAL_LINEAR_SLOPE]
        param_obj[KEY_MATERNAL_LINEAR_SUSZERO] = cdj[KEY_MATERNAL_LINEAR_SUSZERO]
    if param_obj[KEY_MATERNAL_PROTECTION_TYPE] == "SIGMOID":
        param_obj[KEY_MATERNAL_SIGMOID_HALFMAXAGE] = cdj[KEY_MATERNAL_SIGMOID_HALFMAXAGE]
        param_obj[KEY_MATERNAL_SIGMOID_STEEPFAC] = cdj[KEY_MATERNAL_SIGMOID_STEEPFAC]
        param_obj[KEY_MATERNAL_SIGMOID_SUSINIT] = cdj[KEY_MATERNAL_SIGMOID_SUSINIT]
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
    output_df = pd.DataFrame(columns=[KEY_SIMULATION_TIMESTEP, KEY_INDIVIDUAL_ID, KEY_INDIVIDUAL_AGE,
                                      KEY_INDIVIDUAL_MOD_ACQUIRE, KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE])
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
                suscept_file.write("individual susceptibility row is id: {0}, age: {1}, mod_acquire: {2}, failage_acquire: "
                                   "{3}.\n".format(a[KEY_INDIVIDUAL_ID], a[KEY_INDIVIDUAL_AGE], a[KEY_INDIVIDUAL_MOD_ACQUIRE],
                                                   a[KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE]))
                if i > 0 and output_df.loc[i-1].id == int(a[KEY_INDIVIDUAL_ID]):
                    continue
                output_df.loc[i] = [int(time_step), int(a[KEY_INDIVIDUAL_ID]), float(a[KEY_INDIVIDUAL_AGE]),
                                    float(a[KEY_INDIVIDUAL_MOD_ACQUIRE]), float(a[KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE])]
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
    # e.g. text = '00:00:00 [0] [D] [Individual] id = 1, age = 594.748291, mod_acquire = 1.000000,
    m_immune_failage_acquire = 307.381203'
    :returns: d: dictionary with (name=value)
    """
    del_chars = ','
    scrunched_text = text.replace(del_chars, '')
    text_list = scrunched_text.split()
    keys = [key for key in text_list if key.isalpha() or '_' in key]
    values = [value for value in text_list if '.' in value or value.isnumeric()]
    d = dict(zip(keys, values))
    return d


def calculate_initial_mod_acquire(age, susceptibility_type, maternal_protection_type, maternal_linear_slope,
                                  maternal_linear_suszero,
                                  maternal_sigmoid_steepfac, maternal_sigmoid_halfmaxage, maternal_sigmoid_susinit,
                                  immune_failage_acquire, imm_mod=1, debug=False):
    """
    calculate mod_acquire
    :param age:
    :param susceptibility_type
    :param imm_mod:
    :param maternal_protection_type:
    :param maternal_linear_slope:
    :param maternal_linear_suszero:
    :param maternal_sigmoid_steepfac:
    :param maternal_sigmoid_halfmaxage:
    :param maternal_sigmoid_susinit:
    :param immune_failage_acquire:
    :param debug:
    :return: mod_acquire
    """
    mod_acquire = imm_mod
    susceptibility = 1.0

    if susceptibility_type == "FRACTIONAL":
        if maternal_protection_type == "LINEAR":
            susceptibility = maternal_linear_slope * age + maternal_linear_suszero
        elif maternal_protection_type == "SIGMOID":
            susceptibility = maternal_sigmoid_susinit + (1.0 - maternal_sigmoid_susinit) / \
                             (1 + np.exp((maternal_sigmoid_halfmaxage - age) / maternal_sigmoid_steepfac))
        else:
            if debug:
                print("invalid maternal_protection_type:{0}".format(maternal_protection_type))
        susceptibility = susceptibility if susceptibility < 1.0 else 1.0
    elif susceptibility_type == "BINARY":
        if age < immune_failage_acquire:
            susceptibility = 0.0
        else:
            susceptibility = 1.0

    mod_acquire *= susceptibility

    if debug:
        print("calculated mod_acquire is {0} .\n".format(mod_acquire))
    return mod_acquire


def reference_failage_thresholds(susceptibility_type,
                                 maternal_protection_type,
                                 maternal_linear_slope,
                                 maternal_linear_suszero,
                                 maternal_sigmoid_steepfac,
                                 maternal_sigmoid_halfmaxage,
                                 maternal_sigmoid_susinit,
                                 debug=False):
    """
    Specify a small number of values on the interval (0.0, 1.0), and calculate a immune_failage_acquire associated with
    each of those values. Given a threshold_set and threshold_vals (from the attached script) the fraction of
    immune_failage_acquire values from the simulation output that are less than an entry in threshold_vals should equal the
    corresponding value in threshold_set.
    Linear:      AgeCutoff = (RAND - SusZero) / Slope
    Sigmoid:     AgeCutoff = HalfMaxAge - SteepFac * LOG( (1.0 - SusInit) / (RAND - SusInit) - 1.0 + 0.001)
    :param susceptibility_type,
    :param maternal_protection_type:
    :param maternal_linear_slope:
    :param maternal_linear_suszero:
    :param maternal_sigmoid_steepfac:
    :param maternal_sigmoid_halfmaxage:
    :return: random number generated:
    """
    threshold_set = np.array([0.2, 0.4, 0.6, 0.8])

    if susceptibility_type == "BINARY":
        if maternal_protection_type == "LINEAR":
            threshold_vals = (threshold_set - maternal_linear_suszero) / maternal_linear_slope
        elif maternal_protection_type == "SIGMOID":
            # Validate threshold_set
            # If a corresponding threshold value is <= 0, SIGMOID may not validate correctly
            min_threshold_set = maternal_sigmoid_susinit + (1.0 - maternal_sigmoid_susinit) \
                                / (1.0 + np.exp(maternal_sigmoid_halfmaxage / maternal_sigmoid_steepfac))
            threshold_set = np.array([ts for ts in threshold_set if ts > min_threshold_set])
            threshold_vals = maternal_sigmoid_halfmaxage - maternal_sigmoid_steepfac * \
                             np.log((1.0 - maternal_sigmoid_susinit) /
                                    (threshold_set - maternal_sigmoid_susinit) - 1.0 + 0.001)
        else:
            if debug:
                print("invalid maternal_protection_type:{0}".format(maternal_protection_type))

    return threshold_set, threshold_vals


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
    enable_immunity = param_obj[KEY_ENABLE_IMMUNITY]
    base_infectivity = float(param_obj[KEY_BASE_INFECTIVITY_CONSTANT])
    susceptibility_type = param_obj[KEY_SUSCEPTIBILITY_TYPE]
    maternal_protection_type = param_obj[KEY_MATERNAL_PROTECTION_TYPE]
    linear_slope = None
    linear_susZero = None
    sigmoid_halfMaxAge = None
    sigmoid_steepFac = None
    sigmoid_susinit = None

    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj["Config_Name"]) + ", Run number: " + str(param_obj["Run_Number"])
                      + "\n# Compares the threshold immunity values with the actual values and validates each "
                        "individual's mod_acquire value.\n")
        outfile.write("Simulation parameters: simulation_duration={0}, simulation_timestep={1}, enable_immunity={2}, "
                      "base_infectivity={3}\n".
                      format(simulation_duration, simulation_timestep, enable_immunity, base_infectivity))
        outfile.write("maternal_protection_type = {0}:\n".format(maternal_protection_type))
        outfile.write("susceptibility_type = {0}:\n".format(susceptibility_type))

        if param_obj[KEY_MATERNAL_PROTECTION_TYPE] == "LINEAR":
            linear_slope = float(param_obj[KEY_MATERNAL_LINEAR_SLOPE])
            linear_susZero = float(param_obj[KEY_MATERNAL_LINEAR_SUSZERO])
            outfile.write("It's a linear type with linear_slope:{0} and linear_susZero:{1}\n"
                          .format(linear_slope, linear_susZero))
        elif param_obj[KEY_MATERNAL_PROTECTION_TYPE] == "SIGMOID":
            sigmoid_halfMaxAge = param_obj[KEY_MATERNAL_SIGMOID_HALFMAXAGE]
            sigmoid_steepFac = param_obj[KEY_MATERNAL_SIGMOID_STEEPFAC]
            sigmoid_susinit = param_obj[KEY_MATERNAL_SIGMOID_SUSINIT]
            outfile.write("It's a sigmoid type with half_max_age:{0}, steepFac:{1}, susInit:{2}\n".
                          format(sigmoid_halfMaxAge, sigmoid_steepFac, sigmoid_susinit))

        success = True
        expected_mod_acquire_all = []
        individual_mod_acquire_all = []
        ages = []
        # mod_acquire check
        for index, row in output_df.iterrows():
            current_individual = row
            ages.append(current_individual[KEY_INDIVIDUAL_AGE])
            expected_mod_acquire = calculate_initial_mod_acquire(current_individual[KEY_INDIVIDUAL_AGE],
                                                                 susceptibility_type, maternal_protection_type,
                                                                 linear_slope, linear_susZero, sigmoid_steepFac,
                                                                 sigmoid_halfMaxAge, sigmoid_susinit,
                                                                 current_individual[KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE])
            expected_mod_acquire_all.append(expected_mod_acquire)
            individual_mod_acquire_all.append(current_individual[KEY_INDIVIDUAL_MOD_ACQUIRE])
            if not np.isclose(expected_mod_acquire, current_individual[KEY_INDIVIDUAL_MOD_ACQUIRE], 0.0001):
                outfile.write("BAD: actual mod_acquire for individual {0} at time step {1} is {2}, expected {3}.\n".
                              format(current_individual[KEY_INDIVIDUAL_ID], current_individual[KEY_SIMULATION_TIMESTEP],
                                     current_individual[KEY_INDIVIDUAL_MOD_ACQUIRE], expected_mod_acquire))
                success = False
        if success:
            outfile.write("GOOD: actual mod_acquire for all individuals was within 0.0001 of the expected.\n")
        title = "Individual_Mod_Acquire"
        dtk_sft.plot_data(expected_mod_acquire_all, individual_mod_acquire_all, label1="Expected_Mod_Acquire",
                          label2="Actual_Mod_Acquire", title=title, xlabel="Individual",
                          ylabel="Mod_Acquire", category=title, show=True)
        # failage threshold check, only operate on BINARY enumerations
        if susceptibility_type == "BINARY":
            threshold_set, threshold_vals = reference_failage_thresholds(susceptibility_type, maternal_protection_type,
                                                                         linear_slope, linear_susZero,
                                                                         sigmoid_steepFac, sigmoid_halfMaxAge,
                                                                         sigmoid_susinit, False)
            # Ensure each individual is in the DataFrame only once.
            if len(output_df) == len(output_df[KEY_INDIVIDUAL_ID].unique()):
                # Extract vector of immune_failage_acquire values
                mfvals = output_df[KEY_INDIVIDUAL_IMMUNE_FAILAGE_ACQUIRE].values
                plot_list_mfvals = []
                # Test threshold values
                for ts, tv in zip(threshold_set, threshold_vals):
                    # The quality of the match depends on the number of individuals;
                    # for ~10k individuals, a 2% match is a good level
                    if not np.isclose((mfvals <= tv).sum() / mfvals.size, ts, atol=0.02):
                        outfile.write("BAD: threshold value check:\n")
                        outfile.write("threshold_set:" + str(ts) + "\n")
                        outfile.write("threshold_vals:" + str(tv) + "\n")
                        outfile.write("mfvals: " + str(mfvals) + "\n")
                        success = False
                    plot_list_mfvals.append((mfvals <= tv).sum() / mfvals.size)

                title = "Expected_vs_Actual_Immune_Failage_Acquire"
                dtk_sft.plot_data(threshold_set, plot_list_mfvals,
                                  label1="Expected_Immune_Failage_Acquire",
                                  label2="Actual_Immune_Failage_Acquire",
                                  title=title, xlabel="Individual",
                                  ylabel="Immune_Failage_Acquire", category=title, show=True)
                if success:
                    outfile.write("GOOD: Match quality was within 2% difference, which is good for ~10k individuals.\n")
        outfile.write(dtk_sft.format_success_msg(success))
    return success
# endregion
