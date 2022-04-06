#!usr/bin/python
import os
import matplotlib
from sys import platform
if platform == "linux" or platform == "linux2":
    print('Linux OS. Using non-interactive Agg backend')
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import seaborn as sns
import pandas as pd
import json
import math
from enum import Enum, auto
from collections.abc import Sequence

import dtk_test.dtk_sft as sft
from dtk_test.dtk_OutputFile import CsvOutput, ReportEventRecorder, LineListReport
from dtk_test.dtk_InsetChart import InsetChart


columns_RER = [ReportEventRecorder.Column.Time.name,
               ReportEventRecorder.Column.Node_ID.name,
               ReportEventRecorder.Column.Age.name,
               ReportEventRecorder.Column.Infected.name]
columns_LLR = [LineListReport.Column.TIME.name,
               LineListReport.Column.NODE.name,
               LineListReport.Column.AGE.name,
               LineListReport.Column.INFECTED.name]

default_line_list_report_name = "ReportLineList.csv"
default_time_start = 0
default_time_end = float('inf')
default_non_infection_rate = 0
per_100k = 100000


class Custom_Report:
    def __init__(self, custom_report_name='custom_reports.json'):
        self.custom_report_name = custom_report_name
        self.time_start = None
        self.time_end = None
        self.report_name = None
        self.report_type = None
        self.non_infected_rate = None
        self.load_parameters()

    def load_parameters(self):
        try:
            with open(self.custom_report_name, 'r') as file:
                crj = json.load(file)["Custom_Reports"]["ReportLineList"]["Reports"][0]
            self.time_start = crj["Time_Start"] if "Time_Start" in crj else None
            self.time_end = crj["Time_End"] if "Time_End" in crj else None
            self.report_name = crj["Report_Name"] if "Report_Name" in crj else None
            self.non_infected_rate = crj["Non_Infected_Rate"] if "Non_Infected_Rate" in crj else None
            self.report_type = crj["Report_Type"] # this one is required parameter
        except KeyError as ke:
            print(f"Can't find {ke} in {self.custom_report_name}, please check the test.\n")


class Report_Type(Enum):
    INFECTION = auto()
    MORTALITY = auto()


def set_undefined_parameters(cr, messages):
    if cr.time_start == None:
        cr.time_start = default_time_start
        messages.append(f"\t'Time_Start' is undefined, using default value {default_time_start}.\n")
    if cr.time_end == None:
        cr.time_end = default_time_end
        messages.append(f"\t'Time_End' is undefined, using default value {default_time_end}.\n")
    if cr.report_name == None:
        cr.report_name = default_line_list_report_name
        messages.append(f"\t'Report_Name' is undefined, using default value {default_line_list_report_name}.\n")
    if cr.non_infected_rate == None:
        cr.non_infected_rate = default_non_infection_rate
        messages.append(f"\t'Non_Infected_Rate' is undefined, using default value {default_non_infection_rate}.\n")


def load_line_list_report(line_list_report_name, messages):
    if not os.path.exists(line_list_report_name):
        messages.append(f"BAD: line list report file is set to {line_list_report_name} but it doesn't exist.\n")
        return
    else:
        messages.append(f"Loading {line_list_report_name}:\n")
        line_list_df = CsvOutput(line_list_report_name).df
        # line_list_df = line_list_df.sort_values(by=[LineListReport.Column.TIME.name])
        return line_list_df


def filter_by_report_type(event_df, cr, messages, custom_report_name):
    if cr.report_type == Report_Type.MORTALITY.name:
        event_df = event_df[event_df[ReportEventRecorder.Column.Event_Name.name].isin(
            [ReportEventRecorder.Event.DiseaseDeaths.name,
             ReportEventRecorder.Event.NonDiseaseDeaths.name])]
    elif cr.report_type == Report_Type.INFECTION.name:
        event_df = event_df[event_df[ReportEventRecorder.Column.Event_Name.name] ==
                            ReportEventRecorder.Event.NewInfection.name]
    else:
        messages.append(f"\tBAD: Report_Type in {custom_report_name} should be an Enum with possible "
                        f"values: '{Report_Type.MORTALITY.name}' and '{Report_Type.INFECTION.name}', "
                        f"got {cr.report_type}. \n")
    return event_df


def filter_by_report_time(event_df, cr):
    event_df = event_df[(event_df[ReportEventRecorder.Column.Time.name] >= cr.time_start) &
                        (event_df[ReportEventRecorder.Column.Time.name] <= cr.time_end)]
    return event_df


def filter_by_cr(event_df, cr, messages, custom_report_name):
    event_df = filter_by_report_type(event_df, cr, messages, custom_report_name)
    event_df = filter_by_report_time(event_df, cr)
    return event_df


def filter_by_columns(df_list, columns):
    column_shape = get_shape(columns)
    if len(column_shape) == 1:
        df_list = [df[columns] for df in df_list]
    elif len(column_shape) > 1 and column_shape[0] == len(df_list):
        for i in range(len(df_list)):
            column = columns[i]
            df = df_list[i]
            df_list[i] = df[column]
    else:
        raise ValueError("filtered columns and df list are not supported")
    return df_list


def get_shape(lst, shape=()):
    if not isinstance(lst, list):
        # base case
        return shape

    if isinstance(lst[0], list):
        l = len(lst[0])
        if not all(len(item) == l for item in lst):
            msg = 'not all lists have the same length'
            raise ValueError(msg)

    shape += (len(lst),)

    # recurse
    shape = get_shape(lst[0], shape)

    return shape



def test_report_time(line_list_df, cr, messages):
    min_time = min(line_list_df[LineListReport.Column.TIME.name])
    if min_time < cr.time_start:
        messages.append(
            f"\tBAD: Line list reporter should start recording at time {cr.time_start}, but it starts "
            f"at time {min_time}.\n")
    max_time = max(line_list_df[LineListReport.Column.TIME.name])
    if max_time > cr.time_end:
        messages.append(f"\tBAD: Line list reporter should end recording at time {cr.time_end}, but it ends "
                        f"at time {max_time}.\n")


def test_report_content(line_list_df, event_df, columns_to_compare, cr, messages, debug):
    succeed = True
    if cr.report_type == Report_Type.INFECTION.name:
        if any(line_list_df[LineListReport.Column.INFECTED.name] != 1):
            succeed = False
            messages.append(f"\tBAD: expected all entries in Line List report has "
                            f"{LineListReport.Column.INFECTED.name} column = 1, found non-one values. Please see "
                            f"DEBUG_line_list_df.csv for details.\n")
    if dataframe_equals(line_list_df, event_df, messages,
                        ignore_column_names=True, columns_to_compare=columns_to_compare):
        messages.append("\tGOOD: Line list reporter works correctly with mortality output.\n")
    else:
        succeed = False
        messages.append("\tBAD: Line list reporter doesn't work correctly with mortality output.\n"
                        "\tline_list_df should match event_df, please see DEBUG_event_df.csv and "
                        "DEBUG_line_list_df.csv for details.\n")
    if not succeed or debug:
        with open("DEBUG_event_df.csv", 'w') as event_file:
            event_df.to_csv(event_file, line_terminator="\n", index=False)
        with open("DEBUG_line_list_df.csv", 'w') as line_list_file:
            line_list_df.to_csv(line_list_file, line_terminator="\n", index=False)


def dataframe_equals(line_list_df, event_df, messages, ignore_column_names=True, columns_to_compare=None):
    if not ignore_column_names:
        return line_list_df.equals(event_df)
    else:
        succeed = True
        if columns_to_compare:
            line_list_df, event_df = filter_by_columns([line_list_df, event_df], columns_to_compare)

        # compare column by column
        # line_list_columns = columns_to_compare[0]
        # even_columns = columns_to_compare[1]
        # for i in range(len(line_list_columns)):
        #     assert_equal = line_list_df[line_list_columns[i]].eq(event_df[even_columns[i]].values)
        #     if not assert_equal.all():
        #         succeed = False
        #         messages.append(f"\tBAD: {line_list_columns[i]} column in line_list_df doesn't match {even_columns[i]}"
        #                         f" in event_df.\n")
        #     else:
        #         messages.append(f"\tGOOD: {line_list_columns[i]} column in line_list_df matches {even_columns[i]}"
        #                         f" in event_df.\n")

        # compare the whole dataframe
        if line_list_df.shape == event_df.shape:
            are_equal = line_list_df.eq(event_df.values)
            if not are_equal.all().all():
                succeed = False
                messages.append(f"\tBAD: line_list_df doesn't match event_df, please see DEBUG_assert_equal.csv for details.\n")
                with open("DEBUG_assert_equal.csv", 'w') as are_equal_file:
                    are_equal.to_csv(are_equal_file, line_terminator="\n")
            else:
                messages.append(f"\tGOOD: line_list_df matches event_df.\n")

            plot_heatmap_with_binary_data(are_equal, title='assert equal result', plotname='assert_equal_heatmap.png')
            plot_scatter_with_binary_data(are_equal, title='assert equal result', plotname='assert_equal_scatter.png')
        else:
            succeed = False
            messages.append(f"\tBAD: shape of line_list_df is {line_list_df.shape} doesn't match shape of event_df"
                            f"{event_df.shape}.\n")
        return succeed


def plot_heatmap_with_binary_data(data, title, plotname):
    grid_kws = {'width_ratios': (0.75, 0.03), 'wspace': 0.05}
    fig, (ax, cbar_ax) = plt.subplots(1, 2, gridspec_kw=grid_kws)
    # Discretized colorbar
    cmap = sns.cubehelix_palette(start=2.8, rot=.1, light=0.9, n_colors=2)

    sns.heatmap(data, yticklabels=False, ax=ax, cbar_ax=cbar_ax, cmap=ListedColormap(cmap)
                #,linewidths=0, linecolor='lightgray'
                )
    ax.set_title(title)
    # Customize tick marks and positions
    cbar_ax.set_yticklabels(['Not Match', 'Match'])
    cbar_ax.yaxis.set_ticks([0, 1])
    fig.savefig(plotname)
    if sft.check_for_plotting():
        plt.show()
    plt.close()


def plot_scatter_with_binary_data(data, title, plotname):
    fig, axs = plt.subplots(len(data.columns), 1)
    axs[0].set_title(title)
    for i in range(len(data.columns)):
        column = data.columns[i]
        ax = axs[i]
        # disable seaborn scatterplot until we have seaborn package upgraded in hpc
        # sns.scatterplot(x = data.index, y=column, data=data, ax=ax, edgecolor="none", alpha = 0.7)
        ax.scatter(x = data.index, y=data[column],edgecolor="none", alpha = 0.7)
        ax.set_ylim(0, 1)
        ax.set_yticklabels(['Not Match', 'Match'])
        ax.set_yticks([0, 1])
    axs[-1].set_xlabel("index")
    fig.savefig(plotname)
    if sft.check_for_plotting():
        plt.show()
    plt.close()


def test_non_infected_rate(non_infected_line_list_df, cr, stat_pop_df, messages):
    non_infected_line_list_df["Year"] = non_infected_line_list_df[LineListReport.Column.TIME.name] // \
                                        sft.DAYS_IN_YEAR
    df = non_infected_line_list_df.groupby("Year").size().reset_index(name='counts')

    stat_pop_df = stat_pop_df.reset_index(ReportEventRecorder.Column.Time.name)
    stat_pop_df["Year"] = stat_pop_df[ReportEventRecorder.Column.Time.name] // sft.DAYS_IN_YEAR
    stat_pop_mean_df = stat_pop_df[['Year', 'Statistical Population']].groupby("Year").mean().reset_index()
    non_infected_dict = {"non_infected_count": [],
                         "expected_non_infected_count": [],
                         "tolerance": []}
    for year in range(cr.time_start//sft.DAYS_IN_YEAR,
                      min(cr.time_end//sft.DAYS_IN_YEAR, max(stat_pop_mean_df['Year']))+1):
        non_infected_count = df[df['Year']==year]['counts']
        non_infected_count = non_infected_count.iloc[0] if not non_infected_count.empty else 0
        stat_pop_year = stat_pop_mean_df[stat_pop_mean_df['Year']==year]['Statistical Population'].iloc[0]
        expected_non_infected_count = cr.non_infected_rate * stat_pop_year / per_100k
        tolerance = sft.cal_tolerance_poisson(expected_non_infected_count, prob=0.05) \
            if expected_non_infected_count >=10 \
            else 0.5
        message = f'at year {year} expected {expected_non_infected_count} non_infected count in Line List report, ' + \
                  f'got {non_infected_count}'
        if math.fabs(non_infected_count - expected_non_infected_count) > tolerance * expected_non_infected_count:
            messages.append(f"\tBAD: {message} which exceeds the tolerance({tolerance:.0%}).\n")
        else:
            messages.append(f"\tGOOD: {message} which is within the tolerance({tolerance: .0%}).\n")
        non_infected_dict["non_infected_count"].append(non_infected_count)
        non_infected_dict["expected_non_infected_count"].append(expected_non_infected_count)
        non_infected_dict["tolerance"].append(tolerance * expected_non_infected_count)


    non_infected_df = pd.DataFrame.from_dict(non_infected_dict)
    plot_non_infected_count(non_infected_df)
    pass


def plot_non_infected_count(non_infected_df):
    fig, ax = plt.subplots()
    ax.errorbar(x=non_infected_df["expected_non_infected_count"].index,
                y=non_infected_df['expected_non_infected_count'],
                yerr=non_infected_df["tolerance"], label='expected_non_infected_count', color='blue')
    ax.plot(non_infected_df["non_infected_count"].index, non_infected_df['non_infected_count'],
            color='green', label='non_infected_count', marker='o', markersize=8, linewidth=2)

    ax.set_xlabel("Year")
    ax.set_title("Non Infected Count per year")
    plt.legend(loc=0)
    fig.savefig("non_infected_count.png")
    if sft.check_for_plotting():
        plt.show()
    plt.close()


def create_report_file(event_df, config_filename, output_folder, sft_output_filename, debug=False):
    messages = []

    config_name = sft.get_config_name(config_filename)
    messages.append(f"Config_name = {config_name}\n")

    custom_report_name = sft.get_config_parameter(config_filename, ["Custom_Reports_Filename"])[0]
    cr = Custom_Report(custom_report_name)
    set_undefined_parameters(cr, messages)

    line_list_report_name = os.path.join(output_folder, cr.report_name)
    line_list_df = load_line_list_report(line_list_report_name, messages)
    if line_list_df is not None and not line_list_df.empty:
        messages.append(f"Filtering test data from ReportEventRecorder based on parameters in Line "
                        f"List Reporter:\n")
        event_df = filter_by_cr(event_df, cr, messages, custom_report_name)

        columns_to_compare = [columns_LLR, columns_RER]
        line_list_df, event_df = filter_by_columns([line_list_df, event_df], columns_to_compare)

        messages.append(f"Comparing {line_list_report_name} with ReportEventRecorder:\n")
        test_report_time(line_list_df, cr, messages)
        messages.append(f"Report_Type is {cr.report_type} and Non_Infected_Rate is {cr.non_infected_rate}\n")
        if cr.report_type == Report_Type.MORTALITY.name or \
                (cr.report_type == Report_Type.INFECTION.name and not cr.non_infected_rate):
            messages.append(f"line_list_df and event_df should match each other at this point.\n")
            test_report_content(line_list_df, event_df, columns_to_compare, cr, messages, debug)
        else:# same as elif cr.report_type == Report_Type.INFECTION.name and cr.non_infected_rate:
            messages.append(f"Only infected portion in line_list_df and event_df should match each other at this point.\n")
            infected_line_list_df = line_list_df[line_list_df[LineListReport.Column.INFECTED.name] == 1].copy()
            test_report_content(infected_line_list_df, event_df, columns_to_compare, cr, messages, debug)

            messages.append(f"Test Non_Infected_Rate:\n")
            non_infected_line_list_df = line_list_df[line_list_df[LineListReport.Column.INFECTED.name] == 0].copy()
            stat_pop_df  = InsetChart(file=os.path.join(output_folder, 'InsetChart.json'),
                                      channel_names="Statistical Population").df
            test_non_infected_rate(non_infected_line_list_df, cr, stat_pop_df, messages)

    else:
        messages.append("BAD: can't find line list report file or it's empty, stop the test.\n")

    succeed = True
    with open(sft_output_filename, 'w') as sft_out_file:
        for msg in messages:
            if "BAD" in msg:
                succeed = False
            sft_out_file.write(msg)
        sft_out_file.write(sft.format_success_msg(succeed))


def create_report_file_mod_acquire(event_df, stdout_df, config_filename, output_folder, sft_output_filename):
    messages = []

    config_name = sft.get_config_name(config_filename)
    messages.append(f"Config_name = {config_name}\n")

    custom_report_name = sft.get_config_parameter(config_filename, ["Custom_Reports_Filename"])[0]
    cr = Custom_Report(custom_report_name)
    set_undefined_parameters(cr, messages)

    line_list_report_name = os.path.join(output_folder, cr.report_name)
    line_list_df = load_line_list_report(line_list_report_name, messages)
    if line_list_df is not None and not line_list_df.empty:
        messages.append(f"Filtering test data from ReportEventRecorder based on parameters in Line "
                        f"List Reporter:\n")
        event_df = filter_by_cr(event_df, cr, messages, custom_report_name)

        messages.append("Compare mod_acquire with expected value in StdOut using ReportEventRecorder as lookup table "
                        "for ID:\n")
        pass_count = fail_count = 0
        for index, row in line_list_df.iterrows():
            age = row[LineListReport.Column.AGE.name]
            mod_acquire=row[LineListReport.Column.MODACQUIRE.name]
            time = row[LineListReport.Column.TIME.name]

            idval = event_df[(event_df[ReportEventRecorder.Column.Time.name]==time) &
                          (event_df[ReportEventRecorder.Column.Age.name]==age)][ReportEventRecorder.Column.Individual_ID.name]
            idval = idval.iloc[0] if not idval.empty else None

            if cr.report_type == Report_Type.INFECTION.name:
                expected_mod_acquire = stdout_df[(stdout_df["Id"]==idval) & (stdout_df["Time"]==time)]['immunity']
                expected_mod_acquire = expected_mod_acquire.iloc[0] if not expected_mod_acquire.empty else 1.0
            else:
                expected_mod_acquire = stdout_df[stdout_df["Id"] == idval]['immunity']
                expected_mod_acquire = expected_mod_acquire.iloc[-1] if not expected_mod_acquire.empty else 1.0

            msg = f"at time {time}, Individual {idval} with age = {age} reports MODACQUIRE = {mod_acquire}, while expected" \
                f" value = {expected_mod_acquire}.\n"
            if mod_acquire is not None and mod_acquire == expected_mod_acquire:
                pass_count += 1
            else:
                fail_count += 1
                messages.append(f"BAD: {msg}")

        if pass_count <= 0:
            messages.append("BAD: none of the mod_acquire values match expected values.\n")
        elif fail_count == 0:
            messages.append("GOOD: all mod_acquire values match expected values.\n")
    else:
        messages.append("BAD: can't find line list report file or it's empty, stop the test.\n")

    succeed = True
    with open(sft_output_filename, 'w') as sft_out_file:
        for msg in messages:
            if "BAD" in msg:
                succeed = False
            sft_out_file.write(msg)
        sft_out_file.write(sft.format_success_msg(succeed))

