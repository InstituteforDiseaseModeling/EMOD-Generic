import json
from os import path

import dtk_sft as sft
plt = sft.plt
from dtk_General_Support import ConfigKeys
import pandas as pd
import numpy as np

KEY_campaign_filename = "Campaign_Filename"
KEY_config_name = "Config_Name"
KEY_sim_start_time = "Start_Time"
KEY_simulation_duration = "Simulation_Duration"
KEY_simulation_timestep = "Simulation_Timestep"
KEY_demographics_filenames = "Demographics_Filenames"
KEY_enable_initial_prevalence = "Enable_Initial_Prevalence"
KEY_num_clades = "Number_of_Clades"
KEY_l2NoGpC = "Log2_Number_of_Genomes_per_Clade"
KEY_run_number = "Run_Number"


def load_config_file(config_filename="config.json", debug=False):
    config_keys = []
    config_keys.append(KEY_run_number)
    config_keys.append(KEY_sim_start_time)
    config_keys.append(KEY_simulation_duration)
    config_keys.append(KEY_simulation_timestep)
    config_keys.append(KEY_campaign_filename)
    config_keys.append(KEY_config_name)
    config_keys.append(KEY_demographics_filenames)
    config_keys.append(KEY_enable_initial_prevalence)
    config_keys.append(KEY_l2NoGpC)
    config_keys.append(KEY_num_clades)

    config_object = {}
    with open(config_filename) as infile:
        config_json = json.load(infile)['parameters']
        for k in config_keys:
            config_object[k] = config_json[k]

    if debug:
        with open('DEBUG_config_object.json','w') as outfile:
            json.dump(config_object, outfile, indent=4)

    return config_object


def load_campaign_file(campaign_filename="campaign.json", debug=False):
    camp_json = None
    with open(campaign_filename) as infile:
        camp_json = json.load(infile)
    events = camp_json['Events']

    campaign_dict = {}
    intervention_config = None
    for e in events:
        start_day = e['Start_Day']

        intervention_config = e['Event_Coordinator_Config']['Intervention_Config']
        clade = intervention_config['Clade']
        genome = intervention_config['Genome']
        strain_id = f"{clade}_{genome}"

        campaign_dict[strain_id] = start_day

    if debug:
        with open('DEBUG_campaign_dictionary.json','w') as outfile:
            json.dump(campaign_dict, outfile)
    return campaign_dict


def load_demographic_overlay(overlay_filename, debug=False):
    overlay_json = None
    with open(overlay_filename) as infile:
        overlay_json = json.load(infile)
    if debug:
        with open('demo_parse_log.txt','w') as outfile:
            outfile.write(f'Keys: {overlay_json.keys()}')
            n = overlay_json['Nodes']
            outfile.write(f'Nodes: {n}\n')
            o = n[0]
            outfile.write(f'Node zero: {o}\n')
            ats = o['IndividualAttributes']
            outfile.write(f'Ind atts: {ats}\n')
            strs = ats['InitialPrevalenceStrains']
            outfile.write(f'Strains: {strs}\n')
    strains_json = overlay_json['Nodes'][0]['IndividualAttributes']['InitialPrevalenceStrains']
    strains_dict = {}
    for my_strain in strains_json:
        my_clade = my_strain['Clade']
        my_genome = my_strain['Genome']
        my_fraction = my_strain['Fraction']
        my_strainid = f'{my_clade}_{my_genome}'
        strains_dict[my_strainid] = my_fraction
    return strains_dict


def create_dataframe_from_report(output_folder, report_filename, debug=False):
    reporter_path = path.join(output_folder, report_filename)
    with open(reporter_path) as infile:
        report_data = json.load(infile)

    strain_data = pd.DataFrame(report_data['data'])
    if debug:
        strain_data.to_csv("DEBUG_reporter_df.csv")
    return strain_data


def process_stdout_file(campaign_dictionary,
                        stdout_filename="test.txt",
                        start_time=0, simulation_duration=365,
                        sim_timestep=1, skip_zero_day_outbreak_infections=False,
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
    matches.append("incubation_timer initialized to ")
    matches.append("infectious_timer = ")

    filtered_lines = []
    with open(stdout_filename) as stdout:
        for line in stdout:
            if sft.has_match(line, matches):
                filtered_lines.append(line)
    if debug:
        with open("DEBUG_filtered_lines.txt","w") as outfile:
            outfile.writelines(filtered_lines)

    # First, build timestep series
    timesteps = np.arange(start_time, simulation_duration + 1, sim_timestep)

    # Second, declare dataframe with zeroes
    strain_df = pd.DataFrame(0, index=timesteps, columns=list(campaign_dictionary.keys()))

    # Then, build dictionary of times, clades and genomes from stdout
    report_dict = {}
    curr_timestep = start_time
    report_dict[curr_timestep] = {}
    all_strain_ids = []
    new_infection_line = ""
    not_counting_messages = []
    stat_pop = 0
    for line in filtered_lines:
        if matches[0] in line:
            curr_timestep += sim_timestep
            stat_pop = int(sft.get_val("StatPop: ", line))
            report_dict[curr_timestep] = {}
        elif matches[1] in line: # grab this line and save
            new_infection_line = line
            clade = ""
            genome = ""
            strain_id = ""
            individual_id = -1
            infectious_timer = 0
            incubation_timer = 0.0
        elif matches[2] in line:
            incubation_timer = float(sft.get_val(matches[2],line)) # If this line doesn't show up, no incubation, so zero
        elif matches[3] in line: # now add the infection if the timer is more than one
            clade = sft.get_val("Clade: ", new_infection_line)
            genome = sft.get_val("Genome: ", new_infection_line)
            strain_id = f"{clade}_{genome}"
            if curr_timestep == campaign_dictionary[strain_id]:
                individual_id = int(sft.get_val("Individual: ", new_infection_line))
                infectious_timer = float(sft.get_val(matches[3], line))
                if skip_zero_day_outbreak_infections:
                    if infectious_timer < 1.0 and incubation_timer == 0.0: # not counting infections that are done by end of timestep
                        # the timer filter only applies to new individuals
                        not_counting_messages.append(f'Time: {curr_timestep} Individual: {individual_id} '
                                                     f'Incubation_Timer: {incubation_timer} '
                                                     f'Strain: {strain_id} InfectiousTimer: {infectious_timer} not counted.\n')
                        continue
                else:
                    # Count all infections including zero days
                    pass

            if strain_id not in all_strain_ids:
                all_strain_ids.append(strain_id)
            if strain_id not in report_dict[curr_timestep]:
                report_dict[curr_timestep][strain_id] = 1
            else:
                report_dict[curr_timestep][strain_id] += 1
            new_infection_line = ""

    if debug:
        with open('DEBUG_strain_dictionary.json','w') as outfile:
            json.dump(report_dict, outfile, indent=4)
        with open('DEBUG_not_counted_infections.txt', 'w') as outfile:
            outfile.write("Infectious timers less than 1.0 are not counted, here's what was found:\n")
            outfile.writelines(not_counting_messages)

    # Now reset the zeros in the dataframe with the data
    for timestep in report_dict:
        for strain in report_dict[timestep]:
            strain_df.iloc[timestep][strain] = report_dict[timestep][strain]

    strain_df.set_index(strain_df.columns.values[0], drop=True)
    if debug:
        strain_df.to_csv('DEBUG_console_df.csv')
    return strain_df


def create_report_file(strain_dataframe, campaign_dictionary,
                       report_name,
                       compare_messages=None):
    """

    Args:
        strain_dataframe: dataframe of infections by strain id (strain_id = Clade_Genome)
        campaign_dictionary: keys are strain_ids, values are days introduced in campaign
        report_name: file to create (scientific_feature_report.txt)

    Returns: None

    """
    success = True # optimism
    first_days = True # sets to false if an infection shows early
    with open(report_name, 'w') as outfile:
        if compare_messages:
            for m in compare_messages:
                if m.startswith("BAD"):
                    success = False
                outfile.write(m)
        for strain in campaign_dictionary:
            reported_cases_timesteps = strain_dataframe.index[strain_dataframe[strain] > 0].tolist() # Indexes
            for ts in reported_cases_timesteps:
                if ts < campaign_dictionary[strain]: # If timestep before outbreak
                    success=False
                    first_days = False
                    outfile.write(f'BAD: at day {ts} had {strain_dataframe[strain][ts]} infections of '
                                  f'strain {strain} before outbreak at {campaign_dictionary[strain]}.\n')
                elif ts == campaign_dictionary[strain]: #If day of outbreak
                    outfile.write(f"GOOD! Outbreak day {ts} for strain {strain} has cases, as it should.\n")
            if campaign_dictionary[strain] not in reported_cases_timesteps:
                success=False
                outfile.write(f'BAD! Should have had some cases on day {campaign_dictionary[strain]} of strain {strain} but didn''t see any.\n')
        if first_days:
            outfile.write("GOOD! According to console, every instance of each strain occurred after Outbreak intervention.\n")
        outfile.write(sft.format_success_msg(success))


def compare_dataframes(console_df=None, reporter_df=None,
                       debug=False):
    strains = {}
    for strain_id in console_df.columns: # column 0 is an index
        strain_split = strain_id.split('_')
        clade = int(strain_split[0])
        genome = int(strain_split[1])
        if clade not in strains:
            strains[clade] = [genome]
        else:
            strains[clade].append(genome)

    all_messages = []
    # PRECONDITION assume only one node. Otherwise loop through nodes
    # and handle first infections differently
    for clade in strains:
        for genome in strains[clade]:
            messages = []
            reporter_df_filtered = reporter_df[
                (reporter_df['clade'] == clade) &
                (reporter_df['genome'] == genome)
            ]
            if debug:
                reporter_df_filtered.to_csv(f'DEBUG_filtered_report_{clade}_{genome}.csv')
            # first infections from console_df will show in 'tot_inf' column
            # each subsequent new inf in 'new_inf' column
            series_of_strain = console_df[f'{clade}_{genome}']
            # does the index of the first nonzero in series_of_strain == 'time' of first
            # row of reporter_df_filtered
            first_infection_time = reporter_df_filtered.iloc[0]['time']
            first_infection_count = reporter_df_filtered.iloc[0]['tot_inf']
            first_infection_count += reporter_df_filtered.iloc[0]['new_inf'] # TODO: testing this
            if series_of_strain[first_infection_time] != first_infection_count:
                messages.append(
                    f'BAD! Expected console to report first {first_infection_count} infections '
                    f'for strain {clade}_{genome} at time {first_infection_time} as per '
                    f'reporter output. Instead, got this many: {series_of_strain[first_infection_time]}\n'
                )
            curr_index = 1
            while curr_index < len(reporter_df_filtered):
                my_row = reporter_df_filtered.iloc[curr_index]
                curr_time = my_row['time']
                curr_new_infections = my_row['new_inf']
                # grab the value in the series based on time and compare
                if series_of_strain[curr_time] != curr_new_infections:
                    messages.append(
                        f'BAD! Console count (expected): {curr_new_infections} '
                        f'Reporter count (actual): {series_of_strain[curr_time]} '
                        f'timestep {curr_time} clade:{clade} genome:{genome}.\n'
                    )
                curr_index += 1
            if len(messages) == 0:
                messages.append(f'GOOD! Reporter data for Clade {clade} and Genome {genome} is reflected in console.\n')
            all_messages += (messages)
    return all_messages

def plot_strain_data(strain_df, strain_origin_dict,
                     v_line_label_x_offset=-3,
                     v_line_label_y_coord=6,
                     multinode=False,
                     show=True):
    '''

    Args:
        strain_df: Dataframe of strain incidence with index of timestep
        strain_origin_dict: dictionary of each strain and expected start timestep
        v_line_label_x_offset: how far off of the x coordinate of the origin line to offset (-3)
        v_line_label_y_coord: how far up the y axis to put the origin line label (6)
        multinode: True if dataframe is multinode WARNING only supports node ids starting with 1 ascending by single digits
        show: whether or not to plot interactively (True)

    Returns:
        None. Saves figure or plots

    '''
    if not sft.check_for_plotting():
        show=False

    strains = []
    if multinode:
        # WARNING only supports node ids starting with 1 ascending by single digits
        # (1, 2, 3, 4) OK (1, 15, 20) no (15242, 15243) no
        # ceate new columns and sum
        # first create column names from node 1
        strain_columns = {}
        for c in strain_df.columns:
            if c.startswith('1_'):
                splits = c.split('_')
                strain_id = f"{splits[1]}_{splits[2]}"
                strain_columns[strain_id] = []
                strains.append(strain_id)
        # second add each column and find the columns to sum
        for c in strain_df.columns:
            strain = c[c.find('_') + 1:] # find the rest of the string after the first underscore
            if strain in strain_columns:
                strain_columns[strain].append(c)
        # third add the strain columns to the dataframe by summing the matching columns
        for s in strain_columns:
            strain_df[s] = 0
            for ns in strain_columns[s]:
                strain_df[s] += strain_df[ns]
    else:
        strains = strain_df.columns


    lines = strain_df.plot.line(y=strains)
    for strain in strain_origin_dict:
        plt.axvline(x=strain_origin_dict[strain], linestyle='--', color='black')
        plt.text(x=strain_origin_dict[strain] + v_line_label_x_offset,
                 y=v_line_label_y_coord,
                 s=strain,
                 rotation=90)
        plt.title('Strain frequency by date \n Dashed lines are expected appearances')
    plt.savefig('Strain_Incidence_and_Origin.png')
    if show:
        plt.show()
    return None



def plot_genome_histogram(filtered_df, clade, show=False):
    """
    :param filtered_df: DataFrame of all of the data to plot.
    """
    plot_title = f'Frequency_of_clade_{clade}'
    sft.plot_hist_df(filtered_df,
                     x_column='genome',
                     y_column='tot_inf',
                     title=plot_title, show=show)

class ScientificFeatureTest():
    def __init__(self, config_filename="config.json",
                 output_folder="output",
                 sft_report_name=sft.sft_output_filename,
                 debug=False):
        self._config_parameters_key='parameters'
        self._campaign_events_key='Events'
        self._config_filename=config_filename
        self._output_folder=output_folder
        self._sft_report_name=sft_report_name
        self.debug = debug
        sft.wait_for_done()
        sft.start_report_file(output_filename=sft_report_name,
                              config_name=config_filename)

    def start_report_file(self):
        sft.start_report_file(output_filename=self._sft_report_name,
                              config_name=self._config_filename)

    def sft_print_debug(self):
        print(f'debug: {self.debug}')
        print(f'config_filename: {self._config_filename}')
        print(f'output_folder: {self._output_folder}')
        print(f'sft_report: {self._sft_report_name}')

    def actual_intervention_class(self, event_json):
        return event_json['Event_Coordinator_Config']['Intervention_Config']['class']

class StrainIdentityTest(ScientificFeatureTest):
    def __init__(self, config_filename, output_folder,
                 sft_report_name,
                 reporter_filename="ReportStrainTracking.json",
                 debug=False):
        super(StrainIdentityTest, self).__init__(config_filename,
                                                 output_folder,
                                                 sft_report_name,
                                                 debug)
        self.config_keys = {ConfigKeys.Campaign_Filename,
                            ConfigKeys.Config_Name,
                            ConfigKeys.Start_Time,
                            ConfigKeys.Simulation_Duration,
                            ConfigKeys.Simulation_Timestep,
                            ConfigKeys.Demographics_Filenames,
                            ConfigKeys.Enable_Initial_Prevalence,
                            ConfigKeys.Log2_Number_Genomes,
                            ConfigKeys.Run_Number,
                            ConfigKeys.Number_Clades,
                            ConfigKeys.Incubation_Period_Distribution,
                            ConfigKeys.Incubation_Period_Constant}
        with open(config_filename) as config_file:
            config_params = json.load(config_file)[self._config_parameters_key]
            params = {}
            for key in self.config_keys:
                params[key] = config_params[key]
            self.params = params
        self.reporter_filename = reporter_filename
        self.campaign_filename = self.params[ConfigKeys.Campaign_Filename]
        self.start_report_file()

    def sit_print_debug(self):
        self.sft_print_debug()
        print(f'reporter_filename: {self.reporter_filename}')


class StrainIdentityReporterTest(StrainIdentityTest):
    def __init__(self, config_filename, output_folder,
                 sft_report_name, reporter_filename,
                 stdout_filename="test.txt", debug=False):
        super(StrainIdentityReporterTest, self).__init__(config_filename,
                                                         output_folder,
                                                         sft_report_name,
                                                         reporter_filename,
                                                         debug)
        self.stdout_filename = stdout_filename
        #TODO: move most of the methods above into here
        #TODO: migrate reporter tests to use the class


class StrainIdentityTransmissionTest(StrainIdentityTest):
    def __init__(self, config_filename="config.json",
                 output_folder="output",
                 sft_report_name=sft.sft_output_filename,
                 reporter_filename="ReportStrainTracking.json",
                 debug=False):
        super(StrainIdentityTransmissionTest, self).__init__(config_filename,
                                                             output_folder,
                                                             sft_report_name,
                                                             reporter_filename,
                                                             debug)
        self._proportions_outbreak_key = 'outbreak'
        if self.debug:
            self.sit_print_debug()

    def parse_campaign_file(self):
        proportions_object = {}
        with open(self.campaign_filename) as infile:
            campaign_json = json.load(infile)
        events = campaign_json[self._campaign_events_key]
        outbreak_day = None
        for event in events:
            if not outbreak_day:
                outbreak_day = event['Start_Day']
            if "OutbreakIndividual" == self.actual_intervention_class(event):
                # These should all be NChooserEventCoordinator types
                coordinator = event['Event_Coordinator_Config']
                intervention_config = coordinator['Intervention_Config']
                clade = intervention_config['Clade']
                genome = intervention_config['Genome']
                distribution = coordinator['Distributions'][0]
                num_interventions = distribution['Num_Targeted'][0]
                start_day = distribution['Start_Day']
                if start_day != outbreak_day:
                    raise ValueError(f"Different start days detected in {self.campaign_filename}. "
                                     "Make sure all Start_Days are the same first.\n")
                strain_id = f'{clade}_{genome}'
                proportions_object[strain_id] = {self._proportions_outbreak_key:num_interventions}
        self.outbreak_day = outbreak_day
        return proportions_object

    def test_expectations_for_day(self, proportions_object,
                                  reporter_df, day_in_question,
                                  messages_list=[], debug=False):
        day_df = reporter_df[reporter_df['time'] == day_in_question]
        if debug:
            day_df.to_csv(f'DEBUG_report_day_{day_in_question}.csv')

        observations = []
        expected = []
        for strain_id in proportions_object:
            splits = strain_id.split('_')
            clade = int(splits[0])
            genome = int(splits[1])
            row = day_df[(day_df['clade'] == clade) & (day_df['genome'] == genome)]
            outbreak_inf = proportions_object[strain_id][self._proportions_outbreak_key]
            new_inf = row.iloc[0]['new_inf']
            observations.append(new_inf)
            expected.append(outbreak_inf)
        inital_multiplier = expected[0] / observations[0]
        scaled_obs = []
        for o in observations:
            scaled_obs.append(inital_multiplier * o)
        stats = sft.stats
        result = stats.chisquare(f_obs=scaled_obs,
                                 f_exp=expected)
        messages_list.append(f"observations: {observations}\n")
        messages_list.append(f"observations_scaled: {scaled_obs}\n")
        messages_list.append(f"expected: {expected}\n")
        messages_list.append(f"scaled result: {result}\n")
        success_message_format=f"expected p-value >= 0.05, got {result[1]}"
        if result[1] > 0.05:
            messages_list.append(f"GOOD: {success_message_format}\n")
        else:
            messages_list.append(f"BAD: {success_message_format}\n")
        return messages_list

