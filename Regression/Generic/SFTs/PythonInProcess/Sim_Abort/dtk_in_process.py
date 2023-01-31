#!/usr/bin/python
ABORT_TIMESTEP = None


def application(timestep):
    timestep = float(timestep)

    print(f"Hello from timestep {timestep}")

    if timestep >= ABORT_TIMESTEP:
        print("Time to end simulation.")
        return "ABORT"

    return None
