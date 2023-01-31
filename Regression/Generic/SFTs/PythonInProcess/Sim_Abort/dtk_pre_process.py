#!/usr/bin/python

"""
Generates a random number between 0 and 98 (Simulation_Duration = 100) to end simulation early on
that timestep
"""

import dtk_in_process
import dtk_post_process
import random


def application(config_filename, debug=False):
    abort_time = random.randint(0, 98)
    dtk_in_process.ABORT_TIMESTEP = abort_time
    dtk_post_process.ABORT_TIMESTEP = abort_time

    return config_filename
