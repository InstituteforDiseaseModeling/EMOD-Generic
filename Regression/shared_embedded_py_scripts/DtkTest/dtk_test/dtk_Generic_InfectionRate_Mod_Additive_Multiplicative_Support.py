#!/usr/bin/python

import json
import dtk_test.dtk_sft as dtk_sft
import math
import pandas as pd
import os
import dtk_test.dtk_InfectivityScalingBoxcar_Support as isb_support
# import dtk_InfectivityScalingExponential_Support as ise_support
import dtk_test.dtk_InfectivityScalingSinusoidal_Support as iss_support


"""
Modifications - Multiplicative
Infectivity in a node can be increased multiplicatively by selecting Enable_Infectivity_Scaling from the simulation 
configuration. When this option is enabled, InfectivityMultiplier is expected for each Node as part of the 
"NodeAttributes" in the demographics file. InfectivityMultiplier is then used to multiply the infectivity present in 
that node.

Additionally, selecting Enable_Infectivity_Scaling allows the inclusion of several other forcing options in the 
simulation configuration:

Enable_Infectivity_Scaling_Density - (Expects Area for each node as part of the "NodeAttributes" in the demographics 
file, and Population_Density_HalfMax in the simulation configuration.) Infectivity in each node is multiplied by 
1-exp(-0.693147*stat_pop/area/pop_density_halfmax). This multiplication reduces the total infectivity in a node based on 
the population density; the value for Population_Density_HalfMax is the population density at which the infectivity has 
fallen to half of its unmodified value.

Enable_Infectivity_Scaling_Sinusoid - (Expects both InfectivitySinusoidalAmplitude and InfectivitySinusoidalForcingPhase 
for each node as part of the "NodeAttributes" in the demographics file.) Infectivity in each node is multiplied by 
1.0+sin_amp*sin(2.0*PI*(time-sin_phase)/365.0). This multiplication provides a zero-bias annual oscillation to the 
total infectivity in a node.

Enable_Infectivity_Scaling_Boxcar - (Expects InfectivityBoxcarAmplitude, InfectivityBoxcarStartTime, and 
InfectivityBoxcarEndTime for each node as part of the "NodeAttributes" in the demographics file.) Infectivity in each 
node is multiplied by 1.0+boxcar_amp if time%365 is between boxcar_start and boxcar_end. Note that this scaling is 
intended to provide non-zero bias annual oscillation; if boxcar_end < boxcar_start, then the scaling is applied either 
if time%365 < boxcar_end or time%365 > boxcar_start.

Enable_Infectivity_Scaling_Exponential - (Expects Infectivity_Exponential_Baseline, Infectivity_Exponential_Delay, and 
Infectivity_Exponential_Rate as part of the simulation configuration.) Infectivity in all nodes is multiplied by 
1-(1-exp_base)*exp(exp_delay-time)/exp_rate). This multiplication reduces infectivity to a fraction of its unmodified 
value for a specified duration; that fraction then decays back to its unmodified value based on the specified rate.


Modifications - Additive
Infectivity in a node can be increased additively by selecting Enable_Infectivity_Reservoir from the simulation 
configuration. When this option is enabled, InfectivityReservoirSize is expected for each Node as part of the 
"NodeAttributes" in the demographics file. InfectivityReservoirSize is then the quantity-per-timestep added to the 
total infectivity present in that node; it is equivalent to the expected number of additional infections in a node, per 
timestep.

This additional amount on infectivity is not affected by multiplicative modifications to infectivity.
"""


class Config:
    config_name = "Config_Name"
    simulation_timestep = "Simulation_Timestep"
    duration = "Simulation_Duration"
    infectivity = "Base_Infectivity"
    sample_rate = "Base_Individual_Sample_Rate"
    run_number = "Run_Number"
    demo_filenames = "Demographics_Filenames"
    start_time = "Start_Time"

    enable_infectivity_scaling = "Enable_Infectivity_Scaling"

    enable_infectivity_density = "Enable_Infectivity_Scaling_Density"
    halfmax = "Infectivity_Population_Density_HalfMax"

    enable_infectivity_sinusoid = "Enable_Infectivity_Scaling_Sinusoid"

    enable_infectivity_boxcar = "Enable_Infectivity_Scaling_Boxcar"

    enable_infectivity_exp = "Enable_Infectivity_Scaling_Exponential"
    exp_baseline = "Infectivity_Exponential_Baseline"
    exp_delay = "Infectivity_Exponential_Delay"
    exp_rate = "Infectivity_Exponential_Rate"

    enable_infectivity_reservoir = "Enable_Infectivity_Reservoir"


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


class Demo:
    initial_population = "InitialPopulation"
    infectivity_multiplier = "InfectivityMultiplier"
    area = "Area"
    infectivity_reservoir_size = "InfectivityReservoirSize"
    sinusoid_amp = "InfectivitySinusoidalAmplitude"
    sinusoid_phase = "InfectivitySinusoidalPhase"
    boxcar_amp = "InfectivityBoxcarAmplitude"
    boxcar_start = "InfectivityBoxcarStartTime"
    boxcar_end = "InfectivityBoxcarEndTime"


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
            Config.sample_rate, Config.demo_filenames, Config.enable_infectivity_scaling,
            Config.enable_infectivity_reservoir, Config.enable_infectivity_density,
            Config.enable_infectivity_boxcar, Config.enable_infectivity_sinusoid, Config.enable_infectivity_exp,
            Config.start_time]
    for key in keys:
        param_obj[key] = cdj[key]
    if param_obj[Config.enable_infectivity_density] == 1:
        param_obj[Config.halfmax] = cdj[Config.halfmax]
    # the following params are in demo json now
    # if param_obj[Config.enable_infectivity_sinusoid] == 1:
    #     param_obj[Config.sinusoid_amp] = cdj[Config.sinusoid_amp]
    #     param_obj[Config.sinusoid_phase] = cdj[Config.sinusoid_phase]
    # if param_obj[Config.enable_infectivity_boxcar] == 1:
    #     param_obj[Config.boxcar_amp]=cdj[Config.boxcar_amp]
    #     param_obj[Config.boxcar_end]=cdj[Config.boxcar_end]
    #     param_obj[Config.boxcar_start]=cdj[Config.boxcar_start]
    if param_obj[Config.enable_infectivity_exp] == 1:
        param_obj[Config.exp_baseline] = cdj[Config.exp_baseline]
        param_obj[Config.exp_delay] = cdj[Config.exp_delay]
        param_obj[Config.exp_rate] = cdj[Config.exp_rate]

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
        vaccine_type = event["Event_Coordinator_Config"]["Intervention_Config"][Campaign.vaccine_type]
        campaign_obj[Campaign.vaccine_type].append(vaccine_type)
        mod_acquire = event["Event_Coordinator_Config"]["Intervention_Config"]["Waning_Config"][Campaign.initial_effect]
        campaign_obj[Campaign.initial_effect].append(mod_acquire)

    if debug:
        with open("DEBUG_campaign_object.json", 'w') as outfile:
            json.dump(campaign_obj, outfile, indent=4)

    return campaign_obj


def load_demo_overlay_file(demo_filenames="demo_overlay.json", debug=False):
    """reads demo overlay file

    :param demo_filenames: demo_overlay.json file
    :returns: demo_obj structure, dictionary
    """
    with open(demo_filenames[-1]) as infile:
        demo_json = json.load(infile)
    demo_obj = {}
    for demo_param in [getattr(Demo, attr) for attr in dir(Demo) if not attr.startswith("__")]:
        if demo_param in demo_json["Nodes"][0]["NodeAttributes"]:
            demo_obj[demo_param] = demo_json["Nodes"][0]["NodeAttributes"][demo_param]

    if debug:
        with open("DEBUG_demo_object.json", 'w') as outfile:
            json.dump(demo_obj, outfile, indent=4)

    return demo_obj


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
            contagion = 0
            prob = 0
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

    # insetchart_df = pd.DataFrame(index=range(len(icj[insetchart.new_infections])))
    # insetchart_df[insetchart.new_infections] = icj[insetchart.new_infections]
    # insetchart_df[insetchart.stat_pop] = icj[insetchart.stat_pop]
    insetchart_df = pd.DataFrame.from_items([(InsetChart.new_infections, icj[InsetChart.new_infections]['Data']),
                                             (InsetChart.stat_pop, icj[InsetChart.stat_pop]['Data'])])

    if debug:
        insetchart_df.to_csv("DEBUG_data_InsetChart.csv")

    return insetchart_df


def create_report_file(param_obj, campaign_obj, demo_obj, stdout_df, insetchart_df, report_name, debug):
    with open(report_name, "w") as outfile:
        config_name = param_obj[Config.config_name]
        sample_rate = param_obj[Config.sample_rate]
        outfile.write("Config_name = {}\n".format(config_name))
        # write information into sft report file for parameter sweeping test in dtk_tools 
        outfile.write("{0} = {1} {2} = {3} {4} = {5}\n".format(Config.infectivity, param_obj[Config.infectivity],
                                                               Config.run_number, param_obj[Config.run_number],
                                                               Config.sample_rate, sample_rate))

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

        outfile.write("{0} = {1} {2} = {3} {4} = {5}\n".format(Campaign.vaccine_type, vaccine_types,
                                                               Campaign.acquisition_blocking, mod_acquire,
                                                               Campaign.transmission_blocking, mod_transmission))

        enabled_mods = []
        for enabled_mod in [Config.enable_infectivity_scaling, Config.enable_infectivity_exp,
                            Config.enable_infectivity_boxcar, Config.enable_infectivity_sinusoid,
                            Config.enable_infectivity_density, Config.enable_infectivity_reservoir]:
            #print(type(param_obj[enabled_mod]), param_obj[enabled_mod])
            if param_obj[enabled_mod] == 1:
                enabled_mods.append(enabled_mod)
        outfile.write("These are the enabled infectivity modifications: {}\n".format(enabled_mods))

        infectivity_multiplier = float(demo_obj[Demo.infectivity_multiplier]) if Demo.infectivity_multiplier in demo_obj else None
        infectivity_reservoir_size = float(demo_obj[Demo.infectivity_reservoir_size]) if Demo.infectivity_reservoir_size in demo_obj else None
        area = float(demo_obj[Demo.area]) if Demo.area in demo_obj else None
        halfmax = float(param_obj[Config.halfmax]) if Config.halfmax in param_obj else None
        boxcar_amp = float(demo_obj[Demo.boxcar_amp]) if Demo.boxcar_amp in demo_obj else None
        boxcar_start = demo_obj[Demo.boxcar_start] if Demo.boxcar_start in demo_obj else None
        boxcar_end = demo_obj[Demo.boxcar_end] if Demo.boxcar_end in demo_obj else None
        sinusoid_amp = float(demo_obj[Demo.sinusoid_amp]) if Demo.sinusoid_amp in demo_obj else None
        sinusoid_phase = float(demo_obj[Demo.sinusoid_phase]) if Demo.sinusoid_phase in demo_obj else None
        exp_baseline = float(param_obj[Config.exp_baseline]) if Config.exp_baseline in param_obj else None
        exp_delay = float(param_obj[Config.exp_delay]) if Config.exp_delay in param_obj else None
        exp_rate = float(param_obj[Config.exp_rate]) if Config.exp_rate in param_obj else None
        start_time = float(param_obj[Config.start_time]) if Config.start_time in param_obj else None


        message = ""
        for param_name, param_value in {Demo.infectivity_multiplier: infectivity_multiplier,
                                        Demo.infectivity_reservoir_size: infectivity_reservoir_size,
                                        Demo.area: area,
                                        Config.halfmax: halfmax,
                                        Demo.boxcar_amp: boxcar_amp,
                                        Demo.boxcar_start: boxcar_start,
                                        Demo.boxcar_end: boxcar_end,
                                        Demo.sinusoid_amp: sinusoid_amp,
                                        Demo.sinusoid_phase: sinusoid_phase,
                                        Config.exp_baseline: exp_baseline,
                                        Config.exp_delay:exp_delay,
                                        Config.exp_rate: exp_rate,
                                        Config.start_time: start_time}.items():
            if param_value is not None: # param_value could be 0
                message += " " + param_name + " = " + str(param_value)
            else:
                message += " " + param_name + " = None"

        outfile.write(message + "\n")

        # outfile.write("{0} = {1} {2} = {3} {4} = {5} {6} = {7} {8} = {9}\n".format(Demo.infectivity_multiplier, infectivity_multiplier,
        #                                                        Demo.area, demo_obj[Demo.area],
        #                                                        Demo.infectivity_reservoir_size, demo_obj[Demo.infectivity_reservoir_size],
        #                                                        Demo.boxcar_amp, demo_obj[Demo.boxcar_amp],
        #                                                        Config.halfmax, param_obj[Config.halfmax]))

        outfile.write("Part 1: Testing contagion and probability with calculated values for every time step:\n")
        success = True
        contagion = stdout_df[Stdout.contagion]
        prob = stdout_df[Stdout.prob]
        stat_pop = stdout_df[Stdout.stat_pop]
        infected = stdout_df[Stdout.infected]
        calculated_contagion_list = []
        calculated_prob_list = []

        mod_additive = 0
        if Config.enable_infectivity_reservoir in enabled_mods:
            mod_additive = infectivity_reservoir_size
        if Config.enable_infectivity_scaling not in enabled_mods:
            infectivity_multiplier = 1
        # the following is testing the old exponential scaling codes.
        # if Config.enable_infectivity_exp in enabled_mods:
        #     expected_infectiousness = [0] * (int(param_obj[Config.duration] / param_obj[Config.simulation_timestep]) + 1)
        #     exp_modifier = ise_support.calculate_infectiousness(new_infections=1, index=1, simulation_timestep=1,
        #                                                                total_timesteps=param_obj[Config.duration],
        #                                                                base_infectivity=1, baseline=exp_baseline,
        #                                                                delay=exp_delay, rate=exp_rate, debug=debug)
        #     expected_infectiousness = list(map(sum, zip(expected_infectiousness, exp_modifier)))
        for t in range(len(contagion)):
            mod_multiplicative = 1
            if Config.enable_infectivity_density in enabled_mods and area and halfmax : # area and halfmax could be zero
                mod_multiplicative *= 1 - math.exp(
                    -0.693147 * stat_pop[t] / area / halfmax)
            if Config.enable_infectivity_boxcar in enabled_mods:
                mod_multiplicative *= isb_support.calculate_infectiousness(infected_pop=1, index=1, simulation_timestep=t,
                                                                          start_time=boxcar_start,
                                                                          end_time=boxcar_end,
                                                                          base_infectivity=1, amplitude=boxcar_amp,
                                                                          debug=debug)
            if Config.enable_infectivity_sinusoid in enabled_mods:
                mod_multiplicative *= iss_support.calculate_infectiousness(infected_pop=1, index=1, simulation_timestep=t,
                                                                           phase=sinusoid_phase, base_infectivity=1,
                                                                           amplitude=sinusoid_amp, debug=debug)
            if Config.enable_infectivity_exp in enabled_mods:
                # mod_multiplicative *= expected_infectiousness[t]
                if t + 1 < exp_delay:  # t starts from 0, so t + 1 = time_since_sim_start
                    mod_multiplicative *= exp_baseline
                else:
                    # spec change
                    # mod_multiplicative *= 1.0 - ((1.0 - exp_baseline)* math.exp((exp_delay - t - 1)/exp_rate))
                    mod_multiplicative *= 1.0 - ((1.0 - exp_baseline) * math.exp((exp_delay - t - 1) * exp_rate))
            if Config.enable_infectivity_scaling not in enabled_mods:
                mod_multiplicative = 1

            # calculated_contagion = (param_obj[Config.infectivity] *
            #                         infected[t] * infectivity_multiplier *
            #                         mod_multiplicative * (1.0 - mod_transmission) +  mod_additive) / stat_pop[t]

            # infectivity scaling is on top of Infectivity_Reservoir
            calculated_contagion = ((param_obj[Config.infectivity] * infected[t] + mod_additive) *
                                    infectivity_multiplier * mod_multiplicative * (1.0 - mod_transmission)) / stat_pop[t]

            calculated_contagion_list.append(calculated_contagion)
            tolerance = max(5e-2 * calculated_contagion, 2e-3)
            if math.fabs(calculated_contagion - contagion[t]) > tolerance:
                success = False
                outfile.write("    BAD: at time step {0}, the total contagion is {1}, expected {2}, tolerance={3}.\n".format(
                    t, contagion[t], calculated_contagion, tolerance
                ))
            calculated_prob = 1.0 - math.exp(-1 * calculated_contagion * param_obj[Config.simulation_timestep] * (1.0 - mod_acquire))

            calculated_prob_list.append(calculated_prob)
            tolerance = max(5e-2 * calculated_prob, 2e-3)

            if math.fabs(calculated_prob - prob[t]) > tolerance:
                success = False
                outfile.write("    BAD: at time step {0}, the infected probability is {1}, expected {2}, tolerance={3}.\n".format(
                    t, prob[t], calculated_prob, tolerance
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
        #failed_timestep = []
        expected_new_infection_list = []
        start_day = campaign_obj[Campaign.start_day]
        test_stop_day = len(new_infections)
        for t in range(len(new_infections)):
            expected_new_infection = (stat_pop[t] - infected[t]) * calculated_prob_list[t] * sample_rate
            if t == start_day:
                expected_new_infection += int(stat_pop[t] * campaign_obj[Campaign.coverage] * sample_rate)

            expected_new_infection_list.append(expected_new_infection)
            if expected_new_infection == 0 and t > start_day:
                if test_stop_day == len(new_infections):
                    test_stop_day = t
        if test_stop_day - start_day > 1:
            if not dtk_sft.test_multinomial(list(new_infections[start_day:test_stop_day]),
                                            expected_new_infection_list[start_day:test_stop_day], outfile, prob_flag=False):
                success = result = False
            else:
                result = True
        else:  # for infection probability larger than 1, all individual will get infected in one time step.
            if math.fabs(expected_new_infection_list[start_day] - new_infections[start_day]) <= expected_new_infection_list[start_day] * 0.01:
                result = True
            else:
                success = result = False
                outfile.write("BAD: at time step {0}, expected new infection is {1}, got {2} from insetchart.json.\n"
                              "".format(start_day, expected_new_infection_list[start_day], new_infections[start_day]))
        outfile.write("Part 2: Result is {}.\n".format(result))

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

