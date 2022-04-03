import dtk_sft as sft
import dtk_Strain_Identity_Support as d_sis
from dtk_General_Support import ConfigKeys


def application(output_folder="output", config_filename="config.json",
                reporter_filename="ReportStrainTracking.json",
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
    debug = True # While I'm building the test
    test = d_sis.StrainIdentityTransmissionTest(output_folder=output_folder,
                                                config_filename=config_filename,
                                                reporter_filename=reporter_filename,
                                                sft_report_name=report_name,
                                                debug=debug)

    cfg_object = d_sis.load_config_file(config_filename=config_filename, debug=debug)
    sft.np.random.seed(cfg_object[d_sis.KEY_run_number])

    proportions_object =  test.parse_campaign_file()
    incubation_time = test.params[ConfigKeys.Incubation_Period_Constant] + 1

    reporter_df = d_sis.create_dataframe_from_report(output_folder, reporter_filename, debug=True)

    new_inf_day = test.outbreak_day + incubation_time

    messages = test.test_expectations_for_day(proportions_object, reporter_df, day_in_question=new_inf_day)

    success = True
    with open(test._sft_report_name, 'a') as outfile:
        for m in messages:
            if(m.startswith("BAD")):
                success = False
            outfile.write(m)
        outfile.write(sft.format_success_msg(success))


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
                debug=args.debug)
