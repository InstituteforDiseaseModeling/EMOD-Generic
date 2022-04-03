#!/usr/bin/python

import json
import os.path as path
import math
import dtk_test.dtk_sft as sft


class ConfigKeys:
    TOTAL_TIMESTEPS = "Simulation_Duration"
    SIMULATION_TIMESTEP = "Simulation_Timestep"
    CONFIG_NAME = "Config_Name"
    RUN_NUMBER = "Run_Number"
    IMMUNITY_ACQUISITION_FACTOR = "Post_Infection_Acquisition_Multiplier"
    CAMPAIGN_NAME = "Campaign_Filename"


class CampaignKeys:
    START_DAY = "Start_Day"
    INFECTION_TYPE = "Infection_Type"
    INTERVENTION_CONFIG = "Intervention_Config"
    EVENT_COORDINATOR_CONFIG = "Event_Coordinator_Config"
    PRIME_ACQUIRE = "Prime_Acquire"
    PRIME_TRANSMIT = "Prime_Transmit"
    PRIME_MORTALITY = "Prime_Mortality"
    BOOST_ACQUIRE = "Boost_Acquire"
    BOOST_TRANSMIT = "Boost_Transmit"
    BOOST_MORTALITY = "Boost_Mortality"
    BOOST_THRESHOLD_ACQUIRE = "Boost_Threshold_Acquire"
    BOOST_THRESHOLD_TRANSMIT = "Boost_Threshold_Transmit"
    BOOST_THRESHOLD_MORTALITY = "Boost_Threshold_Mortality"
    NUMBER_REPETITIONS = "Number_Repetitions"
    TIMESTEPS_BETWEEN_REPETITIONS = "Timesteps_Between_Repetitions"
    DEMOGRAPHIC_COVERAGE = "Demographic_Coverage"


"""
MultiEffectBoosterVaccine is derived from MultiEffectVaccine, and preserves many of the same parameters.
The behavior is much like MultiEffectVaccine, except that upon distribution and successful take,
the vaccine's effect is determined by the recipient's immune state. If the recipient's immunity
modifier in the corresponding channel (Acquisition, Transmission, Mortality) is above a user-specified
threshold (Boost_Threshold), then the vaccine's initial effect will be equal to the parameter
Prime_Effect. If the recipient's immune modifier is below this threshold, then the vaccine's
initial effect will be equal to the parameter Boost_Effect. After distribution, the effect evolves
according to the Waning_Config, just like a Simple Vaccine. In essence, this intervention provides
a MultiEffectVaccine with at least one effect to all naive (below- threshold) individuals, and at least one effect to
all primed (above-threshold) individuals; this behavior is intended to mimic, to some degree,
biological priming and boosting.
All parameters necessary to define a MultiEffectVaccine are also necessary for a MultiEffectBoosterVaccine
(though the Initial_Effect of the Waning_Config will be overwritten by one of Prime_Effect or
Boost_Effect upon distribution).
"""


def load_emod_parameters(config_filename="config.json"):
    """
    reads config file and populates params_obj
    :param config_filename: name of config file (config.json)
    :returns param_obj:     dictionary with KEY_TOTAL_TIMESTEPS, etc., keys (e.g.)
    """
    with open(config_filename) as infile:
        cdj = json.load(infile)["parameters"]
    param_obj = {}
    param_obj[ConfigKeys.TOTAL_TIMESTEPS] = cdj[ConfigKeys.TOTAL_TIMESTEPS]
    param_obj[ConfigKeys.IMMUNITY_ACQUISITION_FACTOR] = cdj[ConfigKeys.IMMUNITY_ACQUISITION_FACTOR]
    param_obj[ConfigKeys.CONFIG_NAME] = cdj[ConfigKeys.CONFIG_NAME]
    param_obj[ConfigKeys.RUN_NUMBER] = cdj[ConfigKeys.RUN_NUMBER]
    param_obj[ConfigKeys.CAMPAIGN_NAME] = cdj[ConfigKeys.CAMPAIGN_NAME]

    return param_obj


def parse_json_report(key_new_infections_group, key_statistical_population_group, key_disease_deaths_group,
                      output_folder="output", propertyreport_name="PropertyReport.json", debug=False):
    """
    creates report_data_obj structure with keys
    :param propertyreport_name: PropertyReport.json file with location (output/PropertyReport.json)
    :param key_new_infections_group : defined in post process file
    :returns: report_data_obj structure, dictionary with KEY_NEW_INFECTIONS etc., keys (e.g.)
    """
    propertyreport_path = path.join(output_folder, propertyreport_name)
    with open(propertyreport_path) as infile:
        icj = json.load(infile)["Channels"]
    report_data_obj = {}
    for key in key_new_infections_group:
        new_infections = icj[key]["Data"]
        report_data_obj[key] = new_infections
    for key in key_statistical_population_group:
        statistical_population = icj[key]["Data"]
        report_data_obj[key] = statistical_population
    if key_disease_deaths_group is not None:
        for key in key_disease_deaths_group:
            disease_death = icj[key]["Data"]
            report_data_obj[key] = disease_death
    if debug:
        with open("DEBUG_data_PropertyReport.json", "w") as outfile:
            json.dump(report_data_obj, outfile, indent=4, sort_keys=True)

    return report_data_obj


def parse_vax_event(campaign_filename, prime_group_event, debug):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    vax_event_obj = {}
    vax_event_obj[CampaignKeys.PRIME_ACQUIRE] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.PRIME_ACQUIRE]
    vax_event_obj[CampaignKeys.BOOST_ACQUIRE] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_ACQUIRE]
    vax_event_obj[CampaignKeys.BOOST_THRESHOLD_ACQUIRE] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_THRESHOLD_ACQUIRE]
    vax_event_obj[CampaignKeys.PRIME_TRANSMIT] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.PRIME_TRANSMIT]
    vax_event_obj[CampaignKeys.BOOST_TRANSMIT] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_TRANSMIT]
    vax_event_obj[CampaignKeys.BOOST_THRESHOLD_TRANSMIT] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_THRESHOLD_TRANSMIT]
    vax_event_obj[CampaignKeys.PRIME_MORTALITY] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.PRIME_MORTALITY]
    vax_event_obj[CampaignKeys.BOOST_MORTALITY] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_MORTALITY]
    vax_event_obj[CampaignKeys.BOOST_THRESHOLD_MORTALITY] = cf[prime_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG][CampaignKeys.INTERVENTION_CONFIG][CampaignKeys.BOOST_THRESHOLD_MORTALITY]

    if debug:
        print("vax_event_obj: " + str(vax_event_obj) + "\n")

    return vax_event_obj


def parse_outbreak_event(campaign_filename, outbreak_group_event, debug):
    with open(campaign_filename) as infile:
        cf = json.load(infile)["Events"]
    outbreak_event_obj = {}
    outbreak_event_obj[CampaignKeys.START_DAY] = cf[outbreak_group_event][CampaignKeys.START_DAY]
    outbreak_event_obj[CampaignKeys.NUMBER_REPETITIONS] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.NUMBER_REPETITIONS, 1)
    outbreak_event_obj[CampaignKeys.TIMESTEPS_BETWEEN_REPETITIONS] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.TIMESTEPS_BETWEEN_REPETITIONS, -1)
    outbreak_event_obj[CampaignKeys.DEMOGRAPHIC_COVERAGE] = cf[outbreak_group_event][CampaignKeys.
        EVENT_COORDINATOR_CONFIG].get(CampaignKeys.DEMOGRAPHIC_COVERAGE, 1)

    if debug:
        print("outbreak_event_obj: " + str(outbreak_event_obj) + "\n")

    return outbreak_event_obj


def test_acquisition_blocking(acquire_effect, outbreak_event_obj, success, new_infections, statistical_populations,
                              num_group, timestep, x, outfile):

    new_infection_seed_test = new_infections[1 + x * num_group]
    statistical_population_seed_test = statistical_populations[1 + x * num_group]
    expected_new_infection_seed_test = statistical_population_seed_test * (1.0 - acquire_effect) \
                                       * outbreak_event_obj[CampaignKeys.DEMOGRAPHIC_COVERAGE]
    tolerance_1 = 0.0 if expected_new_infection_seed_test == 0.0 else 2e-2 * statistical_population_seed_test
    low_acceptance_bound = expected_new_infection_seed_test - tolerance_1
    high_acceptance_bound = expected_new_infection_seed_test + tolerance_1
    if math.fabs(new_infection_seed_test - expected_new_infection_seed_test) > tolerance_1:
        success = False
        outfile.write("BAD: At time step {0}, {1} reported new infections in Group 2_Seed_Test, which is "
                      "not within acceptance range ({2}, {3}). Expected {4}.\n"
                      .format(timestep, new_infection_seed_test, low_acceptance_bound,
                              high_acceptance_bound, expected_new_infection_seed_test))
    return success


def test_transmission_blocking(trans_effect, success, new_infections, statistical_populations,
                               num_group, timestep, x, outfile):
    new_infection_seed_test = new_infections[1 + x * num_group]
    new_infection_seed_control = new_infections[0 + x * num_group]
    new_infection_control = new_infections[2 + x * num_group]
    new_infection_test = new_infections[3 + x * num_group]
    expected_new_infection_test = (1.0 - trans_effect) * new_infection_control * new_infection_seed_test / \
                                  float(new_infection_seed_control)
    statistical_population_test = statistical_populations[3]
    tolerance_2 = 0.0 if expected_new_infection_test == 0.0 else 5e-2 * statistical_population_test
    low_acceptance_bound = expected_new_infection_test - tolerance_2
    high_acceptance_bound = expected_new_infection_test + tolerance_2
    if math.fabs(new_infection_test - expected_new_infection_test) > tolerance_2:
        success = False
        outfile.write("BAD: At time step {0}, {1} reported new infections in Group 4_Test, which is not "
                      "within acceptance range ({2}, {3}). Expected {4}.\n"
                      .format(timestep, new_infection_test, low_acceptance_bound,
                              high_acceptance_bound, expected_new_infection_test))
    return success


def test_mortality_blocking(mortality_effect, success, new_infections, new_disease_deaths,
                            num_group, timestep, x, outfile):
    new_infection_seed_test = new_infections[1 + x * num_group]
    disease_death_seed_test = new_disease_deaths[1 + x * num_group]
    expected_disease_death_seed_test = new_infection_seed_test * (1.0 - mortality_effect)
    tolerance_3 = 0.0 if expected_disease_death_seed_test == 0.0 else 2e-2 * new_infection_seed_test
    low_acceptance_bound = expected_disease_death_seed_test - tolerance_3
    high_acceptance_bound = expected_disease_death_seed_test + tolerance_3
    if math.fabs(disease_death_seed_test - expected_disease_death_seed_test) > tolerance_3:
        success = False
        outfile.write("BAD: At time step {0}, {1} reported disease deaths in Group 2_Seed_Test, which is not "
                      "within acceptance range ({2}, {3}). Expected {4}.\n"
                      .format(timestep, disease_death_seed_test, low_acceptance_bound,
                              high_acceptance_bound, expected_disease_death_seed_test))
    return success


def create_report_file(vax_event_obj, outbreak_event_obj, param_obj, report_data_obj, report_name, new_infections_group,
                       statistical_population_group, disease_deaths_group, acquisition, transmission, mortality, debug):
    with open(report_name, "w") as outfile:
        outfile.write("# Test name: " + str(param_obj[ConfigKeys.CONFIG_NAME] + ", Run number: " +
                      str(param_obj[ConfigKeys.RUN_NUMBER])) +
                      "\n# Test compares the expected reported infections and disease deaths with the experimental "
                      "values.\n")
        timestep = outbreak_event_obj[CampaignKeys.START_DAY]
        new_infections = []
        statistical_populations = []
        new_disease_deaths = []
        num_group = len(new_infections_group)
        acquire_effect = vax_event_obj[CampaignKeys.PRIME_ACQUIRE]
        mortality_effect = vax_event_obj[CampaignKeys.PRIME_MORTALITY]
        trans_effect = vax_event_obj[CampaignKeys.PRIME_TRANSMIT]
        acq_success = True
        trans_success = True
        mort_success = True
        for x in range(outbreak_event_obj[CampaignKeys.NUMBER_REPETITIONS]):
            for i in range(num_group):
                new_infection = report_data_obj[new_infections_group[i]][timestep]
                statistical_population = report_data_obj[statistical_population_group[i]][timestep]
                new_infections.append(new_infection)
                statistical_populations.append(statistical_population)
                if disease_deaths_group is not None:
                    # disease deaths in the last 2 groups happen 1 day later than the first 2 groups
                    pre_disease_death = report_data_obj[disease_deaths_group[i]][int(timestep + i/2 - 1)]
                    disease_death = report_data_obj[disease_deaths_group[i]][int(timestep + i/2)]
                    new_disease_death = disease_death - pre_disease_death
                    new_disease_deaths.append(new_disease_death)
            if acquisition:
                acq_success = test_acquisition_blocking(acquire_effect, outbreak_event_obj, acq_success, new_infections,
                                                        statistical_populations, num_group, timestep, x, outfile)
            if transmission:
                trans_success = test_transmission_blocking(trans_effect, trans_success,
                                                           new_infections, statistical_populations, num_group,
                                                           timestep, x, outfile)
            if mortality:
                mort_success = test_mortality_blocking(mortality_effect, mort_success,
                                                       new_infections, new_disease_deaths, num_group,
                                                       timestep, x, outfile)
            timestep += outbreak_event_obj[CampaignKeys.TIMESTEPS_BETWEEN_REPETITIONS]
            acquire_effect = acquire_effect + (1.0 - acquire_effect) * vax_event_obj[CampaignKeys.BOOST_ACQUIRE]
            mortality_effect = mortality_effect + (1.0 - mortality_effect) * vax_event_obj[CampaignKeys.BOOST_MORTALITY]
            trans_effect = trans_effect + (1.0 - trans_effect) * vax_event_obj[CampaignKeys.BOOST_TRANSMIT]
        if acq_success and acquisition:
            outfile.write("GOOD: For all time steps, reported new infections in Group 2_Seed_Test were within 2% of "
                          "expected (statistical population).\n")
        if trans_success and transmission:
            outfile.write("GOOD: For all time steps, reported new infections in Group 4_Test were within 2% "
                          "of expected (statistical population).\n")
        if mort_success and mortality:
            outfile.write("GOOD: For all time steps, reported disease deaths in Group 2_Seed_Test "
                          "were within 2% of expected (statistical population).\n")
        success = acq_success and trans_success and mort_success
        outfile.write(sft.format_success_msg(success))
    sft.plot_data(new_infections, new_disease_deaths,
                  label1="new_infections", label2="disease_death",
                  xlabel="0&4:Seed_Control, 1&5:Seed_Test, 2&6:Control, 3&7:Test",
                  title="new_infections vs. new_disease_death",
                  category='New_infections_vs_new_disease_death', show=True)
    if debug:
        print("SUMMARY: Success={0}\n".format(success))
    return success
