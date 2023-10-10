#!/usr/bin/python
import math
import json
import sqlite3
import pandas as pd
import numpy as np
from sys import platform
if platform == "linux" or platform == "linux2":
    import matplotlib
    print('Linux OS. Using non-interactive Agg backend')
    matplotlib.use('Agg')
import matplotlib.pyplot as plt

import dtk_test.dtk_sft as sft
from dtk_test.dtk_InsetChart import InsetChart

"""

Support library for RouteAwareSimpleVaccine SFTs
"""


class Columns:
    misc = "MISC"
    notes = "NOTES"
    mcw = "MCW"
    mcw = "AGE"
    individual = "INDIVIDUAL"
    node = "NODE"
    event = "EVENT"
    time = "SIM_TIME"


class Route:
    environment = "Environment"
    contact = "Contact"


class Group:
    environmental = "Environmental_Blocking"
    contact = "Contact_Blocking"
    control = "Control"


class Transmission_Route:
    environmental = "ENVIRONMENTAL"
    contact = "CONTACT"
    outbreak = "OUTBREAK"


class Constant:
    infected = "Infected"
    seed = "Seed"
    control = "Control"
    vaccine = "Vaccine"
    normalized = "Normalized"
    expected = "Expected"
    disease_deaths = "Disease Deaths"
    contagion = "Contagion"


def convert_sqlite_to_csv(db_file, csv_filename='database.csv'):
    with sqlite3.connect(db_file, isolation_level=None, detect_types=sqlite3.PARSE_COLNAMES) as conn:
        db_df = pd.read_sql_query("SELECT * FROM SIM_EVENTS", conn)
    db_df.to_csv(csv_filename, index=False)
    return db_df


def plot_test_data(df, plot_name=Constant.infected):
    fig = plt.figure()
    ax = fig.add_axes([0.12, 0.15, 0.76, 0.76])
    ax.set_title(plot_name)
    df.plot(ax=ax)
    ax.legend(loc=0)

    fig.savefig(plot_name + '.png')
    if sft.check_for_plotting():
        plt.show()
    plt.close(fig)



# Used in Acquisition_Effect_Contact and Acquisition_Effect_Environmental SFTs
def create_report_file_acquisition_effect(df, property_report_name, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing with {property_report_name}.\n")
        # Skip time 0 since there may be no infection caused by contact/environmental transmission in control group.
        df = df.drop([0, 1])
        df_infected = df.filter(regex=Constant.infected)

        # Get control and seed groups data and drop them from dataframe
        control_column = [col for col in df_infected.columns if Constant.control in col][0]
        control_data = df_infected[control_column]
        seed_column = [col for col in df_infected.columns if Constant.seed in col][0]
        # df_infected = df_infected.drop(columns=[control_column, seed_column])
        df_infected = df_infected.drop([control_column, seed_column], axis='columns')

        for column in df_infected.columns:
            # Normalized by Control group
            df_infected[column+":"+ Constant.normalized] = df_infected[column] / control_data
            ip_group = column.split(":")[-1]
            vaccine_effect = float(ip_group.split("_")[-1])
            if vaccine_effect == 1:
                if any(df_infected[column] != 0.0):
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the {Constant.infected} channel should be "
                                          f"all 0. Found non-zero value.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the {Constant.infected} channel contains "
                                          f"only zero.\n")
            else:
                expected_infected = (1 - vaccine_effect)
                mean_infected = df_infected[column+":"+ Constant.normalized].mean()
                if math.fabs(mean_infected - expected_infected) > 0.1:
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the expected infected portion after "
                                          f"normalization should be about {expected_infected}, got an average of "
                                          f"{mean_infected}.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the expected infected portion after "
                                          f"normalization should be about {expected_infected}, got an average of "
                                          f"{mean_infected}.\n")
        sft_report_file.write(sft.format_success_msg(success))

    df_plot = df_infected.filter(regex=Constant.normalized)
    plot_test_data(df_plot)

    return success


# Used in Transmission_Effect_Contact and Transmission_Effect_Environmental SFTs
def create_report_file_transmission_effect(df, property_report_name, config_filename, report_name, env_targ):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing with {property_report_name}.\n")
        # Skip time 0 since there is no infection caused by transmission
        df = df.drop(0)
        df_contagion  = df.filter(regex=Constant.contagion)
        if(env_targ):
            df_croute = df_contagion.filter(regex=Route.environment)
        else:
            df_croute = df_contagion.filter(regex=Route.contact)

        for column in df_croute.columns:
            ip_group = column.split(":")[-1]
            if Constant.seed not in ip_group:
                # Normalize by the infected individuals in the seed group
                ref_column = [col for col in df_croute.columns if (Constant.control in col) and (Constant.seed not in col)][0]
                df_croute[column+":"+ Constant.normalized] = df_croute[column] / df_croute[ref_column]
        df_normalized = df_croute.filter(regex=Constant.normalized)

        # Get control group data and drop it from dataframe
        control_column = [col for col in df_normalized.columns if Constant.control in col][0]
        control_data = df_normalized[control_column]
        # df_normalized = df_normalized.drop(columns=[control_column])
        df_normalized = df_normalized.drop([control_column], axis='columns')
        for column in df_normalized.columns:
            # Normalized by Control group
            df_normalized[column] = df_normalized[column] / control_data
            ip_group = column.split(":")[-2]
            vaccine_effect = float(ip_group.split("_")[-1])
            if vaccine_effect == 1:
                if any(df_normalized[column] != 0.0):
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the {Constant.contagion} channel should be "
                                          f"all 0. Found non-zero value.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the {Constant.contagion} channel contains "
                                          f"only zero.\n")
            else:
                expected_infected = (1 - vaccine_effect)
                mean_infected = df_normalized[column].mean()
                if math.fabs(mean_infected - expected_infected) > 0.1:
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the expected contagion after "
                                          f"normalization should be about {expected_infected}, got an average of "
                                          f"{mean_infected}.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the expected contagion after "
                                          f"normalization should be about {expected_infected}, got an average of "
                                          f"{mean_infected}.\n")
        sft_report_file.write(sft.format_success_msg(success))

    df_plot = df_normalized[[col for col in df_normalized.columns if Constant.vaccine in col]]
    plot_test_data(df_plot, plot_name=Constant.contagion)

    return success

# Used in Mortality_Effect_Contact and Mortality_Effect_Environmental SFTs
def create_report_file_mortality_effect(df, property_report_name, config_filename, report_name):
    with open(report_name, "w") as sft_report_file :
        config_name = sft.get_config_name(config_filename)
        sft_report_file.write("Config_name = {}\n".format(config_name))
        success = True

        sft_report_file.write(f"Testing with {property_report_name}.\n")
        # Skip time 0 since there may be no disease death caused by contact/environmental transmission in control group.
        df = df.drop([0, 1])
        # initial_population =
        df_disease_deaths = df.filter(regex=Constant.disease_deaths)

        control_column = [col for col in df_disease_deaths.columns if Constant.control in col][0]
        control_data = df_disease_deaths[control_column]
        seed_column = [col for col in df_disease_deaths.columns if Constant.seed in col][0]
        df_disease_deaths = df_disease_deaths.drop([control_column, seed_column], axis='columns')

        for column in df_disease_deaths.columns:

            # Normalized by Control group
            df_disease_deaths[column+":"+ Constant.normalized] = df_disease_deaths[column] / control_data
            ip_group = column.split(":")[-1]
            vaccine_effect = float(ip_group.split("_")[-1])
            if vaccine_effect == 1:
                if any(df_disease_deaths[column] != 0.0):
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the {Constant.disease_deaths} channel should be "
                                          f"all 0. Found non-zero value.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the {Constant.disease_deaths} channel contains "
                                          f"only zero.\n")
            else:
                expected_death = (1 - vaccine_effect)
                mean_death = df_disease_deaths[column+":"+ Constant.normalized].mean()
                if math.fabs(mean_death - expected_death) > 0.1:
                    success = False
                    sft_report_file.write(f"\tBAD: for ip group {ip_group}, the expected disease deaths portion after "
                                          f"normalization should be about {expected_death}, got an average of "
                                          f"{mean_death}.\n")
                else:
                    sft_report_file.write(f"\tGOOD: for ip group {ip_group}, the expected disease deaths portion after "
                                          f"normalization should be about {expected_death}, got an average of "
                                          f"{mean_death}.\n")

        df_plot = df_disease_deaths.filter(regex=Constant.normalized)
        plot_test_data(df_plot, plot_name=Constant.disease_deaths)

        sft_report_file.write(sft.format_success_msg(success))

    return success
