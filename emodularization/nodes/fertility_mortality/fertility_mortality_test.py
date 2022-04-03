#!/usr/bin/python
import sys
sys.path.append( "/var/www/wsgi/" ) # this no scale
sys.path.append( "../../individuals/build/lib.win-amd64-2.7" )
sys.path.append('../../../Python/')
import os
import logging
import json
import random
import math
import datetime
import unittest
import numpy as np
import shutil
import scipy
# import glob
# import csv

import dtk_test.dtk_sft as sft
import dtk_nodedemog as nd
import dtk_genericindividual as gi
import dtk_InfectivityScalingBoxcar_Support as Boxcar_S

"""
In this test, we test fertility and mortality rates with DTK PyMod code.

The fertility and mortality rates are configured in nd.json(same as gi.json)
and demoegraphics.json. In each test, we have an initial population in a
single node. The test set either the fertility params or mortality params and/or
age params. Then run the simulation for several years, at each day, we collect
the information about possible mother, pregnant mother, new borns and deaths.

After the simulation is done, we test the fertility rate or mortality rate at each time step
with expected values/distribution calculated based on the params we set for the test. For
fertility tests, we also test the total new borns, total pregnancies, new born male/female ratio
and 280 days of pregnancy duration (only for IP tests). For mortality tests, we test total deaths
during the simulation.
"""
class DemographicsParameters():
    Nodes = "Nodes"
    NodeAttributes = "NodeAttributes"
    InitialPopulation = "InitialPopulation"
    BirthRate = "BirthRate"
    IndividualAttributes = "IndividualAttributes"
    FertilityDistribution = "FertilityDistribution"
    MortalityDistribution = "MortalityDistribution"
    MortalityDistributionMale = "MortalityDistributionMale"
    MortalityDistributionFemale = "MortalityDistributionFemale"
    NumPopulationGroups = "NumPopulationGroups"
    PopulationGroups = "PopulationGroups"
    ResultScaleFactor = "ResultScaleFactor"
    ResultValues = "ResultValues"
    AgeDistribution = "AgeDistribution"
    DistributionValues = "DistributionValues"

class ConfigParameters():
    Enable_Vital_Dynamics = "Enable_Vital_Dynamics"
    # birth rate dependence
    Enable_Birth = "Enable_Birth"
    Birth_Rate_Dependence = "Birth_Rate_Dependence"
    x_Birth = "x_Birth"
    FIXED_BIRTH_RATE = "FIXED_BIRTH_RATE"
    POPULATION_DEP_RATE = "POPULATION_DEP_RATE"
    DEMOGRAPHIC_DEP_RATE = "DEMOGRAPHIC_DEP_RATE"
    INDIVIDUAL_PREGNANCIES = "INDIVIDUAL_PREGNANCIES"
    INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR = "INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR"
    # birth rate time dependence
    Birth_Rate_Time_Dependence = "Birth_Rate_Time_Dependence"
    NONE = "NONE"
    ANNUAL_BOXCAR_FUNCTION = "ANNUAL_BOXCAR_FUNCTION"
    SINUSOIDAL_FUNCTION_OF_TIME = "SINUSOIDAL_FUNCTION_OF_TIME"
    Boxcar_Amplitude = "Birth_Rate_Boxcar_Forcing_Amplitude"
    Boxcar_Start_Time = "Birth_Rate_Boxcar_Forcing_Start_Time"
    Boxcar_End_Time = "Birth_Rate_Boxcar_Forcing_End_Time"
    Sinusoidal_Amplitude = "Birth_Rate_Sinusoidal_Forcing_Amplitude"
    Sinusoidal_Phase = "Birth_Rate_Sinusoidal_Forcing_Phase"
    Base_Year = "Base_Year"
    # death rate dependence
    Enable_Natural_Mortality = "Enable_Natural_Mortality"
    Death_Rate_Dependence = "Death_Rate_Dependence"
    NOT_INITIALIZED = "NOT_INITIALIZED"
    NONDISEASE_MORTALITY_BY_AGE_AND_GENDER = "NONDISEASE_MORTALITY_BY_AGE_AND_GENDER"
    NONDISEASE_MORTALITY_BY_YEAR_AND_AGE_FOR_EACH_GENDER = "NONDISEASE_MORTALITY_BY_YEAR_AND_AGE_FOR_EACH_GENDER"
    X_Other_Mortality = "x_Other_Mortality"
    # age dependence
    Age_Initialization_Distribution_Type = "Age_Initialization_Distribution_Type"
    DISTRIBUTION_SIMPLE = "DISTRIBUTION_SIMPLE"
    DISTRIBUTION_COMPLEX = "DISTRIBUTION_COMPLEX"
    DISTRIBUTION_OFF = "DISTRIBUTION_OFF"

class Constant():
    ci_probability = 0.9999  # 99.99% confidence interval
    days_per_year = 365
    two_percent_per_year = 0.02 / days_per_year
    individual_birthrate = 1.0 / 8 / days_per_year  # 1 child every 8 years of fertility (about 4 total)
    nine_months = 280
    birthrate_threshold = 0.005

class FertilityMortalityTest(unittest.TestCase):
    human_pop = {}
    my_persisten_vars = {}
    nursery = {}  # store newborn counter by timestep (dict of int to int for now)
    timestep = 0
    debug = False
    statistical_population = []
    possible_mother = []
    pregnant_mother = []
    new_pregnancy = {}
    possible_mom_by_age_time = []
    well_mixed_contagion_pool = 0
    population_by_age_gender = {"male": {}, "female": {}}
    for sex in population_by_age_gender:
        for age in range(0, 100):
            population_by_age_gender[sex][age] = []
    show = False

    # region class method
    def setUp(self):
        # reload(nd)
        # reload(gi)
        self.human_pop = {}
        self.my_persisten_vars = {}
        self.nursery = {}  # store newborn counter by timestep (dict of int to int for now)
        self.timestep = 0
        self.statistical_population = []
        self.possible_mother = []
        self.pregnant_mother = []
        self.new_pregnancy = {}
        self.possible_mom_by_age_time = []
        self.well_mixed_contagion_pool = 0
        self.population_by_age_gender = {"male": {}, "female": {}}
        for sex in self.population_by_age_gender:
            for age in range(0, 100):
                self.population_by_age_gender[sex][age] = []
        now = datetime.datetime.now()
        now_string = now.strftime("%H-%M-%S")
        logging.basicConfig(filename="{0}.log".format(now_string),
                            format='%(asctime)s:%(levelname)s:%(message)s',
                            level=logging.INFO)
        pass

    def tearDown(self):
        pass
    # endregion

    def create_person_callback(self, mcw, age, gender):
        person = {}
        person["mcw"] = mcw
        person["age"] = age
        person["sex"] = gender

        new_id = gi.create((gender, age, mcw))
        person["id"] = new_id
        if new_id in self.human_pop:
            raise Exception(" individual {0} is already created.".format(new_id))
        else:
            self.human_pop[new_id] = person
        if self.debug:
            logging.info("Human population now = {0}.".format(len(self.human_pop)))
        if age == 0:
            logging.info("Made a baby with id = {0} on timestep {1}.".format(new_id, self.timestep))
            if self.timestep not in self.nursery:
                self.nursery[self.timestep] = (0, 0)
            boys = self.nursery[self.timestep][0]
            girls = self.nursery[self.timestep][1]
            if gender == 0:
                boys += mcw
            else:
                girls += mcw
            self.nursery[self.timestep] = (boys, girls)

    def conceive_baby_callback(self, individual_id, duration):
        logging.info("{0} just got pregnant".format(individual_id))
        # if individual_id not in human_pop:
        # print( "Yikes! {0} is supposed to get pregnant, but she's not in our population!".format( individual_id ) )
        # else:
        # print( str( gi.is_dead( individual_id ) ) );
        if self.timestep in self.new_pregnancy:
            self.new_pregnancy[self.timestep] += 1
        else:
            self.new_pregnancy[self.timestep] = 1
        gi.initiate_pregnancy(individual_id)

    def conceive_baby_by_age_year_callback(self, individual_id, duration):
        logging.info("{0} just got pregnant".format(individual_id))
        # if individual_id not in human_pop:
        # print( "Yikes! {0} is supposed to get pregnant, but she's not in our population!".format( individual_id ) )
        # else:
        # print( str( gi.is_dead( individual_id ) ) );
        age_f = gi.get_age(individual_id) / Constant.days_per_year
        # age = math.floor(age_f)
        age = sft.round_down(age_f, precision=2)
        if self.timestep in self.new_pregnancy:
            if age in self.new_pregnancy[self.timestep]:
                self.new_pregnancy[self.timestep][age] += 1
            else:
                self.new_pregnancy[self.timestep][age] = 1
        else:
            self.new_pregnancy[self.timestep] = {age: 1}

        # logging.info("found one new pregnancy age {0}: {1}".format(age, age_f))
        gi.initiate_pregnancy(individual_id)

    def update_pregnancy_callback(self, individual_id, dt):
        # if individual_id not in human_pop:
        # print( "Yikes! {0} not in our population!".format( individual_id ) )
        # else:
        # print( "{0} updating pregnancy (dt = {1}).".format( individual_id, dt ) )
        return gi.update_pregnancy(individual_id, int(dt))

    # def mortality_callback(self, age, sex, dt):
    #     mortality_rate = nd.get_mortality_rate((age, sex, dt))
    #     return mortality_rate

    def mortality_callback(self, age, sex):  # , dt):
        mortality_rate = nd.get_mortality_rate((age, sex))
        return mortality_rate

    def expose(self, action, prob, individual_id):
        # if random.random() < 0.0001 and self.timestep == 1:  # let's infect some people at random (outbreaks)
        #     logging.info("Let's infect (outbreak) based on random draw.")
        #     return 1
        # else:
        #     if self.well_mixed_contagion_pool > 0:
        #         prob = scipy.stats.expon.cdf(self.well_mixed_contagion_pool * gi.get_immunity(individual_id))
        #         # print( "prob = {0} with contagion {1}.".format( prob, well_mixed_contagion_pool ) )
        #         # prob = 1 - math.exp( -well_mixed_contagion_pool * gi.get_immunity( individual_id ) )
        #         # print( "prob = {0}.".format( prob ) )
        #         if random.random() < prob:
        #             logging.debug("Let's infect based on transmission.")
        #             return 1
        return 0

    def deposit(self, contagion, individual):
        # self.well_mixed_contagion_pool += contagion
        # logging.debug("Depositing {0} contagion creates total of {1}.".format(contagion, self.well_mixed_contagion_pool))
        return

    def get_json_template(self, json_filename="demographics_template.json"):
        with open(json_filename) as infile:
            j_file_obj = json.load(infile)
        return j_file_obj

    def set_demographics_file(self, demographics, demo_filename="demographics.json"):
        with open(demo_filename, 'w') as outfile:
            json.dump(demographics, outfile, indent=4, sort_keys=True)

    def set_config_file(self, config, config_filename="nd.json"):
        with open(config_filename, 'w') as outfile:
            json.dump(config, outfile, indent=4, sort_keys=True)

    def set_gi_file(self, config, gi_filename="gi.json"):
        with open(gi_filename, 'w') as outfile:
            json.dump(config, outfile, indent=4, sort_keys=True)

    def set_base_year(self, _base_year = 0, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        config[ConfigParameters.Base_Year] = _base_year
        self.set_config_file(config)
        
    def enable_birth(self, enable_birth = True, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        if enable_birth:
            print("Enable birth")
            config[ConfigParameters.Enable_Birth] = 1
        else:
            print("Disable birth")
            config[ConfigParameters.Enable_Birth] = 0
        self.set_config_file(config)
        self.set_gi_file(config)

    def enable_natural_death(self, enable_natural_death=True, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        if enable_natural_death:
            print("Enable natural death")
            config[ConfigParameters.Enable_Natural_Mortality] = 1
        else:
            print("Disable natural death")
            config[ConfigParameters.Enable_Natural_Mortality] = 0
        self.set_config_file(config)
        self.set_gi_file(config)

    def configure_birth_rate(self, Birth_Rate_Dependence, rate, x_Birth, initial_population=10000,
                             demo_template_filename="demographics_template.json", enable_birth=True,
                             enable_death=False):
        print("configure demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_template_filename)
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][
            DemographicsParameters.BirthRate] = rate
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][
            DemographicsParameters.InitialPopulation] = initial_population
        self.set_demographics_file(demographics)

        print("configure nd.json and gi.json.\n")
        config = self.get_json_template(json_filename="nd_template.json")
        config[ConfigParameters.Birth_Rate_Dependence] = Birth_Rate_Dependence
        config[ConfigParameters.x_Birth] = x_Birth
        config[ConfigParameters.Death_Rate_Dependence] = ConfigParameters.NOT_INITIALIZED
        self.set_config_file(config)
        self.set_gi_file(config)
        self.enable_natural_death(enable_natural_death=enable_death)
        self.enable_birth(enable_birth=enable_birth)

    def configure_birth_rate_time_dependence(self, Birth_Rate_Time_Dependence=ConfigParameters.NONE,
                                             amplitude=1.0, param1=0, param2=0):
        print("configure fixed birth rate time dependence.\n")
        if Birth_Rate_Time_Dependence == ConfigParameters.NONE:
            return
        else:
            config = self.get_json_template(json_filename="nd.json")
            config[ConfigParameters.Birth_Rate_Time_Dependence] = Birth_Rate_Time_Dependence
            if Birth_Rate_Time_Dependence == ConfigParameters.SINUSOIDAL_FUNCTION_OF_TIME:
                config[ConfigParameters.Sinusoidal_Amplitude] = amplitude
                config[ConfigParameters.Sinusoidal_Phase] = param1
            elif Birth_Rate_Time_Dependence == ConfigParameters.ANNUAL_BOXCAR_FUNCTION:
                config[ConfigParameters.Boxcar_Amplitude] = amplitude
                config[ConfigParameters.Boxcar_Start_Time] = param1
                config[ConfigParameters.Boxcar_End_Time] = param2
            else:
                raise Exception("Birth_Rate_Time_Dependence must be {0}, {1} "
                                "or {2}".format(ConfigParameters.NONE,
                                                ConfigParameters.SINUSOIDAL_FUNCTION_OF_TIME,
                                                ConfigParameters.ANNUAL_BOXCAR_FUNCTION))
            self.set_config_file(config)
            self.set_gi_file(config)

    def draw_random_boxcar_time_dependence(self):
        amplitude = random.uniform(0, 10)
        mode = Boxcar_S.draw_random_mode()
        start_time, end_time = Boxcar_S.generate_start_and_end_time(mode=mode)
        return mode, amplitude, start_time, end_time

    def configure_random_sinusoidal_time_dependence(self):
        amplitude = random.uniform(0, 1)
        phase = random.uniform(0, 360)
        self.configure_birth_rate_time_dependence(
            Birth_Rate_Time_Dependence=ConfigParameters.SINUSOIDAL_FUNCTION_OF_TIME,
            amplitude=amplitude, param1=phase)
        return amplitude, phase

    def set_fertility_distribution(self, PopulationGroups, ResultScaleFactor, ResultValues,
                                   demo_filename="demographics.json"):
        print("configure fertility distribution in demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_filename)
        # PopulationGroups
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.FertilityDistribution][DemographicsParameters.PopulationGroups] = PopulationGroups
        # NumPopulationGroups
        NumPopulationGroups = [len(PopulationGroups[0]), len(PopulationGroups[1])]
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.FertilityDistribution][
            DemographicsParameters.NumPopulationGroups] = NumPopulationGroups
        # ResultScaleFactor
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.FertilityDistribution][
            DemographicsParameters.ResultScaleFactor] = ResultScaleFactor
        # ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.FertilityDistribution][DemographicsParameters.ResultValues] = ResultValues
        self.set_demographics_file(demographics)

        print ("configure base year in nd.json and gi.json.\n")
        config = self.get_json_template(json_filename="nd.json")
        config[ConfigParameters.Base_Year] = PopulationGroups[1][0]
        self.set_config_file(config)
        self.set_gi_file(config)

    def set_age_distribution(self, ResultValues, DistributionValues, age_distribution_type):
        print ("configure age distribution type in nd.json and gi.json.\n")
        config = self.get_json_template(json_filename="nd.json")
        config[ConfigParameters.Age_Initialization_Distribution_Type] = age_distribution_type
        self.set_config_file(config)
        self.set_gi_file(config)

        print ("Configure age distribution in demographics.json.\n")
        demographics = self.get_json_template(json_filename="demographics.json")
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.AgeDistribution][DemographicsParameters.ResultValues] = ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.AgeDistribution][DemographicsParameters.DistributionValues] = DistributionValues
        self.set_demographics_file(demographics)

    def configure_death_rate(self, Death_Rate_Dependence, x_other_mortality=1, enable_birth=False, enable_natural_death=True):
        print ("configure Death_Rate_Dependence in nd.json and gi.json.\n")
        config = self.get_json_template(json_filename="nd_template.json")
        config[ConfigParameters.Death_Rate_Dependence] = Death_Rate_Dependence
        config[ConfigParameters.X_Other_Mortality] = x_other_mortality
        self.set_config_file(config)
        self.set_gi_file(config)
        self.enable_birth(enable_birth=enable_birth)
        self.enable_natural_death(enable_natural_death=enable_natural_death)

    def set_mortality_distribution(self, PopulationGroups, ResultScaleFactor, ResultValues, initial_population,
                                   demo_filename="demographics_template.json"):
        print("configure mortality distribution in demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_filename)
        # PopulationGroups
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistribution][DemographicsParameters.PopulationGroups] = PopulationGroups
        # NumPopulationGroups
        NumPopulationGroups = [len(PopulationGroups[0]), len(PopulationGroups[1])]
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistribution][
            DemographicsParameters.NumPopulationGroups] = NumPopulationGroups
        # ResultScaleFactor
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistribution][
            DemographicsParameters.ResultScaleFactor] = ResultScaleFactor
        # ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistribution][DemographicsParameters.ResultValues] = ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][
            DemographicsParameters.InitialPopulation] = initial_population
        self.set_demographics_file(demographics)

    def set_male_mortality_distribution(self, PopulationGroups, ResultScaleFactor, ResultValues, initial_population,
                                        demo_filename="demographics_template.json"):
        print("configure male mortality distribution in demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_filename)
        # PopulationGroups
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionMale][
            DemographicsParameters.PopulationGroups] = PopulationGroups
        # NumPopulationGroups
        NumPopulationGroups = [len(PopulationGroups[0]), len(PopulationGroups[1])]
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionMale][
            DemographicsParameters.NumPopulationGroups] = NumPopulationGroups
        # ResultScaleFactor
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionMale][
            DemographicsParameters.ResultScaleFactor] = ResultScaleFactor
        # ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionMale][DemographicsParameters.ResultValues] = ResultValues
        # initial population
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][
            DemographicsParameters.InitialPopulation] = initial_population
        self.set_demographics_file(demographics)

    def set_female_mortality_distribution(self, PopulationGroups, ResultScaleFactor, ResultValues, initial_population,
                                          demo_filename="demographics_template.json"):
        print("configure male mortality distribution in demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_filename)
        # PopulationGroups
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionFemale][
            DemographicsParameters.PopulationGroups] = PopulationGroups
        # NumPopulationGroups
        NumPopulationGroups = [len(PopulationGroups[0]), len(PopulationGroups[1])]
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionFemale][
            DemographicsParameters.NumPopulationGroups] = NumPopulationGroups
        # ResultScaleFactor
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionFemale][
            DemographicsParameters.ResultScaleFactor] = ResultScaleFactor
        # ResultValues
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.IndividualAttributes][
            DemographicsParameters.MortalityDistributionFemale][DemographicsParameters.ResultValues] = ResultValues
        # initial population
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][
            DemographicsParameters.InitialPopulation] = initial_population
        self.set_demographics_file(demographics)

    def run_test(self, duration=365, birth_by_age_year_flag=False, death_by_age_gender_flag=False, debug=False):
        gi.reset()
        logging.info("We cleared out human_pop. Should get populated via populate_from_files and callback...")

        # set creation callback
        nd.set_callback(self.create_person_callback)
        nd.populate_from_files()

        # set vital dynamics callbacks
        gi.set_mortality_callback(self.mortality_callback)
        gi.my_set_callback(self.expose)
        gi.set_deposit_callback(self.deposit)
        if birth_by_age_year_flag:
            nd.set_conceive_baby_callback(self.conceive_baby_by_age_year_callback)
        else:
            nd.set_conceive_baby_callback(self.conceive_baby_callback)
        nd.set_update_preg_callback(self.update_pregnancy_callback)

        # pop_age = []
        # for hum_id in self.human_pop:
        #     age = gi.get_age(hum_id)
        #     pop_age.append(age)
        # sft.plot_hist(pop_age, label1="initial_population_age")

        # with open("initial_human_pop.json", "w") as file:
        #     file.write(json.dumps(self.human_pop, indent=4, sort_keys=True))

        logging.info("Update fertility & mortality of population of size {0} for {1} year.".format(len(self.human_pop),
                                                                                                   duration / 365))
        graveyard = {}

        for t in range(0, duration):
            possible_mom = 0
            pregnant_mom = 0
            possible_mom_by_age = {}
            population_by_age_gender_per_timestep = {"male": {}, "female": {}}
            self.timestep = t
            # logging.info("Updating individuals at timestep {0}.".format(t))
            graveyard_per_timestep = []
            self.statistical_population.append(len(self.human_pop))
            # with open("human_pop.json", "w") as file:
            #     file.write(json.dumps(self.human_pop, indent=4, sort_keys=True))

            # this is for shedding and infection only
            num_infected = 0
            for hum_id in self.human_pop:
                # syslog.syslog( syslog.LOG_INFO, "Updating individual {0} at timestep {1}.".format( hum_id, t ) )
                nd.update_node_stats(
                    (1.0, 0.0, gi.is_possible_mother(hum_id), 0))  # mcw, infectiousness, is_poss_mom, is_infected

            logging.info("Updating individuals (exposing and vital-dynamics) at timestep {0}.".format(t))

            for hum_id in list(self.human_pop.keys()): # advoid "RuntimeError: dictionary changed size during iteration"
                human = self.human_pop.get(hum_id)
                gi.update2(hum_id)  # this should do exposure & vital-dynamics
                gender = self.human_pop[hum_id]["sex"]
                if gi.is_dead(hum_id):
                    logging.info("Individual {0} died at timestep {1}.".format(hum_id, self.timestep))
                    graveyard_per_timestep.append(human)
                    try:
                        self.human_pop.pop( hum_id )
                    except Exception as ex:
                        logging.info(
                            "Exception trying to remove individual {0} from python list: {1}".format(hum_id, ex))
                ipm = False
                ip = False
                if gender == 1:
                    ipm = gi.is_possible_mother( hum_id )
                    # print "ipm is {}".format(ipm)
                    ip = gi.is_pregnant( hum_id )
                    # print "ip is {}".format(ip)
                age = gi.get_age( hum_id )# /Constant.days_per_year
                if birth_by_age_year_flag:
                    age_year = sft.round_down(age / Constant.days_per_year, precision=2)
                else:
                    age_year = math.floor(age / Constant.days_per_year)
                # print "age is {0}, gender is {1}.".format(age, gender)

                if death_by_age_gender_flag:
                    sex = "male" if gender == 0 else "female"
                    if age_year in population_by_age_gender_per_timestep[sex]:
                        population_by_age_gender_per_timestep[sex][age_year] += 1
                    else:
                        population_by_age_gender_per_timestep[sex][age_year] = 1

                if ipm:
                    possible_mom += 1
                    if birth_by_age_year_flag:
                        if age_year in possible_mom_by_age:
                            possible_mom_by_age[age_year] += 1
                        else:
                            possible_mom_by_age[age_year] = 1
                    self.assertTrue(14 <= age / Constant.days_per_year <= 45, "child-bearing age is 14-45, got age = {0}".format(age))
                    self.assertTrue(gender == 1, "possible mom must be female, got gender = {0}".format(gender))
                if ip:
                    pregnant_mom += 1
                    if ipm:
                        possible_mom -= 1  # pregnant mom will not be considered as possible mom
                        if birth_by_age_year_flag:
                            possible_mom_by_age[age_year] -= 1
                            # self.assertTrue(ipm==1, "only prossible mother can get pregnant, got is_possible_mother {0} for hum_id {1}".format(ipm, hum_id)) # this is not always True, mom can exit the is_possible_mom state due to aging
                # print( "Calling cfp with {0}, {1}, {2}, and {3} at time step {4}.".format( str(ipm), str(ip), str(age), str(hum_id), t) )
                nd.consider_for_pregnancy((ipm, ip, hum_id, age, 1.0))

            # print "possible_mom is {}".format(possible_mom)
            # print "pregnant_mom is {}".format(pregnant_mom)
            self.possible_mother.append(possible_mom)
            self.pregnant_mother.append(pregnant_mom)
            if birth_by_age_year_flag:
                self.possible_mom_by_age_time.append(possible_mom_by_age)
            if graveyard_per_timestep:
                graveyard[self.timestep] = graveyard_per_timestep
            if death_by_age_gender_flag:
                for sex in self.population_by_age_gender:
                    for age in self.population_by_age_gender[sex]:
                        if age in population_by_age_gender_per_timestep[sex]:
                            self.population_by_age_gender[sex][age].append(
                                population_by_age_gender_per_timestep[sex][age])
                        else:
                            self.population_by_age_gender[sex][age].append(0)
            logging.info("Updating fertility at timestep {0}.".format(t))
            nd.update_fertility()
        return graveyard

    def save_files(self, graveyard, path, test_name, debug=False):
        path_name = os.path.join(path, test_name)
        if os.path.exists(path_name):
            shutil.rmtree(path_name)
            print("Delete folder: {}".format(path_name))
        os.makedirs(path_name)
        print("Create folder: {}".format(path_name))
        if debug:
            files_to_save = ["nusery.json", "graveyard.json", "statistical_population.json", "possible_mother.json",
                             "pregnant_mother.json", "new_pregnancy.json"]
            data_to_save = [self.nursery, graveyard, self.statistical_population, self.possible_mother,
                            self.pregnant_mother, self.new_pregnancy]
            for i in range(len(files_to_save)):
                with open(os.path.join(path_name, files_to_save[i]), "w") as file:
                    file.write(json.dumps(data_to_save[i], indent=4, sort_keys=True))

        # files_to_copy = glob.glob("*.png")
        # files_to_copy.append(["nd.json", "gi.json","demographics.json"])
        files_to_copy = ["nd.json", "gi.json", "demographics.json"]
        for file in files_to_copy:
            shutil.move(file, os.path.join(path_name, file))

    def plot_new_births(self, baby_count, duration, label1, label2, xlabel, ylabel, title, category):
        new_borns = []
        for time in range(duration):
            new_borns.append(baby_count[0][time] + baby_count[1][time])
        sft.plot_data(new_borns, label1=label1, label2=label2,
                                   xlabel=xlabel, ylabel=ylabel,
                                   title=title, category=category, show=self.show)

    def get_baby_count(self, duration):
        baby_count = [[], []]
        for time in range(0, duration):
            if time in self.nursery:
                babies = self.nursery[time]
            else:
                babies = [0, 0]
            for i in range(len(babies)):
                baby_count[i].append(babies[i])
        boy_count = sum(baby_count[0])
        girl_count = sum(baby_count[1])
        return baby_count, boy_count, girl_count

    def total_birth_count_test(self, rate_per_day, duration, boy_count, girl_count):
        expected_new_born = rate_per_day * duration
        total_new_born = boy_count + girl_count
        error_tolerance = 5e-2 * expected_new_born
        msg = "expected_new_born is {0} while total_new_born is {1}.".format(expected_new_born, total_new_born)
        logging.info(msg)
        return math.fabs(expected_new_born - total_new_born) <= error_tolerance, msg

    def male_female_ratio_test(self, boy_count, girl_count):
        ratio = boy_count / float(girl_count)
        msg = "baby boy is {0} while baby girl is {1}, M/F ratio is {2}".format(boy_count, girl_count, ratio)
        logging.info(msg)
        return math.fabs(ratio - 1.0) <= 1.5e-1, msg

    def no_death_test(self, graveyard):
        natural_death = 0
        for t in graveyard:
            natural_death += len(graveyard[t])
        error_msg = "Found {0} natural death(see graveyard.json), expected no nondisease death.".format(natural_death)
        logging.info(error_msg)
        return natural_death == 0, error_msg

    def no_birth_test(self):
        birth = 0
        for t in self.nursery:
            birth += len(self.nursery[t])
        error_msg = "Found {0} birth(see nusery.json), expected no birth.".format(birth)
        logging.info(error_msg)
        return birth == 0, error_msg

    def pregnancy_duration_test(self, duration, birth_by_age_year_flag=False):
        succeed = True
        error_msg = []
        for t in list(self.nursery.keys()): # avoid RuntimeError: dictionary changed size during iteration
            num_new_born = sum(self.nursery.pop(t))
            try:
                if birth_by_age_year_flag:
                    pregnancy = self.new_pregnancy.pop(t - Constant.nine_months)
                    num_pregnancy = 0
                    for age in pregnancy :
                        num_pregnancy += pregnancy[age]
                else:
                    num_pregnancy = self.new_pregnancy.pop(t - Constant.nine_months)
                result = num_pregnancy == num_new_born
                msg = "found {0} new pregancies nine months before bisth at " \
                      "time step {1}, number of new birth is {2}.".format(
                    num_pregnancy, t, num_new_born)
                if not result:
                    logging.info(msg)
                    error_msg.append(msg)
                    succeed = False
                    # self.assertTrue(result, msg)
            except Exception as ex:
                msg = "can't find new pregnancy nine months before birth at " \
                      "time step {0}, got exception {1}".format(t, ex)
                logging.info(msg)
                error_msg.append(msg)
                succeed = False
                # raise Exception(msg)
        if self.new_pregnancy:
            for t in self.new_pregnancy:
                result = t + Constant.nine_months > duration - 1
                num_pregnancy = self.new_pregnancy[t]
                msg = "{0} individuals got pregnant at time step {1}, can't find bith after " \
                      "nine months at time step {2}".format(
                    num_pregnancy, t, t + Constant.nine_months)
                if not result:
                    logging.info(msg)
                    error_msg.append(msg)
                    succeed = False
                    # self.assertTrue(result,msg)
        return succeed, error_msg

    def test_fixed_rate(self, rate=0.1, x_Birth=10, initial_population=100, duration=4 * Constant.days_per_year):
        logging.info("test fixed birth rate begins")
        print("test fixed birth rate begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.FIXED_BIRTH_RATE,
                                  rate=rate, x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, debug=True)
        path = "./"
        test_name = "fixed_rate"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test birth rate at each timestep")
        rate_per_day = rate * x_Birth
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        for i in range(len(baby_count)):
            # test boy and girl seperately
            gender = "boy" if i == 0 else "girl"
            msg = "test_poissen for new born: {0}.".format(gender)
            # numpy_poisson= np.random.poisson(rate_per_day/2.0, int(boy_count))
            # sft.plot_cdf(baby_count[0],numpy_poisson)
            with open(os.path.join(path, test_name, "poisson_test.txt"), "a") as file:
                # self.assertTrue(sft.test_poisson(baby_count[i], rate_per_day / 2.0, report_file=file, route=gender),msg)
                result = sft.test_poisson(baby_count[i], rate_per_day / 2.0, report_file=file, route=gender)
                if not result:
                    error_msg.append(msg + "with rate = " + str(rate_per_day / 2.0) + " result is " + str(result))
                    succeed = False
                file.write("kstest for {0} result is {1}.\n".format(gender, succeed))

        print("test total new born")
        result, msg = self.total_birth_count_test(rate_per_day, duration, boy_count, girl_count)
        if not result:
            error_msg.append(msg)
            succeed = False

        print("test male/female ratio")
        result, mag = self.male_female_ratio_test(boy_count,girl_count)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test fixed birth rate passes")
        print("test fixed birth rate passes")

    def test_population_dep_rate(self, rate=0.0001, x_Birth=20, initial_population=1000,
                                 duration=2 * Constant.days_per_year):
        logging.info("test population dep birth rate begins")
        print("test population dep birth rate begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.POPULATION_DEP_RATE,
                                  rate=rate, x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, debug=True)
        path = "./"
        test_name = "population_dep"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test birth rate at each timestep")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        expected_new_born = 0
        ci_error_count = 0
        ci_error = []
        for t in range(duration):
            if rate > Constant.birthrate_threshold:
                rate_per_day = x_Birth * self.statistical_population[t] * Constant.two_percent_per_year
            else:
                rate_per_day = x_Birth * self.statistical_population[t] * rate

            for i in range(2):
                gender = "boy" if i == 0 else "girl"
                # print "test new born: {0} per day.".format(gender)
                low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, rate_per_day / 2.0, 0)
                with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as file:
                    result = True if low_ci <= baby_count[i][t] <= high_ci else False
                    msg = "at time step {0}, test Poisson {1} confidence interval for new born {2} " \
                          "result is {3}.\n".format(t, Constant.ci_probability, gender, result) \
                          + "new born count is {0}, while Poisson {1} confidence interval is " \
                            "{2} - {3}.\n".format(baby_count[i][t], Constant.ci_probability, low_ci, high_ci)
                    if not result:
                        file.write(msg)
                        ci_error_count += 1
                        ci_error.append(msg)
            expected_new_born += rate_per_day
        if float(ci_error_count) / duration > 0.05:
            error_msg = ci_error
            succeed = False
        logging.info(
            "test Poisson {0} condifence interval for new born result is {1}.".format(Constant.ci_probability, succeed))

        self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                             xlabel="time step", ylabel="new births per day",
                             title="new births per day",
                             category=os.path.join(path, test_name, "new_births"))

        print("test total new born")
        result, msg = self.total_birth_count_test(expected_new_born, 1.0, boy_count,
                                                  girl_count)  # rate_per_day is the sum of rate each day, so duration = 1.0
        if not result:
            error_msg.append(msg)
            succeed = False
        # error_tolerance = 2e-2 * expected_new_born
        # msg = "test total new born, expected new born {0}, actual nre born {1}.".format(expected_new_born, boy_count + girl_count)
        # logging.info(msg)
        # self.assertTrue(math.fabs(expected_new_born - boy_count - girl_count) <= error_tolerance, msg=msg)

        print("test male/female ratio")
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            error_msg.append(msg)
            succeed = False

        print("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test population dep birth rate passes")
        print("test population dep birth rate passes")

    def test_demographic_dep_rate(self, rate=0.0001, x_Birth=20, initial_population=1000, duration=4 * 365):
        logging.info("test demographic dep birth rate begins")
        print("test demographic dep birth rate begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.DEMOGRAPHIC_DEP_RATE, rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, debug=self.debug)
        path = "./"
        test_name = "demographic_dep"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test birth rate at each timestep")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        expected_new_born = 0
        for t in range(duration):
            if rate > Constant.birthrate_threshold:
                rate_per_day = x_Birth * self.possible_mother[t] * Constant.individual_birthrate
            else:
                rate_per_day = x_Birth * self.possible_mother[t] * rate
            for i in range(2):
                gender = "boy" if i == 0 else "girl"
                low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, rate_per_day / 2.0, 0)
                with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as file:
                    result = True if low_ci <= baby_count[i][t] <= high_ci else False
                    msg = "at time step {0}, test Poisson {1} confidence interval for new born {2} " \
                          "result is {3}.\n".format(t, Constant.ci_probability, gender, result) \
                          + "new born count is {0}, while Poisson {1} confidence interval is " \
                            "{2} - {3}.\n".format(baby_count[i][t], Constant.ci_probability, low_ci, high_ci)
                    if not result:
                        file.write(msg)
                        error_msg.append(msg)
                        succeed = False
                        # self.assertTrue(result, msg)
            expected_new_born += rate_per_day

        logging.info(
            "test Poisson {0} condifence interval for new born result is {1}.".format(Constant.ci_probability, succeed))

        print("test total new born")
        result, msg = self.total_birth_count_test(expected_new_born, 1.0, boy_count, girl_count) # expected_new_born is the sum of rate each day, so duration = 1.0
        if not result:
            error_msg.append(msg)
            succeed = False

        print("test male/female ratio")
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            error_msg.append(msg)
            succeed = False

        print("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test demograohic dep birth rate passes")
        print("test demograohic dep birth rate passes")

    def test_individual_pregnancies(self, rate=0.000548, x_Birth=100, initial_population=1000, #rate=0.000548
                                    duration=4 * Constant.days_per_year):
        logging.info("test individual pregnancies birth rate begins")
        print("test individual pregnancies birth rate begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES, rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, debug=True)

        path = "./"
        test_name = "individual_pregnancies"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test new pregnancy at each timestep")
        expected_new_pregnancies = []
        total_new_pregnancies = 0
        ci_error_count = 0
        ci_error = []
        for t in range(duration):
            if t in self.new_pregnancy:
                num_pregnancy = self.new_pregnancy[t]
                total_new_pregnancies += num_pregnancy
            else:
                num_pregnancy = 0
            if rate > Constant.birthrate_threshold:
                rate_per_day = x_Birth * self.possible_mother[t] * Constant.individual_birthrate
            else:
                rate_per_day = x_Birth * self.possible_mother[t] * rate

            low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, rate_per_day, 0)
            with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as file:
                result = True if low_ci <= num_pregnancy <= high_ci else False
                msg = "at time step {0}, test Poisson {1} confidence interval for number of new pregnancies " \
                      "result is {2}.".format(t, Constant.ci_probability, result) \
                      + "new pregnancies count is {0}, while Poisson {1} confidence interval is " \
                        "{2} - {3}.\n".format(num_pregnancy, Constant.ci_probability, low_ci, high_ci)
                if not result:
                    file.write(msg)
                    ci_error_count += 1
                    ci_error.append(msg)

                # self.assertTrue(result, msg)
            expected_new_pregnancies.append(rate_per_day)
        if float(ci_error_count)/duration > 0.05:
            error_msg = ci_error
            succeed = False
        logging.info("test Poisson {0} condifence interval for new pregnancies result is {1}.".format(Constant.ci_probability, succeed))

        print("test accumulative new pregnancies")
        error_tolerance = 5e-2 * sum(expected_new_pregnancies)
        msg = "expected_new_pregnancies is {0} while total_new_pregnancies is {1}.".format(sum(expected_new_pregnancies), total_new_pregnancies)
        logging.info(msg)
        if math.fabs(sum(expected_new_pregnancies) - total_new_pregnancies) > error_tolerance:
            succeed = False
            error_msg.append(msg)
        # self.assertTrue(math.fabs(expected_new_pregnancies - total_new_pregnancies) <= error_tolerance, msg)

        new_pregnancy_for_plot = []
        for t in range(duration):
            if t in self.new_pregnancy:
                new_pregnancy_for_plot.append(self.new_pregnancy[t])
            else:
                new_pregnancy_for_plot.append(0)
        if self.debug:
            with open(os.path.join(path, test_name, "new_pregnancy_for_plot.json"), 'w') as file:
                file.write(json.dumps(new_pregnancy_for_plot, indent=4, sort_keys=False))

        sft.plot_data(new_pregnancy_for_plot, expected_new_pregnancies,
                          category=os.path.join(path, test_name, "new_pregnancy"),
                          label1="actual", label2="expected",
                          xlabel="time step", ylabel="new pregnancy per day",
                          title="new pregnancy per day")

        print("test male/female ratio")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)

        # print "test total new born" # this is tested by the next section
        # total_pregnancy = total_new_born = 0
        # for t in self.new_pregnancy:
        #     if t <= duration - Constant.nine_months:
        #         print t, duration - Constant.nine_months, total_pregnancy
        #         total_pregnancy += self.new_pregnancy[t]
        # for t in self.nursery:
        #         total_new_born += sum(self.nursery[t])
        # self.assertTrue(total_pregnancy==total_new_born, "got total_new_born = {0}, expected {1}.".format(total_new_born, total_pregnancy))

        print("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test individual pregnancies birth rate passes")
        print("test individual pregnancies birth rate passes")

    def test_individual_pregnancies_boxcar(self, rate=0.00085, x_Birth=100, initial_population=5000,
                                           duration=2 * Constant.days_per_year, amplitude=2, start_time=90, end_time=180,
                                           mode=Boxcar_S.Modes.LESS):
        logging.info("test individual pregnancies boxcar birth rate begins")
        print("test individual pregnancies boxcar birth rate begins")
        self.debug = True
        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES, rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.configure_birth_rate_time_dependence(Birth_Rate_Time_Dependence=ConfigParameters.ANNUAL_BOXCAR_FUNCTION,
                                                  amplitude=amplitude, param1=start_time, param2=end_time)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=False, debug=self.debug)
        path = "./"
        test_name = "individual_pregnancies_boxcar"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test new pregnancy at each timestep")
        expected_new_pregnancies = []
        total_new_pregnancies = 0
        ci_error_count = 0
        ci_error = []
        for t in range(duration):
            if t in self.new_pregnancy:
                num_pregnancy = self.new_pregnancy[t]
                total_new_pregnancies += num_pregnancy
            else:
                num_pregnancy = 0

            if rate > Constant.birthrate_threshold:
                rate_per_day = x_Birth * Constant.individual_birthrate
            else:
                rate_per_day = x_Birth * rate

            day_in_year = t % 365
            if mode == Boxcar_S.Modes.LESS:
                if start_time <= day_in_year <= end_time:
                    rate_per_day *= (1.0 + amplitude)
            elif mode == Boxcar_S.Modes.LARGER:
                if day_in_year <= end_time or day_in_year >= start_time:
                    rate_per_day *= (1.0 + amplitude)

            rate_per_day *= self.possible_mother[t]

            low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, rate_per_day, 0)
            with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as file:
                result = True if low_ci <= num_pregnancy <= high_ci else False
                msg = "at time step {0}, test Poisson {1} confidence interval for number of new pregnancies " \
                      "result is {2}.".format(t, Constant.ci_probability, result) \
                      + "new pregnancies count is {0}, while Poisson {1} confidence interval is " \
                        "{2} - {3}.\n".format(num_pregnancy, Constant.ci_probability, low_ci, high_ci)
                if not result:
                    file.write(msg)
                    ci_error_count += 1
                    ci_error.append(msg)

                # self.assertTrue(result, msg)
            expected_new_pregnancies.append(rate_per_day)
        if float(ci_error_count)/duration > 0.05:
            error_msg = ci_error
            succeed = False
        logging.info("test Poisson {0} condifence interval for new pregnancies result is {1}.".format(Constant.ci_probability, succeed))

        print("test accumulative new pregnancies")
        error_tolerance = 5e-2 * sum(expected_new_pregnancies)
        msg = "expected_new_pregnancies is {0} while total_new_pregnancies is {1}.".format(sum(expected_new_pregnancies),
                                                                                           total_new_pregnancies)
        logging.info(msg)
        if math.fabs(sum(expected_new_pregnancies) - total_new_pregnancies) > error_tolerance:
            succeed = False
            error_msg.append(msg)
        # self.assertTrue(math.fabs(expected_new_pregnancies - total_new_pregnancies) <= error_tolerance, msg)

        new_pregnancy_for_plot = []
        for t in range(duration):
            if t in self.new_pregnancy:
                new_pregnancy_for_plot.append(self.new_pregnancy[t])
            else:
                new_pregnancy_for_plot.append(0)
        if self.debug:
            with open(os.path.join(path, test_name, "new_pregnancy_for_plot.json"), 'w') as file:
                file.write(json.dumps(new_pregnancy_for_plot, indent=4, sort_keys=False))

        sft.plot_data(new_pregnancy_for_plot, expected_new_pregnancies,
                          category=os.path.join(path, test_name, "new_pregnancy"),
                          label1="actual", label2="expected",
                          xlabel="time step", ylabel="new pregnancy per day",
                          title="new pregnancy per day")

        print("test male/female ratio")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)
        self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                             xlabel="time step", ylabel="new births per day",
                             title="new births per day",
                             category=os.path.join(path, test_name, "new_births"))

        print("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test individual pregnancies boxcar birth rate passes")
        print("test individual pregnancies boxcar birth rate passes")

    def test_individual_pregnancies_sinusoidal(self, rate=0.00085, x_Birth=100, initial_population=1000,
                                           duration=4 * Constant.days_per_year, amplitude=0.5, phase=180):
        logging.info("test individual pregnancies sinusoidal birth rate begins")
        print("test individual pregnancies sinusoidal birth rate begins")
        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES, rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population)
        self.configure_birth_rate_time_dependence(Birth_Rate_Time_Dependence=ConfigParameters.SINUSOIDAL_FUNCTION_OF_TIME,
                                                  amplitude=amplitude, param1=phase)
        self.enable_birth()

        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=False, debug=self.debug)
        path = "./"
        test_name = "individual_pregnancies_sinusoidal"
        self.save_files(graveyard, path, test_name, self.debug)

        error_msg = []
        succeed = True
        print("test new pregnancy at each timestep")
        expected_new_pregnancies = []
        total_new_pregnancies = 0
        ci_error_count = 0
        ci_error = []
        for t in range(duration):
            if t in self.new_pregnancy:
                num_pregnancy = self.new_pregnancy[t]
                total_new_pregnancies += num_pregnancy
            else:
                num_pregnancy = 0

            if rate > Constant.birthrate_threshold:
                rate_per_day = x_Birth * self.possible_mother[t] * Constant.individual_birthrate
            else:
                rate_per_day = x_Birth * self.possible_mother[t] * rate
            day_in_year = t % 365
            rate_per_day *=  1.0 + amplitude * math.sin(2.0 * math.pi * (day_in_year - phase) / Constant.days_per_year)

            low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, rate_per_day, 0)
            with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as file:
                result = True if low_ci <= num_pregnancy <= high_ci else False
                msg = "at time step {0}, test Poisson {1} confidence interval for number of new pregnancies " \
                      "result is {2}.".format(t, Constant.ci_probability, result) \
                      + "new pregnancies count is {0}, while Poisson {1} confidence interval is " \
                        "{2} - {3}.\n".format(num_pregnancy, Constant.ci_probability, low_ci, high_ci)
                file.write(msg)
                if not result:
                    ci_error_count += 1
                    ci_error.append(msg)

                # self.assertTrue(result, msg)
            expected_new_pregnancies.append(rate_per_day)
        if float(ci_error_count)/duration > 0.05:
            error_msg = ci_error
            succeed = False
        logging.info("test Poisson {0} condifence interval for new pregnancies result is {1}.".format(Constant.ci_probability, succeed))

        print("test accumulative new pregnancies")
        error_tolerance = 1e-1 * sum(expected_new_pregnancies)
        msg = "expected_new_pregnancies is {0} while total_new_pregnancies is {1}.".format(sum(expected_new_pregnancies),
                                                                                           total_new_pregnancies)
        logging.info(msg)
        if math.fabs(sum(expected_new_pregnancies) - total_new_pregnancies) > error_tolerance:
            succeed = False
            error_msg.append(msg)
        # self.assertTrue(math.fabs(expected_new_pregnancies - total_new_pregnancies) <= error_tolerance, msg)

        new_pregnancy_for_plot = []
        for t in range(duration):
            if t in self.new_pregnancy:
                new_pregnancy_for_plot.append(self.new_pregnancy[t])
            else:
                new_pregnancy_for_plot.append(0)
        if self.debug:
            with open(os.path.join(path, test_name, "new_pregnancy_for_plot.json"), 'w') as file:
                file.write(json.dumps(new_pregnancy_for_plot, indent=4, sort_keys=False))
            with open(os.path.join(path, test_name, "expected_new_pregnancies.json"), 'w') as file:
                file.write(json.dumps(expected_new_pregnancies, indent=4, sort_keys=False))

        sft.plot_data(new_pregnancy_for_plot, expected_new_pregnancies,
                          category=os.path.join(path, test_name, "new_pregnancy"),
                          label1="actual", label2="expected",
                          xlabel="time step", ylabel="new pregnancy per day",
                          title="new pregnancy per day")

        print("test male/female ratio")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)
        self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                             xlabel="time step", ylabel="new births per day",
                             title="new births per day",
                             category=os.path.join(path, test_name, "new_births"))

        print("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        self.assertTrue(succeed, error_msg)
        logging.info("test individual pregnancies sinusoidal birth rate passes")
        print("test individual pregnancies sinusoidal birth rate passes")

    def _test_individual_pregnancies_by_age_year(self, rate=0.001, x_Birth=5, initial_population=1000,
                                                 duration=1 * Constant.days_per_year, debug=debug):

        logging.info("test individual pregnancies by age and year birth rate begins")
        print("test individual pregnancies by age and year birth rate begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR,
                                  rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population,
                                  demo_template_filename="demographics_fertility_by_age_and_year_template.json")
        self.enable_birth()

        # PopulationGroups = [[15, 16, 17, 18, 19, 20, 21],
        #                     [2015, 2016, 2017, 2018, 2019, 2020, 2021]]
        # ResultScaleFactor = 1e-6
        # ResultValues = [[0, 0, 0, 0, 0, 0, 0],
        #                 [0, 0, 0, 0, 0, 0, 0],
        #                 [0, 0, 0, 0, 0, 0, 0],
        #                 [1000, 0, 0, 0, 0, 0, 0],
        #                 [0, 0, 0, 0, 0, 0, 0],
        #                 [0, 0, 0, 0, 0, 0, 0],
        #                 [0, 0, 0, 0, 0, 0, 0]]


        PopulationGroups = [[15, 16, 17, 18, 19, 20, 21],
                            [2015]]
        ResultScaleFactor = 1e-6
        ResultValues = [[2000],
                        [0],
                        [0],
                        [2000],
                        [0],
                        [0],
                        [0]]
        self.set_fertility_distribution(PopulationGroups, ResultScaleFactor, ResultValues)
        age_values = [15, 16, 17, 18, 19]
        age_distribution = [0, 0.25, 0.5, 0.75, 1]
        self.set_age_distribution(ResultValues=age_values, DistributionValues=age_distribution,
                                  age_distribution_type=ConfigParameters.DISTRIBUTION_COMPLEX)

        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=True, debug=True)
        path = "./"
        test_name = "individual_pregnancies_by_age_year"
        self.save_files(graveyard, path, test_name, self.debug)
        error_msg = []
        succeed = True

        print("test birth rate at each timestep")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)

        if debug:
            category = os.path.join(path, test_name, "new_births")
            self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                                 xlabel="time step", ylabel="new births per day",
                                 title="new births per day",
                                 category=category)
            with open(os.path.join(path, test_name, "possible_mother_by_age_time.json"), "w") as file:
                file.write(json.dumps(self.possible_mom_by_age_time, indent=4, sort_keys=True))

        new_pregnancies = {}
        for t in range(duration):
            possible_mother = self.possible_mom_by_age_time[t]
            for age in possible_mother:
                birthrate = ResultValues[PopulationGroups[0].index(int(age))][0]
                rate_per_day = x_Birth * possible_mother[age] * birthrate * ResultScaleFactor
                if age in new_pregnancies:
                    new_pregnancies[age] += rate_per_day
                else:
                    new_pregnancies[age] = rate_per_day
        print(new_pregnancies)

    def test_nondisease_mortality_by_age_and_gender_2(self, initial_population=6000, duration=1 * Constant.days_per_year,
                                                      debug = debug):
        logging.info("test nondisease mortality by age and gender 2 begins")
        print("test nondisease mortality by age and gender 2 begins")
        x_other_mortality = 0.5
        self.configure_death_rate(Death_Rate_Dependence=ConfigParameters.NONDISEASE_MORTALITY_BY_AGE_AND_GENDER,
                                  x_other_mortality=x_other_mortality)
        PopulationGroups = [[0, 1], [15, 19.999999, 20, 24.99999, 25, 25.99999]]
        ResultScaleFactor = 4
        ResultValues = [
            [0.1, 0.1, 0.01, 0.01, 0.5, 0.5],
            [0.05, 0.05, 0.1, 0.1, 0.005, 0.005]
        ]
        self.set_mortality_distribution(PopulationGroups, ResultScaleFactor,
                                        ResultValues, initial_population,
                                        demo_filename="demographics_mortality_by_age_and_gender_template.json")
        age_values = [17, 17, 18, 18, 21, 21, 22, 22]
        age_distribution = [0, 0.25, 0.25, 0.5, 0.5, 0.75, 0.75, 1]
        self.set_age_distribution(ResultValues=age_values, DistributionValues=age_distribution,
                                  age_distribution_type=ConfigParameters.DISTRIBUTION_COMPLEX)

        graveyard = self.run_test(duration=duration, death_by_age_gender_flag=True, debug=debug)
        path = "./"
        test_name = "nondisease_mortality_by_age_and_gender_2"
        self.save_files(graveyard, path, test_name, self.debug)
        error_msg = []
        succeed = True

        print("collect death information by age and gender from graveyard")
        deaths_by_age = {"male":{},"female":{}}
        for i in range(int(age_values[0]), int(age_values[-1] + math.ceil(float(duration)/Constant.days_per_year))):
            deaths_by_age["male"][i] = []
            deaths_by_age["female"][i] = []
        for t in range(duration):
            deaths_per_timestep_by_age = {"male": {}, "female": {}}
            if t in graveyard:
                graveyard_per_timestep = graveyard[t]
                for human in graveyard_per_timestep:
                    age_year = int((human["age"] + t) / Constant.days_per_year)
                    gender = human["sex"]
                    sex = "male" if gender == 0 else "female"
                    if age_year not in deaths_per_timestep_by_age[sex]:
                        deaths_per_timestep_by_age[sex][age_year] = 1
                    else:
                        deaths_per_timestep_by_age[sex][age_year] += 1

            for sex in deaths_by_age:
                for age in deaths_by_age[sex]:
                    if age in deaths_per_timestep_by_age[sex]:
                        deaths_by_age[sex][age].append(deaths_per_timestep_by_age[sex][age])
                    else:
                        deaths_by_age[sex][age].append(0)
        if debug:
            with open(os.path.join(path, test_name, "death_by_age.json"), "w") as outfile:
                json.dump(deaths_by_age, outfile, sort_keys=True, indent=4)
            # for sex in self.population_by_age_gender:
            #     with open(os.path.join(path, test_name, "population_by_age_{}.csv".format(sex)), "w") as outfile:
            #         w = csv.DictWriter(outfile, self.population_by_age_gender[sex].keys())
            #         w.writerow(self.population_by_age_gender[sex])
            with open(os.path.join(path, test_name, "population_by_age.json"), "w") as outfile:
                json.dump(self.population_by_age_gender, outfile, sort_keys=True, indent=4)

        print ("test death rate for every time step")
        expected_death_by_age_gender = {"male": {}, "female": {}}
        actual_death_by_age_gender = {"male": {}, "female": {}}
        for sex in deaths_by_age:
            for age in deaths_by_age[sex]:
                expected_death_by_age_gender[sex][age] = 0
                actual_death_by_age_gender[sex][age] = 0
                for t in range(duration):
                    death = deaths_by_age[sex][age][t]
                    population = self.population_by_age_gender[sex][age][t]
                    if population == 0:
                        if self.debug:
                            print("sex: {0} age: {1} at timestep {2} has no population, "
                                  "continue to next timestep".format(sex, age, t))
                        continue
                    i = 0
                    for age_bucket in PopulationGroups[1]:
                        # print age, age_bucket, i
                        if age < age_bucket:
                            break
                        i += 1
                    j = 0 if sex == "male" else 1
                    death_rate = ResultValues[j][i - 1] * ResultScaleFactor * population * x_other_mortality
                    expected_death_by_age_gender[sex][age] += death_rate
                    actual_death_by_age_gender[sex][age] += death
                    low_ci, high_ci = scipy.stats.poisson.interval(Constant.ci_probability, death_rate, 0)
                    with open(os.path.join(path, test_name, "test_poisson_99ci.txt"), "a") as outfile:
                        result = True if low_ci <= death <= high_ci else False
                        msg = "at time step {0}, test Poisson {1} confidence interval for sex {2} age {3} " \
                              "result is {4}.\n".format(t, Constant.ci_probability, sex, age, result) \
                              + "death count is {0}, while Poisson {1} confidence interval is " \
                                "{2} - {3}.\n".format(death, Constant.ci_probability, low_ci, high_ci)
                        if debug:
                            outfile.write(msg)
                        if not result:
                            error_msg.append(msg)
                            succeed = False

        logging.info("test Poisson {0} confidence interval for death for every time step, result is {1}.".format(
            Constant.ci_probability, succeed))

        print("test total death by age and gender")
        for sex in expected_death_by_age_gender:
            for age in expected_death_by_age_gender[sex]:
                expected_death = expected_death_by_age_gender[sex][age]
                actual_death = actual_death_by_age_gender[sex][age]
                error_tolerance = math.ceil(2e-1 * expected_death)
                result = math.fabs(expected_death - actual_death) <= error_tolerance
                msg = "Sex {0} age {1} actual deaths are {2}, while expected deaths are {3}, expected difference " \
                      "is less than {4}, test result is {5}".format(sex, age, actual_death, expected_death,
                                                                    error_tolerance, result)
                if not result:
                    succeed = False
                    error_msg.append(msg)
                logging.info(msg)

        print ("test no birth")
        result, msg = self.no_birth_test()
        if not result:
            error_msg.append(msg)
            succeed = False

        with open(os.path.join(path, test_name, "result.txt"), "a") as outfile:
            for line in error_msg:
                outfile.write(line)
        self.assertTrue(succeed, "test nondisease mortality by age and gender 2 fails, please see {0}"
                                 " for error message".format(os.path.join(path, test_name, "result.txt")))

        logging.info("test nondisease mortality by age and gender 2 passes")
        print("test nondisease mortality by age and gender 2 passes")


if __name__ == "__main__":
    unittest.main()
