#!/usr/bin/python
import dtk_test.dtk_sft as sft
import json
import numpy as np
with open("config.json") as infile:
    run_number=json.load(infile)['parameters']['Run_Number']
np.random.seed(run_number)
import math
from dtk_test.dtk_General_Support import ConfigKeys, DemographicsKeys, InsetKeys

"""

This SFT is testing environmental feature 2.1 Environmental route,  2.2 Infections: Calculating total contagion,
2.5.5 Contagion normalization and 2.5.6 Reporting: PropertyReportEnvironmental:

"2.1 Environmental route. 
    Contagion is transmitted over a second route, called environmental, just like it is over the 
    contact route in Generic. Unlike for the contact route, the environmental route contagion doesn't necessarily decay to 
    zero after each timestep.

Node_Contagion_Decay_Rate. 
    The size of the environmental contagion pool for a node shall equal the previous day times 1 
    minus the Node Decay Rate, plus any new shedding for that timestep:

    Previous_pool_size * (1 - Node_Decay_Rate) + New_contagion"

"2.2 Infections
Calculating total contagion. 
    The model returns the sum across all antigens and all groups for the normalized contagion population. For example, 
    if an individual is exposed to N units of contagion on the given route, then the reported value is the sum of that 
    exposure across all possible target groups."
    
"2.5.5 Contagion normalization

    GROUP: The environmental route will be normalized by group population."
    POPULATION(New in DTK-COVID milestone): The environmental route will be normalized by total population.

"2.5.6 Reporting
        InsetChart
            InsetChart inherits all channels from GENERIC_SIM. In addition, it will contain "New Infections By Route 
            (ENVIRONMENT)," "New Infections By Route (CONTACT)," ""Contact Contagion Population," and 
            "Environmental Contagion Population.
    	PropertyReportEnvironmental
	        This report shall be produced if Enable_Property_Output is set to 1. It shall include channels from the base 
	        class (Generic), broken down by HINT group, as well as the following channels: New Infections By Route 
	        (contact), New Infections By Route (environment), Contagion (contact) and Contagion (environment)."

This test is testing the above statement with Multi Routes HINT feature.

Test data is loaded from InsetChart.json and PropertyReportEnvironmental.json. 

Suggested sweep parameters: Node_Contagion_Decay_Rate, Base_Infectivity_Constant

"""

channels = [InsetKeys.ChannelsKeys.Infected,
            "New Infections By Route (ENVIRONMENT)",
            "New Infections By Route (CONTACT)",
            "Contagion (Environment)",
            "Contagion (Contact)",
            InsetKeys.ChannelsKeys.Statistical_Population]

inset_channels = [InsetKeys.ChannelsKeys.Contact_Contagion_Population,
                  InsetKeys.ChannelsKeys.Environmental_Contagion_Population,
                  InsetKeys.ChannelsKeys.Infected,
                  InsetKeys.ChannelsKeys.Statistical_Population]

config_keys = [ConfigKeys.Config_Name, 
               ConfigKeys.Simulation_Timestep,
               ConfigKeys.Simulation_Duration,
               ConfigKeys.Base_Infectivity_Constant,
               ConfigKeys.Run_Number,
               ConfigKeys.Demographics_Filenames,
               ConfigKeys.Enable_Heterogeneous_Intranode_Transmission,
               ConfigKeys.Node_Contagion_Decay_Rate,
               ConfigKeys.Environmental_Normalization]

routes = [
    "environmental",
    "contact"
]

Environmental_Normalization_Options = ["GROUP", "POPULATION"]

def create_report_file(param_obj, property_df, property_obj, inset_chart_obj, insetchart_name, property_report_name,
                       report_name, debug):
    with open(report_name, "w") as outfile:
        config_name = param_obj[ConfigKeys.Config_Name]
        decay_rate = param_obj[ConfigKeys.Node_Contagion_Decay_Rate]
        environmental_normalization = param_obj[ConfigKeys.Environmental_Normalization]
        outfile.write("Config_name = {}\n".format(config_name))
        outfile.write("{0} = {1} {2} = {3} {4} = {5}\n".format(
            ConfigKeys.Node_Contagion_Decay_Rate, decay_rate,
            ConfigKeys.Run_Number, param_obj[ConfigKeys.Run_Number],
            ConfigKeys.Environmental_Normalization, environmental_normalization))

        success = True

        outfile.write("Testing: Testing contagion and decay in each groups for every time step:\n")
        duration = param_obj[ConfigKeys.Simulation_Duration]
        transmission_matrix_e = property_obj[DemographicsKeys.PropertyKeys.TransmissionMatrix][routes[0]]
        base_infectivity = param_obj[ConfigKeys.Base_Infectivity_Constant]
        groups = property_obj[DemographicsKeys.PropertyKeys.Values]
        contagion = []
        for group in groups:
            outfile.write("  Testing group: {}.\n".format(group))
            contagion_list_e = []
            # get list of column names that contains the group that we are testing
            cols = [c for c in property_df.columns if group in c]
            # get the list of the other group
            other_group = [x for x in groups if x is not group][0]
            # get all the column names that contains the other group
            cols_others = [c for c in property_df.columns if group not in c]

            # test data for test group
            infected = property_df[[c for c in cols if channels[0] in c]]
            contagion_e = property_df[[c for c in cols if channels[3] in c]]
            population = property_df[[c for c in cols if channels[5] in c]]


            # test data for the other group
            infected_other = property_df[[c for c in cols_others if channels[0] in c]]
            population_other = property_df[[c for c in cols_others if channels[5] in c]]

            pre_contagion = 0
            for t in range(duration - 1):
                if t == 0:
                    contagion_list_e.append([contagion_e.iloc[t][0], 0])
                    if contagion_e.iloc[t][0]:
                        success = False
                        outfile.write("    BAD: at time step {0}, for group {1} route {2}, the contagion is {3}, "
                                      "expected {4}.\n".format(t, group, routes[0], contagion_e.iloc[t][0],
                                                               0
                                                               ))

                normalization_population = population.iloc[t][0] if \
                    environmental_normalization == Environmental_Normalization_Options[0] else \
                    (population.iloc[t][0] + population_other.iloc[t][0]) # [0] is normalization by group
                infectivity = base_infectivity * infected.iloc[t][0] / normalization_population
                infectivity_other = base_infectivity * infected_other.iloc[t][0] / normalization_population

                # calculate contagion for environmental group
                new_contagion = infectivity * transmission_matrix_e[groups.index(group)][
                    groups.index(group)] + infectivity_other * transmission_matrix_e[groups.index(other_group)][
                    groups.index(group)]

                current_contagion = pre_contagion * (1 - decay_rate) + new_contagion

                # for next time step
                pre_contagion = current_contagion

                # get contagion from property report
                actual_contagion_e = contagion_e.iloc[t + 1][0]

                contagion_list_e.append([actual_contagion_e, current_contagion])

                if math.fabs(current_contagion - actual_contagion_e) > 5e-2 * current_contagion:
                    success = False
                    outfile.write("    BAD: at time step {0}, for group {1} route {2}, the contagion is {3}, "
                                  "expected {4}.\n".format(t + 1, group, routes[0], actual_contagion_e,
                                                           current_contagion
                    ))


            contagion.append(contagion_list_e)
            # plot actual and expected values for contagion
            sft.plot_data(np.array(contagion_list_e)[:, 0], np.array(contagion_list_e)[:, 1],
                              label1=property_report_name, label2="calculated contagion",
                              title="contagion for group {0}\n route {1}".format(group, routes[0]),
                              xlabel='day', ylabel='contagion', category="contagion_{0}_{1}".format(group, routes[0]),
                              line=True, alpha=0.5, overlap=True)

        outfile.write("Checking if contagion data from insetchart matches sum of group contagion from property report"
                      " for every time step:\n")
        total_contagion_list = []
        for t in range(duration - 1):
            total_contagion = 0
            for contagion_list_e in contagion:
                total_contagion += contagion_list_e[t][0]
            total_contagion_list.append(total_contagion)
            insetchart_contagion = inset_chart_obj[InsetKeys.ChannelsKeys.Environmental_Contagion_Population][t]
            if math.fabs(total_contagion - insetchart_contagion) > 5e-2 * total_contagion:
                success = False
                outfile.write("    BAD: at time step {0}, for route {1}, the total contagion from {2} is {3}, "
                              "expected {4}(sum of group contagion in {5}).\n".format(
                                    t, routes[0], insetchart_name, insetchart_contagion,
                                    total_contagion, property_report_name
                                                       ))
        sft.plot_data(inset_chart_obj[InsetKeys.ChannelsKeys.Environmental_Contagion_Population],
                          total_contagion_list,
                          label1=insetchart_name, label2="sum calculated from {}".format(property_report_name),
                          title=InsetKeys.ChannelsKeys.Environmental_Contagion_Population,
                          xlabel='day', ylabel='contagion',
                          category=InsetKeys.ChannelsKeys.Environmental_Contagion_Population,
                          line=True, alpha=0.5, overlap=True)

        outfile.write(sft.format_success_msg(success))
    if debug:
        print(sft.format_success_msg(success))
    return success

