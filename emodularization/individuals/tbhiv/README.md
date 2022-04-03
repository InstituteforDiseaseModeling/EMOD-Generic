# Example Package

This is the subdirectory for TBHIV_SIM Intrahost Python Module (compiled C++). The interface thus far is very
ad-hoc and proof-of-concept. But the module wrapped underneath is the DTK Generic Individual including 
susceptibility, infection, and interventions container. It can be used on its own, with GENERIC_SIM interventions
(thus far SimpleVaccine and SimpleBoosterVaccine), and with the DTK Node Demographics module for population-level
modeling. This module should be built in bamboo, tested, and uploaded to the DTK Artfactory PIP server from which
it shall be pip installable.

To see a simple demo of how to give interventions like SimpleVaccine to an individual, all in python, but using
all DTK C++ code underneath, see test_iv.py in this folder. It works with the sv.json intervention config file.
