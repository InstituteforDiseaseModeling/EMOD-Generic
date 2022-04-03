import dtk_test.dtk_Individual_Property_Support as d_ips
import dtk_test.dtk_sft as sft
import matplotlib.pyplot as plt


def application(output_folder="output", config_filename="config.json",
                inset_name="InsetChart.json", propertyreport_filename="PropertyReport.json",
                report_name=sft.sft_output_filename,
                debug=False):
    # load config file and campaign file
    config_object = d_ips.load_config_file(config_filename, debug=debug)
    sft.start_report_file(report_name, config_object[d_ips.ConfigKeys.CONFIG_NAME])

    overlay_filename = config_object[d_ips.ConfigKeys.DEMOGRAPHICS_FILENAMES][-1]

    prop_array = d_ips.get_properties_from_demographics(overlay_filename, debug=debug)

    inset_channels = [
        d_ips.JsonReport.Channels.STATISTICAL_POPULATION,
        d_ips.JsonReport.Channels.CAMPAIGN_COST
    ]

    # Create dataframe from Inset stat_pop, campaign cost
    channels_dict = d_ips.channels_from_jsonreport(channels_list=inset_channels,
                                                   report_folder=output_folder,
                                                   json_report_name=inset_name,
                                                   debug=debug)

    channels_df = d_ips.dataframe_from_channeldict(channel_dictionary=channels_dict,
                                                   debug=debug)

    # Add columns to dataframe from PropReport stat_pop

    # Create columns for Property stat_pop rollup channels
    # # For example: Stat_Pop:Risk:High as a rollup of
    # #  Stat_Pop:Risk:High:Access:Easy and Stat_Pop:Risk:High:Access:Hard

    # HARD VERSION
    # Plot_Start_Date = 1
    # Plot 1: Campaign Cost against Rollup channels for first N + Plot_Start_Date days
    # # Where N is number of property names

    # For property_name in property names # Looks like campaign will need "blank" days
    # In between each of these "sweeps"
    # # Plot Campaign Cost vs Property stat_pop values of property_name
    # # from Plot_Start_Date to N + Plot_Start_Date where
    # # N is number of values for property_name
    # # Not sure what else here

    # EASY VERSION
    property_channels = d_ips.get_property_channels(inset_channel_names=[d_ips.JsonReport.Channels.STATISTICAL_POPULATION],
                                                    properties_array=prop_array,
                                                    debug=debug)

    property_dict = d_ips.channels_from_jsonreport(channels_list=property_channels,
                                                   report_folder=output_folder,
                                                   json_report_name=propertyreport_filename,
                                                   debug=debug)

    property_df = d_ips.dataframe_from_channeldict(property_dict, debug=debug)

    sft_df = d_ips.dataframe_join_on_day(channels_df, property_df, debug=debug)
    sft.plot_dataframe_line(sft_df)
    # sft_df.plot.line()
    #
    # plt.show()

    # now add sum columns for statistical population
    # first, add rollup columns. In this case, stat_pop for Risk:High and Accessibility:Hard
    # irrespective of the other properties.
    inset_channel_name = d_ips.JsonReport.Channels.STATISTICAL_POPULATION
    for property in prop_array:
        property_name = property[d_ips.DemographicsKeys.PropertyKeys.PROPERTY_NAME]
        for value in property[d_ips.DemographicsKeys.PropertyKeys.PROPERTY_VALUES]:
            property_value_pair = "{0}:{1}".format(property_name, value)
            addend_columns = []
            for col in sft_df.columns:
                if col.startswith(inset_channel_name) and property_value_pair in col:
                    addend_columns.append(col)
            sft_df["{0}:{1}".format(inset_channel_name,property_value_pair)] = sft_df[addend_columns].sum(axis=1)

    with open("DEBUG_dataframe_channelnames.txt","w") as channelnames_file:
        channelnames_file.write(str(sft_df.columns.values))

    # load up campaign file
    campaign_dictionary = d_ips.load_campaign_file(config_object[d_ips.ConfigKeys.CAMPAIGN_FILENAME])
    old_campaign_cost = sft_df['Campaign Cost'][0]
    success = False

    with open(report_name, 'a') as outfile:
        # verify that the first start_day is greater than the start_time + sim_timestep from config
        # Get all of the statistical population channels for comparison
        first_possible_start_day = config_object[d_ips.ConfigKeys.START_TIME] + \
            config_object[d_ips.ConfigKeys.TIMESTEP_SIZE]
        if config_object[d_ips.ConfigKeys.ENABLE_INTERVENTIONS] == 1:
            success = True
        for start_day in sorted(campaign_dictionary):
            print("Trying start day {0}\n".format(start_day))
            if len(campaign_dictionary[start_day]) > 1:
                success = False
                outfile.write("BAD: day {0} should have 1 intervention, has {1}".
                              format(start_day, len(campaign_dictionary[start_day])))
            if start_day < first_possible_start_day:
                success = False
                outfile.write("BAD: day {0} is too early in the sim. Check config.".format(start_day))
            event = campaign_dictionary[start_day][0] # there can be only one
            property_restrictions = \
                event[d_ips.CampaignKeys.EVENT_COORDINATOR_CONFIG_KEY][d_ips.CampaignKeys.PROPERTY_RESTRICTIONS]
            # loop through channnels to check each one for validity
            correct_channel = None
            while not(correct_channel):
                for c in sft_df.columns:
                    is_candidate = False
                    if c.startswith(inset_channel_name) and ':' in c: # Is the right channel
                        is_candidate = True
                        #print("Channel name: {0}\n".format(c))
                        prs = c[c.index(':')+1:].split(',')
                        #print("PRS from channel name: {0}\n".format(prs))
                        #print("Property restrictions from campaign file: {0}\n".format(property_restrictions))
                        if len(prs) != len(property_restrictions): # has the right number of restrictions
                            is_candidate = False
                        for pr in property_restrictions:
                            if pr not in c: # each restriction is in the list
                                is_candidate = False
                    if is_candidate:
                        correct_channel = c
                        print("correct channel {0} found for restrictions {1}\n".format(correct_channel, property_restrictions))
                        break
            expected_cost_increase = sft_df[correct_channel][start_day]
            actual_cost_increase = sft_df[d_ips.JsonReport.Channels.CAMPAIGN_COST][start_day] - old_campaign_cost
            old_campaign_cost = old_campaign_cost + actual_cost_increase
            cost_increase_message = " expected cost increase of {0} for prs {1} on start_day {2}, got cost increase {3}".format(
                    expected_cost_increase, property_restrictions, start_day, actual_cost_increase
                )
            if expected_cost_increase != actual_cost_increase:
                outfile.write("{0}{1}\n".format("BAD!", cost_increase_message))
            else:
                outfile.write("{0}{1}\n".format("GOOD!", cost_increase_message))
        outfile.write(sft.format_success_msg(success))


            # get populations from statpop property channels for that day
    # validate that there is only one event on each day... dictionary, maybe?

    # for each day with an event (again, dictionary)
    # get the properties targeted
    # from the dataframe, get the statistical population of that group
    # from the dataframe, get the campaign cost the day before and the day of the event
    # verify that the campaign cost increases by the statistical population of the targeted group


if __name__=="__main__":
    # execute only if run as a script
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    p.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    p.add_argument('-j', '--jsonreport', default="InsetChart.json", help="Json report to load (InsetChart.json)")
    p.add_argument('-p', '--propertyreport', default="PropertyReport.json", help="Property report to load (PropertyReport.json)")
    p.add_argument('-s', '--stdout', default="Test.txt", help="Name of stdoutfile to parse (Test.txt")
    p.add_argument('-r', '--reportname', default=sft.sft_output_filename, help="Report file to generate")
    p.add_argument('-d', '--debug', action='store_true', help="Turns on debugging")
    args = p.parse_args()

    application(output_folder=args.output, config_filename=args.config,
                report_name=args.reportname, inset_name=args.jsonreport,
                propertyreport_filename=args.propertyreport,
                debug=args.debug)

