#!/usr/bin/python

import dtk_test.dtk_sft as dtk_sft
import dtk_test.dtk_Generic_InfectionRate_Mod_Additive_Multiplicative_Support as IRMAM_support


def application( output_folder="output", stdout_filename="test.txt", insetchart_name="InsetChart.json",
                 config_filename="config.json", campaign_filename="campaign.json",
                 report_name=dtk_sft.sft_output_filename,
                 debug=False):
    if debug:
        print( "output_folder: " + output_folder )
        print( "stdout_filename: " + stdout_filename+ "\n" )
        print( "insetchart_name: " + insetchart_name+ "\n" )
        print( "config_filename: " + config_filename + "\n" )
        print( "campaign_filename: " + campaign_filename + "\n" )
        print( "report_name: " + report_name + "\n" )
        print( "debug: " + str(debug) + "\n" )

    dtk_sft.wait_for_done(stdout_filename)
    param_obj = IRMAM_support.load_emod_parameters(config_filename, debug)
    campaign_obj = IRMAM_support.load_campaign_file(campaign_filename, debug)
    demo_obj = IRMAM_support.load_demo_overlay_file(param_obj[IRMAM_support.Config.demo_filenames], debug)
    stdout_df = IRMAM_support.parse_stdout_file(stdout_filename, param_obj[IRMAM_support.Config.simulation_timestep],debug)
    insetchart_df = IRMAM_support.parse_insetchart_json(insetchart_name, output_folder, debug)
    enabled_param = [IRMAM_support.Config.enable_infectivity_scaling,
                     IRMAM_support.Config.enable_infectivity_reservoir,
                     IRMAM_support.Config.enable_infectivity_exp]
    IRMAM_support.create_report_file(enabled_param, param_obj, campaign_obj, demo_obj, stdout_df, insetchart_df, report_name, debug)


if __name__ == "__main__":
    # execute only if run as a script
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    parser.add_argument('-s', '--stdout', default="test.txt", help="Name of stdoutfile to parse (test.txt)")
    parser.add_argument('-j', '--jsonreport', default="InsetChart.json", help="Json report to load (InsetChart.json)")
    parser.add_argument('-c', '--config', default="config.json", help="Config name to load (config.json)")
    parser.add_argument('-C', '--campaign', default="campaign.json", help="campaign name to load (campaign.json)")
    parser.add_argument('-r', '--reportname', default=dtk_sft.sft_output_filename, help="Report file to generate")
    parser.add_argument('-d', '--debug', help="debug flag", action='store_true')
    args = parser.parse_args()

    application(output_folder=args.output, stdout_filename=args.stdout, insetchart_name=args.jsonreport,
                config_filename=args.config, campaign_filename=args.campaign,
                report_name=args.reportname, debug=args.debug)

