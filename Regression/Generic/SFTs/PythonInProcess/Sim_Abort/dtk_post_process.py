#!/usr/bin/python


"""
Verifies that simulation ends at an earlier-than-Simulation_Duration time randomly generated in pre_process
when an abort signal is provided by the in-process script, not at Simulation_Duration time == 100 as specified
in the configuration
"""
import dtk_test.dtk_sft as dtk_sft
import json
import os

ABORT_TIMESTEP = None


def create_report_file(report_name, log_filename, debug=False):
    success = True

    with open(report_name, "w") as outfile:
        official_time = None
        hello_time = None
        with open(log_filename, "r") as logfile:
            for line in logfile:
                if "Update(): Time:" in line:
                    official_time = float(dtk_sft.get_val("Time: ", line))
                elif "Hello from timestep" in line:
                    hello_time = float(dtk_sft.get_val("timestep ", line))
        # We expect the last printed in_process_time to be equal to abort_timestep
        # and the last Time: timestamp to be one step ahead (known one-off artifact)
        if not official_time or not hello_time:
            success = False
            outfile.write(f"BAD: Missing timestamp data the log file. Please check {log_filename}"
                          f"for 'Update(): Time:' and 'Hello from timestep' log lines.\n")
        elif ABORT_TIMESTEP != hello_time:
            success = False
            outfile.write(f"BAD: We expect the last 'Hello from timestep' to be from the abort_timestep at "
                          f"{ABORT_TIMESTEP}, "
                          f"but we see the last 'Hello from timestep' at {hello_time}.\n")
        elif ABORT_TIMESTEP != official_time - 1:
            success = False
            outfile.write(f"BAD: We expect the last 'Update(): Time:' to be from the abort_timestep + 1 = "
                          f"({ABORT_TIMESTEP + 1}), "
                          f"but we see the last 'Update(): Time:' at {official_time}.\n")
        else:
            outfile.write(f"GOOD: We expected the last 'Update(): Time:' to be from the abort_timestep + 1 = "
                          f"{ABORT_TIMESTEP + 1} and the last 'Hello from timestep' message to be from "
                          f"abort_timestep = {ABORT_TIMESTEP} and that's what we saw.\n")

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
