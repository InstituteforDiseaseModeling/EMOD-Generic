import dtk_sft as sft
import dtk_Strain_Identity_Support as d_sis


def application(output_folder="output", config_filename="config.json",
                stdout_filename="test.txt", reporter_filename="ReportStrainTracking.json",
                report_name="scientific_feature_report.txt", debug=False):
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

    strain_df = d_sis.process_stdout_file(stdout_filename=stdout_filename,
                                          campaign_dictionary=camp_dict,
                                          start_time=cfg_object[d_sis.KEY_sim_start_time],
                                          simulation_duration=cfg_object[d_sis.KEY_simulation_duration],
                                          sim_timestep=cfg_object[d_sis.KEY_simulation_timestep],
                                          debug=debug)

    reporter_df = d_sis.create_dataframe_from_report(output_folder, reporter_filename, debug=debug)
    messages = d_sis.compare_dataframes(console_df=strain_df, reporter_df=reporter_df, debug=debug)
    d_sis.create_report_file(strain_dataframe=strain_df,
                             campaign_dictionary=camp_dict, report_name=report_name,
                             compare_messages=messages)

    d_sis.plot_strain_data(strain_df, camp_dict)


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
