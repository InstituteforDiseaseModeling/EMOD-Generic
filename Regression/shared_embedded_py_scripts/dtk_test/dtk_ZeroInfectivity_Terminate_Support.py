
import json
import os.path as path
import dtk_test.dtk_sft as sft


class LocalConstants:
    cfg_Simulation_Duration = "Simulation_Duration"
    cfg_Enable_Terminate = "Enable_Termination_On_Zero_Total_Infectivity"
    cfg_Min_End_Time = "Minimum_End_Time"
    cfg_Num_Cores = "Num_Cores"

    icj_Infected = "Infected"
    icj_Susceptible = "Susceptible Population"
    pass

"""
Enable Termination on Zero Infectivity allows the modeler to cause the simulation to stop when there are no infections left
"""
def parse_json_report(output_folder="output", insetchart_name="InsetChart.json"):
    """
    creates report_data_obj structure with keys
    :param propertyreport_name: PropertyReport.json file with location (output/PropertyReport.json)
    :returns: report_data_obj structure, dictionary with KEY_NEW_INFECTIONS etc., keys (e.g.)
    """
    propertyreport_path = path.join(output_folder, insetchart_name)
    with open(propertyreport_path) as infile:
        icj = json.load(infile)["Channels"]

    report_data_obj = {}
    report_data_obj[LocalConstants.icj_Infected] = icj[LocalConstants.icj_Infected]
    report_data_obj[LocalConstants.icj_Susceptible] = icj[LocalConstants.icj_Susceptible]
    return report_data_obj


def parse_config_file(config_filename, debug=False):
    with open (config_filename) as infile:
        params = json.load(infile)['parameters']
        pass

    config_object = {}
    config_object[LocalConstants.cfg_Enable_Terminate] = params[LocalConstants.cfg_Enable_Terminate]
    config_object[LocalConstants.cfg_Min_End_Time] = params[LocalConstants.cfg_Min_End_Time]
    config_object[LocalConstants.cfg_Num_Cores] = params[LocalConstants.cfg_Num_Cores]
    config_object[LocalConstants.cfg_Simulation_Duration] = params[LocalConstants.cfg_Simulation_Duration]
    if debug:
        with open("DEBUG_config_object.json","w") as outfile:
            json.dump(config_object, outfile, sort_keys=True, indent=4)
    return config_object


def create_report_file(report_name, config_object, report_object, debug=False):
    with open(report_name, "w") as outfile:
        success = True
        # # all: length of infected channel >= minimum
        actual_duration = len(report_object[LocalConstants.icj_Infected]['Data'])
        config_duration = config_object[LocalConstants.cfg_Simulation_Duration]
        if actual_duration < config_object[LocalConstants.cfg_Min_End_Time]:
            success = False
            outfile.write(
                f"Expect every sim to run longer than {LocalConstants.cfg_Min_End_Time}"
                f" of {config_object[LocalConstants.cfg_Min_End_Time]}. Observed: {actual_duration}.\n"
            )
            pass
        outfile.write(
            f"Verifying that {LocalConstants.cfg_Enable_Terminate} "
            f"enabled: {config_object[LocalConstants.cfg_Enable_Terminate] == 1}.\n"
        )
        enable_honored = (actual_duration < config_duration)
        duration_honored = (actual_duration == config_duration)
        sim_duration_string = (
            f"\tConfiguration called for Sim_Duration of {config_duration}. "
            f"\tSim ran for {actual_duration} timesteps."
        )
        if config_object[LocalConstants.cfg_Enable_Terminate] == True:
            if enable_honored:
                outfile.write(f"GOOD: {LocalConstants.cfg_Enable_Terminate} enabled and sim honored it.\n")
            else:
                outfile.write(f"BAD: {LocalConstants.cfg_Enable_Terminate} isn''t being honored as it should.\n")
            pass
        else:
            if duration_honored:
                outfile.write(
                    f"GOOD: {LocalConstants.cfg_Simulation_Duration} expected "
                    f"and sim ran {actual_duration} timesteps.\n"
                )
            else:
                outfile.write(
                    f"BAD: {config_duration} expected "
                    f"but sim ran {actual_duration} timesteps.\n"
                )
        # # all: if infected at last timestep == 0, then channel before that is nonzero
        # # all: if infected at last timestep > 0, then length of infected channel = Simulation_Duration
        outfile.write(sft.format_success_msg(success))
    sft.plot_data(report_object[LocalConstants.icj_Infected]['Data'],
                  report_object[LocalConstants.icj_Susceptible]['Data'],
                  label1=LocalConstants.icj_Infected,
                  label2=LocalConstants.icj_Susceptible)
    if debug:
        print( "SUMMARY: Success={0}\n".format(success) )
    return success

