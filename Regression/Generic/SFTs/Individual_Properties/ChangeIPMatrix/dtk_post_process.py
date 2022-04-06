import dtk_test.dtk_sft as sft
import dtk_test.dtk_Individual_Property_Support as d_ips

import json
"""
    Initially, the TransmissionMatrix is set to transmit nothing to nobody, which is why we expect no new infections
    outside of the outbreak in group01. At time 45, the matrix is replaced with ChangeIPMatrix intervention to transmit
    from group01 to group02 and amongst group02. This means that at the second outbreak at step 47, we should be seeing 
    new infections in group01 just on step 47 and in group02 at step 47 and thereafter. 
    Test assumes no config or campaign parameters are changing. 
"""


def create_report_file(output_folder, json_report_name, sft_report_filename=sft.sft_output_filename, debug=False):
    with open(sft_report_filename, 'w') as report:
        success = True
        new_infections_group01 = "New Infections:InterventionStatus:group01"
        new_infections_group02 = "New Infections:InterventionStatus:group02"
        property_dict = d_ips.channels_from_jsonreport(channels_list=[new_infections_group01,
                                                                      new_infections_group02],
                                                       report_folder=output_folder,
                                                       json_report_name=json_report_name,
                                                       debug=debug)

        group01 = property_dict[new_infections_group01]
        group02 = property_dict[new_infections_group02]
        # check that only new infections are in group01 are on day 0 and day 47 of outbreaks
        for day, inf in enumerate(group01):
            if day != 0 and day != 47:
                if inf != 0:
                    success = False
                    report.write(f"BAD: Channel {new_infections_group01} day {day} has {inf} new infections and "
                                 f"it shouldn't. Please check. \n")

        # check that new infections in group02 start at day 48 and no earlier
        for day, inf in enumerate(group02):
            if day < 47 and inf != 0:
                success = False
                report.write(f"BAD: Channel {new_infections_group02} day {day} has {inf} "
                             f"new infections and it shouldn't. Please check. \n")
            if day >= 47 and inf == 0:
                # incubation period is 0, we should see new infections the same day
                success = False
                report.write(f"BAD: Channel {new_infections_group02} day {day} has {inf} "
                             f"new infections and there should be some. Please check. \n")

        report.write(sft.format_success_msg(success))


def application(output_folder="output", reporter_filename="PropertyReport.json",
                report_name=sft.sft_output_filename, debug=False):
    """

    Args:
        output_folder: (output)
        reporter_filename: output file from ReportStrainTracking.dll (ReportStrainTracking.json)
        report_name: report file to generate (scientific_feature_report.txt)
        debug: write lots of files as DEBUG_* for further investigation

    Returns:

    """
    if debug:
        print("output_folder: {0}".format(output_folder))
        print("reporter_filename: {0}".format(reporter_filename))
        print("report_name: {0}".format(report_name))
        print("debug: {0}".format(debug))

    sft.wait_for_done()
    create_report_file(output_folder, reporter_filename, report_name, debug)


if __name__ == "__main__":
    import argparse

    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output', default="output", help="Folder to load outputs from (output)")
    p.add_argument('-c', '--configname', default="config.json", help="config filename (config.json")
    p.add_argument('-r', '--reportname', default="scientific_feature_report.txt",
                   help="report filename ({0})".format("scientific_feature_report.txt"))
    p.add_argument('-s', '--stdout', default='test.txt', help='Standard Out filename (test.txt)')
    p.add_argument('-d', '--debug', action='store_true', help='turns on debugging')
    args = p.parse_args()

    application(output_folder=args.output, report_name=args.reportname, debug=args.debug)
