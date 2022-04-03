import dtk_sft as sft
import dtk_Strain_Identity_Support as d_sis


def test_uniform_genome_distribution(reporter_df, max_genome, day_to_test=0, clade_to_test=0, report_file=None, debug=False):
    filtered_df = reporter_df.loc[(reporter_df.time==day_to_test) & (reporter_df.clade==clade_to_test)]
    if debug:
        filtered_df.to_csv(f"DEBUG_clade_{clade_to_test}_df.csv")
    if report_file:
        report_file.write(f"TEST: checking for uniform distribution of distributed infections from clade {clade_to_test} on day {day_to_test}.\n")
    # Better test: check that the first and second half weights are within a binomial approx of 1/2
    # And that quartiles 1, 4 vs quartiles 2, 3 are also close like that
    # success_ks = sft.test_uniform(dist=filtered_df['tot_inf'], p1=0,
    #                            p2=max_genome, report_file=report_file)
    success = sft.test_uniform(filtered_df['tot_inf'], report_file=report_file)
    show = sft.check_for_plotting()
    d_sis.plot_genome_histogram(filtered_df, clade=clade_to_test, show=show)
    return success


def create_report_file(report_df, config_object, campaign_dictionary, debug=False):
    # loop through campaign for randomized outbreaks
    with open(sft.sft_output_filename, 'w') as report_file:
        success = True
        for strain_id in campaign_dictionary:
            day = int(campaign_dictionary[strain_id])
            splits = strain_id.split('_')
            clade = int(splits[0])
            genome = int(splits[1])
            if genome == -1:
                highest_genome = 2**config_object[d_sis.KEY_l2NoGpC]
                looks_good = test_uniform_genome_distribution(
                    reporter_df=report_df, max_genome=highest_genome -1,
                    day_to_test=day,report_file=report_file,
                    clade_to_test=clade, debug=debug)
                if not looks_good:
                    success = False
            else: # This isn't a random genome
                report_file.write("BOO! Not sure what you're doing here, but we expected a random (-1)" +
                                  f"genome and got Clade: {clade} Geneome: {genome}.\n")
                success=False
        report_file.write(sft.format_success_msg(success))
    pass
    # test distribution

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

    if cfg_object[d_sis.KEY_enable_initial_prevalence]==1 and len(cfg_object[d_sis.KEY_demographics_filenames]) == 2:
        inital_strain_dict = d_sis.load_demographic_overlay(cfg_object[d_sis.KEY_demographics_filenames][1])

    camp_dict = d_sis.load_campaign_file(cfg_object[d_sis.KEY_campaign_filename], debug=debug)
    for inital_strain_id in inital_strain_dict:
        camp_dict[inital_strain_id] = cfg_object[d_sis.KEY_sim_start_time]

    reporter_df = d_sis.create_dataframe_from_report(output_folder, reporter_filename, debug=debug)
    create_report_file(report_df=reporter_df, config_object=cfg_object, campaign_dictionary=camp_dict, debug=debug)


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
