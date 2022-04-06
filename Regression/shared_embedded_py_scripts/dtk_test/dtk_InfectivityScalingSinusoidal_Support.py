import json
import os.path as path
import dtk_test.dtk_sft as dtk_sft
import pandas as pd
import os
import math
import itertools as it

matches = ["Update(): Time: ",
           "Infected: ",
           "total contagion = ",
           "StatPop: "]

KEY_INFECTED = "infected"
KEY_INFECTIOUSNESS = "infectiousness"
KEY_NEW_INFECTIONS = "New Infections"
KEY_CUMULATIVE_INFECTIONS = "Cumulative Infections"
KEY_STAT_POP = "Stat Pop"


class ConfigKeys:
    TOTAL_TIMESTEPS = "Simulation_Duration"
    SIMULATION_TIMESTEP = "Simulation_Timestep"
    BASE_INFECTIVITY_CONSTANT = "Base_Infectivity_Constant"
    CONFIG_NAME = "Config_Name"
    RUN_NUMBER = "Run_Number"
    INFECTIVITY_SCALE_TYPE = "Infectivity_Scale_Type"

    class InfectivityScaling:
        AMPLITUDE = "Infectivity_Sinusoidal_Forcing_Amplitude"
        PHASE = "Infectivity_Sinusoidal_Forcing_Phase"


class Constants:
    DAYS_IN_YEAR = 365

"""
The purpose of this ENUM is to modulate the infectivity of the disease,
using a sine function with a period of 1 year.  The epidemiological relevance
is that many diseases are observed to have a high season and a low season,
and often it is not clear whether this is due to social behavior, climate,
or other factors that vary with an annual period.  In the literature, changing
disease infectivity from a constant to a sine function is a common way to handle this.
The implementation is intended to reproduce the following behavior:
beta(t)=beta_0 [1+A* sin((2* pi*(t-t_0))/365)]
Where beta_0= Base_Infectivity_Constant, A = Infectivity_Sinusoidal_Forcing_Amplitude,
and t0 = Infectivity_Sinusoidal_Forcing_Phase, and t is time in days, within the simulation.
A note - it would appear that time is assumed to cd .be in days, and it should be checked
whether changing the simulation timestep to multiple days or fractional days changes the
frequency of this sine wave, which should stay at one year. 
"""


# region post_process support
def load_emod_parameters(config_filename="config.json"):
    """
    reads config file and populates params_obj
    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with ConfigKeys.TOTAL_TIMESTEPS, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    if not cdj[ConfigKeys.INFECTIVITY_SCALE_TYPE] == "SINUSOIDAL_FUNCTION_OF_TIME":
        raise ValueError(ConfigKeys.INFECTIVITY_SCALE_TYPE + " should be set to SINUSOIDAL_FUNCTION_OF_TIME "
                         "for this test to work.\n")
    param_obj[ConfigKeys.TOTAL_TIMESTEPS] = cdj[ConfigKeys.TOTAL_TIMESTEPS]
    param_obj[ConfigKeys.BASE_INFECTIVITY_CONSTANT] = cdj[ConfigKeys.BASE_INFECTIVITY_CONSTANT]
    param_obj[ConfigKeys.InfectivityScaling.AMPLITUDE] = cdj[ConfigKeys.InfectivityScaling.AMPLITUDE]
    param_obj[ConfigKeys.InfectivityScaling.PHASE] = cdj[ConfigKeys.InfectivityScaling.PHASE]
    param_obj[ConfigKeys.SIMULATION_TIMESTEP] = cdj[ConfigKeys.SIMULATION_TIMESTEP]
    param_obj[ConfigKeys.CONFIG_NAME] = cdj[ConfigKeys.CONFIG_NAME]
    param_obj[ConfigKeys.RUN_NUMBER] = cdj[ConfigKeys.RUN_NUMBER]
    return param_obj


def parse_output_file(output_filename="test.txt", simulation_timestep=1, debug=False):
    """
    creates a dataframe of time step and infected,  infectiouness and stat populations
    :param output_filename: file to parse (test.txt)
    :return: output_df:  # of infected population and infectiousness per person at each time step
    """
    filtered_lines = []
    with open(output_filename) as logfile:
        for line in logfile:
            if dtk_sft.has_match(line, matches):
                filtered_lines.append(line)
    if debug:
        with open("filtered_lines.txt", "w") as outfile:
            outfile.writelines(filtered_lines)

    # initialize variables
    time_step = 0
    infectiousness = 0
    output_df = pd.DataFrame(columns=[ConfigKeys.SIMULATION_TIMESTEP, KEY_INFECTED,
                                      KEY_INFECTIOUSNESS, KEY_STAT_POP])
    output_df.index.name = "index"
    index = 0
    for line in filtered_lines:
        if matches[0] in line:
            infected = dtk_sft.get_val(matches[1], line)
            statpop = dtk_sft.get_val(matches[3], line)
            output_df.loc[index] = [time_step, infected, infectiousness, statpop]
            index += 1
            time_step += simulation_timestep
            infectiousness = 0
            continue
        if matches[2] in line:
            infectiousness = dtk_sft.get_val(matches[2], line)
            continue
    res_path = r'./infected_vs_infectiousness.csv'
    if not os.path.exists(os.path.dirname(res_path)):
        os.makedirs(os.path.dirname(res_path))
    output_df.to_csv(res_path)
    return output_df


def parse_json_report(output_folder="output", insetchart_name="InsetChart.json", debug=False):
    """
    creates report_df dataframe with "new_infections" and "cumulative_infections" columns
    :param insetchart_name: InsetChart.json file with location (output/InsetChart.json)
    :returns: report_df dataframe
    """
    insetchart_path = path.join(output_folder, insetchart_name)
    with open(insetchart_path) as infile:
        icj = json.load(infile)["Channels"]

    new_infections = icj[KEY_NEW_INFECTIONS]["Data"]
    cumulative_infections = icj[KEY_CUMULATIVE_INFECTIONS]["Data"]

    report_dict = {KEY_NEW_INFECTIONS: new_infections,
                   KEY_CUMULATIVE_INFECTIONS: cumulative_infections}
    report_df = pd.DataFrame(report_dict)
    report_df.index.name = "index"

    if debug:
        res_path = r'./new_infections_vs_cumulative_infections.csv'
        if not os.path.exists(os.path.dirname(res_path)):
            os.makedirs(os.path.dirname(res_path))
        report_df.to_csv(res_path)
    return report_df


def calculate_infectiousness(infected_pop, index, simulation_timestep, phase, base_infectivity, amplitude, debug=False):
    """
    calculate infectiousness at each time step
    :param infected_pop:
    :param index:
    :param simulation_timestep:
    :param phase:
    :param base_infectivity:
    :param amplitude:
    :param debug:
    :return:
    """
    timestep = index * simulation_timestep
    day_in_year = float(timestep % Constants.DAYS_IN_YEAR)
    infectiousness = base_infectivity * (1.0 + amplitude * math.sin(2.0 * math.pi * (day_in_year - phase)
                                                                    / Constants.DAYS_IN_YEAR))
    infectiousness *= infected_pop
    if debug:
        print(infectiousness)
    return infectiousness


def create_report_file(param_obj, output_df, report_df, report_name, debug):
    simulation_timestep = float(param_obj[ConfigKeys.SIMULATION_TIMESTEP])
    base_inf_val = float(param_obj[ConfigKeys.BASE_INFECTIVITY_CONSTANT])
    amplitude = float(param_obj[ConfigKeys.InfectivityScaling.AMPLITUDE])
    phase = float(param_obj[ConfigKeys.InfectivityScaling.PHASE])
    infected = output_df[KEY_INFECTED]
    infectiousness = output_df[KEY_INFECTIOUSNESS]
    statpop = output_df[KEY_STAT_POP]
    new_infections = report_df[KEY_NEW_INFECTIONS]
    if debug:
        dtk_sft.plot_data(new_infections, label1="new infections", label2="NA",
                          title="Phase: {0} day, amplitude: {1}, base_infectivity: {2}"
                          .format(phase, amplitude, base_inf_val),
                          xlabel="Time_Step_{0}_Days".format(simulation_timestep), ylabel=None,
                          category='New_infections',
                          show=True, line=True)
    with open(report_name, "w") as outfile:
        outfile.write(
            "# Test name: " + str(param_obj[ConfigKeys.CONFIG_NAME]) + ", Run number: " +
            str(param_obj[ConfigKeys.RUN_NUMBER]) + "\n# Tests if the expected and actual infectiousness match "
                                                    "and that the sinusoidal freq is 1yr.\n")
        with open("DEBUG_calculated_infectiousness.txt", "w") as calc_infect_file:
            expected_infectiousness = []
            actual_infectiousness_all = []
            calc_infectiousness_all = []
            success = True
            for index in range(len(infectiousness)):
                infected_pop = int(infected[index])
                expected_infectiousness.append(calculate_infectiousness(infected_pop, index, simulation_timestep,
                                                                        phase, base_inf_val, amplitude, debug))
                timestep = index * simulation_timestep
                actual_infectiousness = float(infectiousness[index])
                calc_infectiousness = expected_infectiousness[index] / float(statpop[index])
                actual_infectiousness_all.append(actual_infectiousness)
                calc_infectiousness_all.append(calc_infectiousness)
                tolerance = 0 if calc_infectiousness == 0 else 5e-2 * calc_infectiousness
                upper_acceptance_bound = calc_infectiousness + tolerance
                lower_acceptance_bound = calc_infectiousness - tolerance
                if math.fabs(actual_infectiousness - calc_infectiousness) > tolerance:
                    success = False
                    outfile.write("BAD: actual infectiousness at time step {0} is {1}, which is not within "
                                  "acceptance range of ({2}, {3}).  Expected {4}.\n"
                                  .format(timestep, actual_infectiousness, lower_acceptance_bound,
                                          upper_acceptance_bound, calc_infectiousness))
                if debug:
                    calc_infect_file.write("Calculated infectiousness is {0} at time step {1} with {2} people "
                                           "infected.".format(calc_infectiousness, timestep, infected_pop))
        if success:
            outfile.write("GOOD: actual infectiousness for each time step was within 5% of the calculated "
                          "infectiousness.\n")
        step = int(Constants.DAYS_IN_YEAR / simulation_timestep)
        start = int(500 / simulation_timestep)  # assuming stable sinusoid after 500 days
        infectiousness_compare_vals = actual_infectiousness_all[start::step]
        if debug:
            print("The infectiousness values of the sinusoid one year apart are: " + str(infectiousness_compare_vals))
        for a, b in it.combinations(infectiousness_compare_vals, 2):
            tol_a = 7.5e-2 * a
            tol_b = 7.5e-2 * b
            if not ((a - tol_a) < b < (a + tol_a)):
                success = False
                outfile.write("BAD: The value {0} is not with in the other's range of ({1}, {2}).  Expected within "
                              "7.5% of {3}.  Sinusoidal freq off from expected 1yr.\n".format(b, a-tol_a, a+tol_a, a))
            if not ((b - tol_b) < a < (b + tol_b)):
                success = False
                outfile.write("BAD: The value {0} is not with in the other's range of ({1}, {2}).  Expected within "
                              "7.5% of {3}.  Sinusoidal freq off from expected 1yr.\n".format(a, b-tol_b, b+tol_b, b))
        if success:
            outfile.write("GOOD: Sinusoid freq=1yr, infectiousness values 1 year apart"
                          " were within 7.5% of each other.\n")
        outfile.write(dtk_sft.format_success_msg(success))
    dtk_sft.plot_data(actual_infectiousness_all, calc_infectiousness_all,
                      label1="actual infectiousness", label2="calc infectiousness",
                      title="Phase: {0} day, amplitude: {1}, base_infectivity: {2}"
                      .format(phase, amplitude, base_inf_val),
                      xlabel="Time_Step_{0}_Days".format(simulation_timestep), ylabel="Infectiousness",
                      category='Infectiousness',
                      show=True, line=True)

    return success
# endregion
