#!usr/bin/python
from __future__ import division
import json
import dtk_test.dtk_sft as sft
import os
import matplotlib
if os.environ.get('DISPLAY','') == '':
    print('no display found. Using non-interactive Agg backend')
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy import stats
import pdb


class CampaignKeys:
    VACCINE_TAKE_VS_AGE_MAP = "Vaccine_Take_Vs_Age_Map"
    DEMOGRAPHIC_COVERAGE = "Demographic_Coverage"
    INTERVENTION_CONFIG = "Intervention_Config"
    START_DAY = "Start_Day"
    EVENT_COORDINATOR_CONFIG = "Event_Coordinator_Config"
    TIMES = "Times"
    VALUES = "Values"
    EXISTING_ANTIBODY_BLOCKING_MULTIPLIER = "Existing_Antibody_Blocking_Multiplier"


class ConfigKeys:
    CAMPAIGN_FILENAME = "Campaign_Filename"
    CONFIG_NAME = "Config_Name"
    RUN_NUMBER = "Run_Number"
    SIMULATION_TIMESTEP = "Simulation_Timestep"
    TOTAL_TIMESTEPS = "Simulation_Duration"
    START_TIME = "Start_Time"


class DemographicsKeys:
    INDIVIDUAL_ATTRIBUTES = "IndividualAttributes"
    NODE_ATTRIBUTES = "NodeAttributes"
    AGE_DISTRIBUTION_1 = "AgeDistribution1"


"""
MeaslesVaccine is derived from SimpleVaccine, and preserves many of the same parameters.
The behavior is much like SimpleVaccine, except that it is age-dependent.  After distribution, the effect evolves
according to the Waning_Config, just like a Simple Vaccine.
All parameters necessary to define a SimpleVaccine are also necessary for a MeaslesVaccine
"""


def load_emod_parameters(config_filename, debug=False):
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    param_obj[ConfigKeys.CONFIG_NAME] = cdj[ConfigKeys.CONFIG_NAME]
    param_obj[ConfigKeys.RUN_NUMBER] = cdj[ConfigKeys.RUN_NUMBER]
    param_obj[ConfigKeys.SIMULATION_TIMESTEP] = cdj[ConfigKeys.SIMULATION_TIMESTEP]
    param_obj[ConfigKeys.TOTAL_TIMESTEPS] = cdj[ConfigKeys.TOTAL_TIMESTEPS]
    param_obj[ConfigKeys.START_TIME] = cdj[ConfigKeys.START_TIME]
    param_obj[ConfigKeys.CAMPAIGN_FILENAME] = cdj[ConfigKeys.CAMPAIGN_FILENAME]
    if debug:
        with open("DEBUG_param_obj.json","w") as outfile:
            json.dump(param_obj, outfile, indent=4)
    return param_obj


def parse_data(campaign_obj, filtered_lines, debug=False):
    if len(filtered_lines) ==0:
        raise ValueError("There's no people!!")
    d = {}
    columns = ['age', 'take']
    outputdf = pd.DataFrame(columns=columns)
    prev = -1
    age_list = []
    take_list = []
    for i in range(1, len(campaign_obj[CampaignKeys.TIMES]) + 1):
        num_in_age_bin = 0
        d[i] = 0
        for line in filtered_lines:
            age = line[:-2]
            age = float(age.split()[-1])
            if (i < len(campaign_obj[CampaignKeys.TIMES])) and (prev < age <= campaign_obj[CampaignKeys.TIMES][i]):
                num_in_age_bin += 1
                age_list.append(age)
                if "not" not in line:
                    d[i] = d[i] + 1
                    take_list.append(1)
                else:
                    take_list.append(0)
            if i == len(campaign_obj[CampaignKeys.TIMES]) and age > prev:
                num_in_age_bin += 1
                age_list.append(age)
                if "not" not in line:
                    d[i] = d[i] + 1
                    take_list.append(1)
                else:
                    take_list.append(0)
        prev = campaign_obj[CampaignKeys.TIMES][i] if i < len(campaign_obj[CampaignKeys
                                                              .TIMES]) else campaign_obj[CampaignKeys.TIMES][-1]
        d[i] = d[i] / num_in_age_bin if num_in_age_bin is not 0 else 0
    outputdf['age'] = age_list
    outputdf['take'] = take_list
    if debug:
        with open("AgeAndTakeDataframe.csv","w") as outfile:
            outputdf.to_csv(outfile)
    return d, outputdf


def load_campaign_file(campaign_filename, debug=False):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    campaign_obj = {}
    campaign_obj[CampaignKeys.VACCINE_TAKE_VS_AGE_MAP] = cf[0][CampaignKeys.EVENT_COORDINATOR_CONFIG][CampaignKeys.
        INTERVENTION_CONFIG][CampaignKeys.VACCINE_TAKE_VS_AGE_MAP]
    campaign_obj[CampaignKeys.VALUES] = cf[0][CampaignKeys.EVENT_COORDINATOR_CONFIG][CampaignKeys.
        INTERVENTION_CONFIG][CampaignKeys.VACCINE_TAKE_VS_AGE_MAP][CampaignKeys.VALUES]
    campaign_obj[CampaignKeys.TIMES] = cf[0][CampaignKeys.EVENT_COORDINATOR_CONFIG][CampaignKeys.
        INTERVENTION_CONFIG][CampaignKeys.VACCINE_TAKE_VS_AGE_MAP][CampaignKeys.TIMES]
    campaign_obj[CampaignKeys.EXISTING_ANTIBODY_BLOCKING_MULTIPLIER] = cf[0][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.EXISTING_ANTIBODY_BLOCKING_MULTIPLIER]
    campaign_obj["start_day"] = [cf[0][CampaignKeys.START_DAY]]
    if debug:
        with open("DEBUG_campaign_obj.json","w") as outfile:
            json.dump(campaign_obj, outfile, indent=4)
    return campaign_obj


def parse_output_file(stdout_filename, debug):
    filtered_lines = []
    with open(stdout_filename) as infile:
        for line in infile:
            if ("take for individual ") in line:
                filtered_lines.append(line)

    if debug:
        with open("DEBUG_filtered_lines.txt", "w") as debug_file:
            debug_file.writelines(filtered_lines)
    return filtered_lines


def create_report_file(param_obj, campaign_obj, report_name, stdout_filename,
                       debug):

    filtered_lines = parse_output_file(stdout_filename, debug)
    test_values = list(campaign_obj[CampaignKeys.VALUES])
    test_values = [t * campaign_obj[CampaignKeys.EXISTING_ANTIBODY_BLOCKING_MULTIPLIER] for t in test_values]
    slopes = ((np.diff(test_values)) / (np.diff(campaign_obj[CampaignKeys.TIMES]))).tolist()
    slopes.append(0)  # for the last value, since after the map values should stay at 1
    print(slopes)
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + param_obj[ConfigKeys.CONFIG_NAME] + ", Run number: " +
                      str(param_obj[ConfigKeys.RUN_NUMBER]) + "\n# Tests the expected age-dependent vaccine take versus"
                                                              " the actual vaccine take.\n")
        d, outputdf = parse_data(campaign_obj, filtered_lines)
        vals = list(d.values())
        keys = list(campaign_obj[CampaignKeys.TIMES])
        d2 = dict(zip(keys, vals))
        outfile.write(f"Actual take vs. age map: {d2}\n")
        actual_slopes = []
        prev = -1

        # loop through the distribution and slice the dataframe
        slopes_std_err = []
        result_messages = []

        slope_template = "Slope expected: {0}\tSlope calculated: {1}\tError calculated: {2}\tIs within two error: {3}"
        success_template = "GOOD: {0}\n"
        fail_template = "BAD: {0}\n"

        fit_data_segments = []
        est_data_segments = []

        x_list = []
        y_list = []
        y_ints = []
        success = False  # we assume failure until we find at least one fit
        for x in range(1, len(campaign_obj[CampaignKeys.TIMES]) + 1):
            if x < len(campaign_obj[CampaignKeys.TIMES]):
                df_age_bucket = outputdf[outputdf['age'] <= campaign_obj[CampaignKeys.TIMES][x]]
                df_age_bucket = df_age_bucket[df_age_bucket['age'] > prev]
                prev = campaign_obj[CampaignKeys.TIMES][x]
            else:
                df_age_bucket = outputdf[outputdf['age'] > campaign_obj[CampaignKeys.TIMES][-1]]
            df_age_bucket = df_age_bucket.sort_values(by='age', ascending=True)
            data_age = df_age_bucket['age']
            data_take = df_age_bucket['take']
            if len(data_age) < 100:
                outfile.write("WARNING: Data bin is less than 100 ({0}), so this test may fail because of low data amount.".format( len( data_age ) ))
            linear_fit = stats.linregress(data_age, data_take)
            actual_slopes.append(linear_fit[0])
            y_ints.append(linear_fit[1])
            c = x - 1  # messy indexing, but has to be done because of the way the campaign map works to test all slopes
            if c < len(campaign_obj[CampaignKeys.TIMES]):
                min_age_idx = df_age_bucket['age'].idxmin()
                max_age = campaign_obj[CampaignKeys.TIMES][c]
                if debug:
                    print("min_age_idx: {0}".format(min_age_idx))

                start_data_x = df_age_bucket.iloc[0, :]['age']
                start_data_y = df_age_bucket.iloc[0, :]['take']
                est_start_y = test_values[c]

                # # Get end points
                if c < len(campaign_obj[CampaignKeys.TIMES]) -1:
                    est_end_y = test_values[c + 1]
                else:
                    est_end_y = test_values[-1]
                end_data_x = max_age
                end_data_y = linear_fit[0] * (end_data_x - start_data_x) + start_data_y

                fit_segment = [start_data_x, end_data_x], [start_data_y, end_data_y]
                fit_data_segments.append(fit_segment)

                est_segment = [start_data_x, end_data_x], [est_start_y, est_end_y]
                est_data_segments.append(est_segment)

                # # calculate error estimation
                estimated_probability = linear_fit[0] * data_age + linear_fit[1]
                SumSquaresXX = np.sum((data_age - np.mean(data_age)) * (data_age - np.mean(data_age)))
                SumSquaresError = np.sum(
                    (data_take - estimated_probability) * (data_take - estimated_probability)) / len(data_age)
                if debug:
                    outfile.write(f"min_age_idx: {min_age_idx} \t max_age: {max_age}\n")
                    outfile.write(f"SumSquaresXX: {SumSquaresXX} \t SumSquaresError: {SumSquaresError}\n")
                std_err = np.sqrt(SumSquaresError / SumSquaresXX)
                slopes_std_err.append(std_err)

                is_within_two_errors = True if std_err == 0 else ((linear_fit[0] - slopes[c]) < 2 * std_err)
                slope_message = \
                    slope_template.format(slopes[c], linear_fit[0], std_err, is_within_two_errors)

                if debug:
                    print("linfit: {0}".format(linear_fit[0]))
                    print(slope_message)

                if is_within_two_errors:
                    success = True
                    result_messages.append(success_template.format(slope_message))
                else:
                    result_messages.append(fail_template.format(slope_message))
        y_lows = []
        y_highs = []
        for i in range(0, len(actual_slopes)):
            xval = campaign_obj[CampaignKeys.TIMES][i]
            yval = campaign_obj[CampaignKeys.TIMES][i] * actual_slopes[i] + y_ints[i]
            y_high = yval + slopes_std_err[i]
            y_low = yval - slopes_std_err[i]
            y_highs.append(y_high)
            y_lows.append(y_low)
            x_list.append(xval)
            y_list.append(yval)
        if debug:
            print(x_list, y_list)

        plot_measles_vaccine_chart(plotdf=outputdf,
                                   test_values=test_values,
                                   campaign_times=campaign_obj[CampaignKeys.TIMES],
                                   x_list=x_list,
                                   y_list=y_list,
                                   y_lows=y_lows,
                                   y_highs=y_highs)

        for message in result_messages:
            if message.startswith("BAD"):
                success = False  # One failure fails the test
            outfile.write(message)
        outfile.write(sft.format_success_msg(success))
    if debug:
        print("SUMMARY: Success={0}\n".format(success))
    return success


def plot_measles_vaccine_chart(plotdf, test_values, campaign_times,
                               x_list, y_list, y_lows, y_highs):
    fig = plt.figure()
    plt.scatter(plotdf['age'], plotdf['take'], s=20, alpha=0.02, lw=0)
    high_values = [0.05 * h + h for h in test_values]
    low_values = [h - 0.05 * h for h in test_values]
    plt.plot(campaign_times, test_values, 'r')
    plt.fill_between(campaign_times, low_values, high_values, facecolor='#FFAFAF')
    plt.plot(x_list, y_list, 'b')
    plt.fill_between(x_list, y_lows, y_highs, facecolor='#A4EFF5')
    plt.xlabel('age')
    plt.ylabel('take')
    plt.title('Age-Dependent Vaccine Age vs. Take')
    plt.legend(['Expected', 'Actual'])
    fig.savefig(str('plot_data') + '.png')
    if sft.check_for_plotting():
        plt.show()
    plt.close(fig)
