import os

import numpy as np
import scipy.stats as spsts
import matplotlib.pyplot as plt
import dtk_test.dtk_sft as dtk_sft

"""
All individuals get a vaccine with a distribution of initial effect (time 0).
An outbreak (time 1) causes the vaccine effect to be logged to standard out as
debug info. The distribution of initial effect should be a beta distribution.
Campaign file beta distribution parameters, for example alpha = 20.0 beta = 4.0
"""

alpha = None
beta = None

def create_report_file(report_name, log_filename, debug=False):
    success = True
    with open(report_name, "w") as outfile:
        with open(log_filename) as fid01:
            flines = [val.strip() for val in fid01.readlines()]

        intervention_factor_str = 'interventions factor ='
        dlines = [val for val in flines if intervention_factor_str in val]
        if not dlines:
            success = False
            outfile.write(f"BAD: No log lines with '{intervention_factor_str}' found in {log_filename}.\n")
        else:
            dvals = [float((val.split(intervention_factor_str)[1]).split(',')[0]) for val in dlines]
            test_data = 1 - np.array(dvals)
            success = dtk_sft.test_standard_beta_distribution(test_data, alpha, beta, outfile, True)

        outfile.write(dtk_sft.format_success_msg(success))


def application(output_folder="output",
                config_filename="config.json",
                report_name=dtk_sft.sft_output_filename,
                log_filename="test.txt",
                debug=False):
    if debug:
        print("output_folder: " + output_folder)
        print("config_filename: " + config_filename + "\n")
        print("report_name: " + report_name + "\n")
        print("log_filename: " + log_filename + "\n")
        print("debug: " + str(debug) + "\n")

    dtk_sft.wait_for_done()
    create_report_file(report_name, log_filename, debug)
