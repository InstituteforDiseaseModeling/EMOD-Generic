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
            day = 0 # Hard coding for grins
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
                report_file.write("HUH: Not testing for uniformity, this is not a random (-1)" +
                                  f"genome. Clade: {clade} Geneome: {genome}.\n")
        # Get weights for genomes in report
        # NOTE: We checked earlier, there should be exactly two
        day_df = report_df.loc[(report_df.time==0)]
        day_df_0 = day_df[day_df.clade==0]
        day_df_1 = day_df[day_df.clade==1]
        day_df_2 = day_df[day_df.clade==2]
        clade_0_initial_infections = day_df_0['tot_inf'].sum()
        clade_1_initial_infections = day_df_1['tot_inf'].sum()
        clade_2_initial_infections = day_df_2['tot_inf'].sum()
        initial_infections = clade_0_initial_infections +\
                             clade_1_initial_infections +\
                             clade_2_initial_infections
        weight_0 = campaign_dictionary['0_-1']
        weight_1 = campaign_dictionary['1_2']
        weight_2 = campaign_dictionary['2_3']
        total_weight = weight_0 + weight_1 + weight_2

        prob_0 = weight_0 / total_weight
        prob_1 = weight_1 / total_weight
        prob_2 = weight_2 / total_weight

        report_file.write("Trying binomial test for clade 0\n")
        sft.test_binomial_95ci(num_success=clade_0_initial_infections,
                               num_trials=initial_infections,
                               prob=prob_0,
                               report_file=report_file,
                               category="Clade 0 infections")

        report_file.write("Trying binomial test for clade 1\n")
        sft.test_binomial_95ci(num_success=clade_1_initial_infections,
                               num_trials=initial_infections,
                               prob=prob_1,
                               report_file=report_file,
                               category="Clade 1 infections")

        report_file.write("Trying binomial test for clade 2\n")
        sft.test_binomial_95ci(num_success=clade_2_initial_infections,
                               num_trials=initial_infections,
                               prob=prob_2,
                               report_file=report_file,
                               category="Clade 2 infections")

        # Compare to actual distribution
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

    if cfg_object[d_sis.KEY_num_clades] != 3:
        raise ValueError("This test is meant to have exactly two clades. Check config Number_of_Clades.")


    reporter_df = d_sis.create_dataframe_from_report(output_folder, reporter_filename, debug=debug)
    create_report_file(report_df=reporter_df, config_object=cfg_object, campaign_dictionary=inital_strain_dict, debug=debug)


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
