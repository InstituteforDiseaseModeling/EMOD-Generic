import dtk_test.dtk_Individual_Property_Support as d_ips
import dtk_test.dtk_sft as sft

def application(output_folder="output", config_filename="config.json",
                report_name=sft.sft_output_filename, inset_name="InsetChart.json",
                propertyreport_filename="PropertyReport.json",
                debug=False):
    # Load config file to get
    # # demographics filename
    # # enable_property_reporting
    # # config name
    config_object = d_ips.load_config_file(config_filename, debug=debug)
    overlay_filename = config_object[d_ips.ConfigKeys.DEMOGRAPHICS_FILENAMES][-1]

    # start report file
    sft.start_report_file(output_filename=report_name,
                          config_name=config_object[d_ips.ConfigKeys.CONFIG_NAME])

    # Load demographics_filenames[-1] to get property array
    properties_array = d_ips.get_properties_from_demographics(overlay_filename,
                                                              debug=debug)

    additive_channels = [
        d_ips.JsonReport.Channels.STATISTICAL_POPULATION,
        d_ips.JsonReport.Channels.NEW_INFECTIONS,
        d_ips.JsonReport.Channels.DEATHS
    ]
    # Load inset chart
    # Load inset chart channels that are additive and in property report
    # # Disease Deaths, New Infections, Statistical Population
    channels_dict = d_ips.channels_from_jsonreport(channels_list=additive_channels,
                                             report_folder=output_folder,
                                             json_report_name=inset_name,
                                             debug=debug)

    # Build property report channel names
    prop_channels = d_ips.get_property_channels(additive_channels,
                                                properties_array,
                                                debug=debug)

    # Load property report channels into dictionary
    property_dict = d_ips.channels_from_jsonreport(channels_list=prop_channels,
                                                   report_folder=output_folder,
                                                   json_report_name=propertyreport_filename,
                                                   debug=debug)

    # Combines two dictionaries, with 'second one wins' logic
    all_channels = {**channels_dict, **property_dict}
    combined_df = d_ips.dataframe_from_channeldict(all_channels,
                                                   debug=debug)

    # VERIFY: Statistical population sliced as per demographics
    messages_array = []
    success = True
    for channel in additive_channels:
        if not d_ips.verify_channel_sum(combined_df, channel, messages_array):
            success = False

    with open(report_name, 'a') as outfile:
        for p in properties_array:
            if debug:
                print(f"checking property: {p}\n")
            local_success, local_messages = d_ips.verify_channel_split(combined_df, additive_channels[0],
                                                                       p, messages_array, outfile, debug=debug)
            if debug:
                print(f"Got local messages: {local_messages}\n")
                print("Messages array now: \n")
                for m in messages_array:
                    print(f"\t {m}\n")
            if not local_success:
                success = False
        for m in messages_array:
            outfile.write(m + '\n')
            if "BAD" in m:
                success = False
        outfile.write(sft.format_success_msg(success))
    pass


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