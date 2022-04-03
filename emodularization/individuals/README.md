# Intrahost Python Modules (PyMods)

All simulation types should have intrahost components available as python modules. These are incredibly useful for:
- Testing: intrahost code can be tested in isolation entirely in python. Including (soon) with interventions for intervention testing.
- Modeling: intrahost DTK components can be used for modeling without having to use the entire DTK.
- Development: Being able to run intrahost code in isolation helps developers improve the interfaces and develop related code more efficiently.
- Visualization: Intrahost components can be exercised via python and visualized in python or in a website.
- Onboarding/Training. Quickstart!

# Building
To build any of the python modules, go into the subdirectory and run:

    python3 package_setup.py build

Note that the options to package_setup in addition to build are bdist_wheel and install.


# Testing & Status
All disease modules now build, and import. All except Malaria run the most basic (non-disease-specific) tests. In the case of Malaria, a range of configuration parameters need to be moved out of SimulationConfig and into more 'local' classes.

# Requirements
The specific requirements for each disease-specific intrahost component is still under development. The hope is that the researchers who are disease owners will help specify the API.
