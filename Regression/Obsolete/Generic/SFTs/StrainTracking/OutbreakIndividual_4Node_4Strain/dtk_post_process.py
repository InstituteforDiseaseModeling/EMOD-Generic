import dtk_sft as sft
import dtk_Strain_Identity_Support as d_sis

import numpy as np
import pandas as pd
import json


def matrix_nodes_by_strains(strains, node_count=1):
    curr_node = 0
    columns = []
    while (curr_node < node_count):
        curr_node += 1
        for s in strains:
            columns.append(f'{curr_node}_{s}')
    return columns


def process_stdout_file_multinode(campaign_dictionary,
                        stdout_filename="test.txt",
                        start_time=0, simulation_duration=365,
                        sim_timestep=1, num_nodes=1,
                        debug=False):
    """

    Args:
        campaign_dictionary: keys are strains, values are days introduced
        stdout_filename: StdOut from Eradication (test.txt)
        start_time: Start_Time parameter from config (0)
        simulation_duration: Simulation_Duration from config (365)
        sim_timestep: Simulation_Timestep from config (1)
        debug: Write filtered lines, strain dictionary and dataframe (as csv) to disk

    Returns: strain_dataframe of new infections by strain per timestep

    """

    matches = []
    matches.append("Update(): Time: ")
    matches.append("New Infection: Individual")

    filtered_lines = []
    with open(stdout_filename) as stdout:
        started = False
        for line in stdout:
            if not started:
                if line.startswith("00:00:"):
                    started = True
                continue
            if sft.has_match(line, matches):
                filtered_lines.append(line)
            elif not line.startswith("00:00:"): # can't see this test ever taking one minute
                filtered_lines.append(line)
    if debug:
        with open("DEBUG_filtered_lines.txt","w") as outfile:
            outfile.writelines(filtered_lines)

    # First, build timestep series
    timesteps = np.arange(start_time, simulation_duration + 1, sim_timestep)

    # Second, create columns node_clade_genome
    columns = matrix_nodes_by_strains(strains=campaign_dictionary.keys(), node_count=num_nodes)

    # Third, build dataframe with zeroes
    strain_df = pd.DataFrame(0, index=timesteps, columns=list(columns))

    # Then, build dictionary of times, clades and genomes from stdout
    report_dict = {}
    curr_timestep_core_0 = start_time
    curr_timestep_core_1 = start_time
    report_dict[f'{curr_timestep_core_0}_0'] = {}
    report_dict[f'{curr_timestep_core_1}_1'] = {}
    strains_cores = {}
    broken_line = None
    for line in filtered_lines:
        if broken_line and not line.startswith("00:00:"): # mend the line and continue
            line = broken_line + line
            broken_line = None
        if matches[0] in line:
            try:
                rank = int(sft.get_val("Rank: ", line))
            except (LookupError, ValueError): #Most likely, we've got a broken line as we are in multicore
                broken_line = line[:-1]
                continue
            if rank == 0:
                curr_timestep_core_0 += sim_timestep
                report_dict[f'{curr_timestep_core_0}_0'] = {}
            elif rank == 1:
                curr_timestep_core_1 += sim_timestep
                report_dict[f'{curr_timestep_core_1}_1'] = {}
            else: # Test only supporting two cores
                raise ValueError(f'Expected only to have two cores. An update line indicated rank: {rank}')
        elif matches[1] in line:
            try:
                clade = int(sft.get_val("Clade: ", line))
                genome = int(sft.get_val("Genome: ", line))
                node = int(sft.get_val("Node: ", line))
            except (LookupError, ValueError): #Most likely, we've got a broken line as we are in multicore
                broken_line = line[:-1] # drop the /n character
                continue
            strain_id = f"{clade}_{genome}"
            node_strain = f"{node}_{strain_id}"
            if strain_id not in strains_cores:
                rank_start_index = line.find('[')
                core = int(line[rank_start_index + 1])
                strains_cores[strain_id] = core
            core = strains_cores[strain_id]
            if core == 0:
                timestep_core_index = f'{curr_timestep_core_0}_0'
                if node_strain not in report_dict[timestep_core_index]:
                    report_dict[timestep_core_index][node_strain] = 1
                else:
                    report_dict[timestep_core_index][node_strain] += 1
            else: # On core 1
                timestep_core_index = f'{curr_timestep_core_1}_1'
                if node_strain not in report_dict[timestep_core_index]:
                    report_dict[timestep_core_index][node_strain] = 1
                else:
                    report_dict[timestep_core_index][node_strain] += 1

    if debug:
        with open('DEBUG_strain_dictionary.json','w') as outfile:
            json.dump(report_dict, outfile, indent=4)

    # Now reset the zeros in the dataframe with the data
    for timestep_core in report_dict:
        for node_strain in report_dict[timestep_core]:
            timestep = int(timestep_core[:timestep_core.find('_')])
            count = report_dict[timestep_core][node_strain]
            strain_df.iloc[timestep][node_strain] = count

    strain_df.set_index(strain_df.columns.values[0], drop=True)
    if debug:
        strain_df.to_csv('DEBUG_console_df.csv')
    return strain_df

def compare_strain_dataframes(reporter_df, console_df, sft_report_filename=sft.sft_output_filename, debug=False):
    started_strains = []
    success = True
    with open (sft_report_filename, 'w') as outfile:
        outfile.write("Beginning test: checking to see if each reporter row (clade, genome, node, time) "
                      "corresponds to an equal count from the console log (test.txt).\n")
        for index, row in reporter_df.iterrows():
            clade = int(row['clade'])
            genome = int(row['genome'])
            node = int(row['node'])
            timestep = int(row['time'])
            strain_id = f"{clade}_{genome}"
            if strain_id not in started_strains:
                started_strains.append(strain_id)
                count = row['tot_inf']
            else:
                count = row['new_inf']
            node_strain = f"{node}_{strain_id}"
            if debug:
                print(f"timestep: {timestep} node_strain: {node_strain}")
                print(f"row: {row}")
                print(f"reporter_df.columns: {reporter_df.columns}")
            console_count = console_df.iloc[timestep][node_strain]
            if count != console_count:
                success=False
                outfile.write(f"BAD! Timestep: {timestep} node: {node} strain: {strain_id} "
                              f"Expected (reporter): {count} Actual (console): {console_count}\n")
        if success:
            outfile.write("GOOD! each row in the reporter was reproduced in console output\n")
        outfile.write(sft.format_success_msg(success))

def application(output_folder="output", config_filename="config.json",
                stdout_filename="test.txt", reporter_filename="ReportStrainTracking.json",
                report_name=sft.sft_output_filename, debug=False):
    """

    Args:
        output_folder: (output)
        config_filename: config to load for variables (config.json)
        stdout_filename:  stdout file to parse (test.txt)
        reporter_filename: output file from ReportStrainTracking.dll (ReportStrainTracking.json)
        report_name: report file to generate (scientific_feature_report.txt)
        debug: write lots of files as DEBUG_* for further investigation

    Returns:

    """
    if debug:
        print("output_folder: {0}".format(output_folder))
        print("config_filename: {0}".format(config_filename))
        print("stdout_filename: {0}".format(stdout_filename))
        print("reporter_filename: {0}".format(reporter_filename))
        print("report_name: {0}".format(report_name))
        print("debug: {0}".format(debug))

    sft.wait_for_done()
    cfg_object = d_sis.load_config_file(config_filename=config_filename, debug=debug)
    # sft.start_report_file(output_filename=report_name, config_name=cfg_object[d_sis.KEY_config_name])
    inital_strain_id = None
    if cfg_object[d_sis.KEY_enable_initial_prevalence]==1 and len(cfg_object[d_sis.KEY_demographics_filenames]) == 2:
        inital_strain_id = d_sis.load_demographic_overlay(cfg_object[d_sis.KEY_demographics_filenames][1])

    camp_dict = d_sis.load_campaign_file(cfg_object[d_sis.KEY_campaign_filename])
    if inital_strain_id:
        camp_dict[inital_strain_id] = cfg_object[d_sis.KEY_sim_start_time]

    strain_df = process_stdout_file_multinode(stdout_filename=stdout_filename,
                                              campaign_dictionary=camp_dict,
                                              start_time=cfg_object[d_sis.KEY_sim_start_time],
                                              simulation_duration=cfg_object[d_sis.KEY_simulation_duration],
                                              sim_timestep=cfg_object[d_sis.KEY_simulation_timestep],
                                              num_nodes=4,
                                              debug=debug)

    reporter_df = d_sis.create_dataframe_from_report(output_folder, reporter_filename, debug=debug)
    compare_strain_dataframes(reporter_df, strain_df, sft_report_filename=report_name)
    d_sis.plot_strain_data(strain_df, camp_dict, multinode=True)


if __name__=="__main__":
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    p.add_argument('-c', '--configname', default="config.json", help="config filename (config.json")
    p.add_argument('-r', '--reportname', default="scientific_feature_report.txt", help="report filename ({0})".format("scientific_feature_report.txt"))
    p.add_argument('-s', '--stdout', default='test.txt', help='Standard Out filename (test.txt)')
    p.add_argument('-d', '--debug', action='store_true', help='turns on debugging')
    args = p.parse_args()

    application(output_folder=args.output, report_name=args.reportname, config_filename=args.configname,
                stdout_filename=args.stdout,
                debug=args.debug)
