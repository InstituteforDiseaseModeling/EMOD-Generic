#!/usr/bin/python
import sys
#sys.path.append( "/var/www/wsgi/" ) # this no scale
#sys.path.append( "../../individuals/build/lib.win-amd64-2.7" )
#sys.path.append('../../../Python/')
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
import dtk_generic_intrahost as gi
import dtk_test.dtk_InfectivityScalingBoxcar_Support as Boxcar_S

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


    def test_individual_pregnancies_by_age_year_2(self, rate=0.0015, x_Birth=75, initial_population=2000,
                                                   duration=3 * Constant.days_per_year, debug=True):
        # test year
        logging.info("test individual pregnancies by age and year birth rate 2 begins")
        print("test individual pregnancies by age and year birth rate 2 begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR,
                                  rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population,
                                  demo_template_filename="demographics_fertility_by_age_and_year_template.json")
        self.enable_birth()

        PopulationGroups = [[0],
                            [0, 0.999999, 1, 1.999999, 2, 2.999999]]
        ResultScaleFactor = 1e-6
        ResultValues = [[100, 100, 50, 50, 200, 200]]
        self.set_fertility_distribution(PopulationGroups, ResultScaleFactor, ResultValues)
        age_values = [15, 15, 16, 16, 17, 17, 18, 18, 19, 19]
        age_distribution = [0, 0.2, 0.2, 0.4, 0.4, 0.6, 0.6, 0.8, 0.8, 1]
        base_year = 0
        self.set_base_year( base_year )
        self.set_age_distribution(ResultValues=age_values, DistributionValues=age_distribution,
                                  age_distribution_type=ConfigParameters.DISTRIBUTION_COMPLEX)

        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=False, debug=True)
        path = "./"
        test_name = "individual_pregnancies_by_age_year_2"
        self.save_files(graveyard, path, test_name, self.debug)
        error_msg = []
        succeed = True

        print ("accumulate data by age and year")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        if debug:
            category = os.path.join(path, test_name, "new_births")
            self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                                 xlabel="time step", ylabel="new births per day",
                                 title="new births per day",
                                 category=category)
            with open(os.path.join(path,test_name, "possible_mother_by_age_time.json"), "w") as file:
                file.write(json.dumps(self.possible_mom_by_age_time, indent=4, sort_keys=True))

        expected_new_pregnancies = {}
        total_new_pregnancies = {}
        for t in range(duration):
            sim_year = int(t/Constant.days_per_year + base_year)
            if t in self.new_pregnancy:
                num_pregnancy = self.new_pregnancy[t]
                if sim_year in total_new_pregnancies:
                    total_new_pregnancies[sim_year] += num_pregnancy
                else:
                    total_new_pregnancies[sim_year] = num_pregnancy
            else:
                num_pregnancy = 0
            birthrate = ResultValues[0][PopulationGroups[1].index(sim_year)]
            rate_per_day = x_Birth * self.possible_mother[t] * birthrate * ResultScaleFactor

            if sim_year in expected_new_pregnancies:
                expected_new_pregnancies[sim_year] += rate_per_day
            else:
                expected_new_pregnancies[sim_year] = rate_per_day

        print("test accumulative new pregnancies")
        for sim_year in expected_new_pregnancies:
            expected_new_pregnancy = expected_new_pregnancies[sim_year]
            actual_new_pregnancy = total_new_pregnancies[sim_year]
            error_tolerance = 2e-1 * expected_new_pregnancy
            msg = "In sim year {0}, expected_new_pregnancies is {1} while total_new_pregnancies is" \
                  " {2}.\n".format(sim_year, expected_new_pregnancy, actual_new_pregnancy)
            logging.info(msg)
            if math.fabs(expected_new_pregnancy - actual_new_pregnancy) > error_tolerance:
                succeed = False
                error_msg.append(msg)

        print("test male/female ratio")
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)

        print("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration, birth_by_age_year_flag=False)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print ("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        with open(os.path.join(path, test_name, "result.txt"), "a") as outfile:
            for line in error_msg:
                outfile.write(line)

        self.assertTrue(succeed, "test individual pregnancies by age and year birth rate 2 fails, please see {0}"
                                 " for error message.".format(os.path.join(path, test_name, "result.txt")))
        logging.info("test individual pregnancies by age and year birth rate 2 passes")
        print("test individual pregnancies by age and year birth rate 2 passes")

    def test_individual_pregnancies_by_age_year_1(self, rate=0.001, x_Birth=15, initial_population=5000,
                                                  duration=2 * Constant.days_per_year, debug=debug):
        # test Age
        logging.info("test individual pregnancies by age and year birth rate 1 begins")
        print("test individual pregnancies by age and year birth rate 1 begins")

        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR,
                                  rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population,
                                  demo_template_filename="demographics_fertility_by_age_and_year_template.json")
        self.enable_birth()
        PopulationGroups = [[0, 14, 15, 16, 17, 18, 19],
                            [2015, 2016]]
        ResultScaleFactor = 1e-6
        ResultValues = [[0, 0],
                        [0, 0],
                        [50, 50],
                        [0, 0],
                        [200, 200],
                        [100, 100],
                        [0, 0]]
        self.set_fertility_distribution(PopulationGroups, ResultScaleFactor, ResultValues)
        age_values = [14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19]
        age_distribution = [0, 0.17, 0.17, 0.34, 0.34, 0.51, 0.51, 0.68, 0.68, 0.84, 0.84, 1]
        self.set_age_distribution(ResultValues=age_values, DistributionValues=age_distribution,
                                  age_distribution_type=ConfigParameters.DISTRIBUTION_COMPLEX)

        base_year = 2015
        self.set_base_year( base_year )
        
        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=True, debug=True)
        path = "./"
        test_name = "individual_pregnancies_by_age_year_1"
        self.save_files(graveyard, path, test_name, self.debug)
        error_msg = []
        succeed = True

        print ("accumulate data by age and year")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)

        if debug:
            category = os.path.join(path, test_name, "new_births")
            self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                                 xlabel="time step", ylabel="new births per day",
                                 title="new births per day",
                                 category=category)
            with open(os.path.join(path, test_name, "possible_mother_by_age_time.json"), "w") as file:
                file.write(json.dumps(self.possible_mom_by_age_time, indent=4, sort_keys=True))

        expected_new_pregnancies = {}
        actual_new_pregnancies = {}
        for year in PopulationGroups[1]:
            expected_new_pregnancies[year] = {}
            actual_new_pregnancies[year] = {}
        for t in range(duration):
            possible_mother = self.possible_mom_by_age_time[t]
            sim_year = t / Constant.days_per_year + base_year
            for age in possible_mother:
                i = 0
                for age_bucket in PopulationGroups[0]:
                    if age < age_bucket:  # age <= age_bucket:
                        break
                    i += 1
                birthrate_right = ResultValues[i][PopulationGroups[1].index(int(sim_year))] if i < len(ResultValues) \
                    else ResultValues[-1][PopulationGroups[1].index(int(sim_year))]
                birthrate_left = ResultValues[i - 1][PopulationGroups[1].index(int(sim_year))] if i - 1 >= 0 \
                    else ResultValues[0][PopulationGroups[1].index(int(sim_year))]
                birthrate = (age - PopulationGroups[0][i - 1]) / (PopulationGroups[0][i] - PopulationGroups[0][i - 1]) * \
                            (birthrate_right - birthrate_left) + birthrate_left if i < len(
                    ResultValues) else birthrate_left
                # birthrate = ResultValues[i-1][PopulationGroups[1].index(int(sim_year))]
                rate_per_day = x_Birth * possible_mother[age] * birthrate * ResultScaleFactor
                num_pregnancy = self.new_pregnancy[t][age] if t in self.new_pregnancy and age in self.new_pregnancy[
                    t] else 0

                if age in expected_new_pregnancies[int(sim_year)]:
                    expected_new_pregnancies[int(sim_year)][age] += rate_per_day
                else:
                    expected_new_pregnancies[int(sim_year)][age] = rate_per_day

                if age in actual_new_pregnancies[int(sim_year)]:
                    actual_new_pregnancies[int(sim_year)][age] += num_pregnancy
                else:
                    actual_new_pregnancies[int(sim_year)][age] = num_pregnancy
        if debug:
            with open(os.path.join(path, test_name, "expected_new_pregnancies.json"), "w") as file:
                json.dump(expected_new_pregnancies, file, indent=4, sort_keys=True)
            with open(os.path.join(path, test_name, "actual_new_pregnancies.json"), "w") as file:
                json.dump(actual_new_pregnancies, file, indent=4, sort_keys=True)

        print("test accumulative new pregnancies")
        expected_new_pregnancy_by_age = {}
        actual_new_pregnancy_by_age = {}
        for sim_year in expected_new_pregnancies:
            for age in expected_new_pregnancies[sim_year]:
                age_year = math.floor(age)
                if age_year in expected_new_pregnancy_by_age:
                    expected_new_pregnancy_by_age[age_year] += expected_new_pregnancies[sim_year][age]
                else:
                    expected_new_pregnancy_by_age[age_year] = expected_new_pregnancies[sim_year][age]
                if age_year in actual_new_pregnancy_by_age:
                    actual_new_pregnancy_by_age[age_year] += actual_new_pregnancies[sim_year][age]
                else:
                    actual_new_pregnancy_by_age[age_year] = actual_new_pregnancies[sim_year][age]
        for age in expected_new_pregnancy_by_age:
            expected_new_pregnancy = expected_new_pregnancy_by_age[age]
            actual_new_pregnancy = actual_new_pregnancy_by_age[age]
            error_tolerance = 2e-1 * expected_new_pregnancy
            print(expected_new_pregnancy, actual_new_pregnancy, age)
            msg = "for age {0}, expected_new_pregnancies is {1} while total_new_pregnancies is" \
                  " {2}.\n".format(age, expected_new_pregnancy, actual_new_pregnancy)
            logging.info(msg)
            if math.fabs(expected_new_pregnancy - actual_new_pregnancy) > error_tolerance:
                succeed = False
                error_msg.append(msg)

        print("test male/female ratio")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)

        print("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration, birth_by_age_year_flag=True)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print ("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        with open(os.path.join(path, test_name, "result.txt"), "a") as outfile:
            for line in error_msg:
                outfile.write(line)
        self.assertTrue(succeed, "test individual pregnancies by age and year birth rate 1 fails, please see {0}"
                                 " for error message.".format(os.path.join(path, test_name, "result.txt")))
        logging.info("test individual pregnancies by age and year birth rate 1 passes")
        print("test individual pregnancies by age and year birth rate 1 passes")

    def test_individual_pregnancies_by_age_year_3(self, rate=0.001, x_Birth=15, initial_population=4000,
                                                   duration=5 * Constant.days_per_year, debug=debug):
        # test Age and year
        logging.info("test individual pregnancies by age and year birth rate 3 begins")
        print("test individual pregnancies by age and year birth rate 3 begins")
        self.configure_birth_rate(Birth_Rate_Dependence=ConfigParameters.INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR,
                                  rate=rate,
                                  x_Birth=x_Birth,
                                  initial_population=initial_population,
                                  demo_template_filename="demographics_fertility_by_age_and_year_template.json")
        PopulationGroups = [[14, 14.999999, 15, 16.999999, 17, 17.999999, 18, 18.999999, 19, 25.999999],
                            [0, 1, 2, 4, 5, 7]]
        ResultScaleFactor = 6.789e-6
        ResultValues = [[25, 15, 30, 52, 15, 35],
                        [25, 15, 30, 52, 15, 35],
                        [20, 9, 25, 48, 17, 30],
                        [20, 9, 25, 48, 17, 30],
                        [34, 25, 18, 30, 67, 15],
                        [34, 25, 18, 30, 67, 15],
                        [10, 19, 51, 15, 47, 7],
                        [10, 19, 51, 15, 47, 7],
                        [15, 35, 49, 23, 54, 45],
                        [15, 35, 49, 23, 54, 45]]
        self.set_fertility_distribution(PopulationGroups, ResultScaleFactor, ResultValues)
        age_values = [14, 14, 15, 15, 16, 16, 17, 17, 18, 18]
        age_distribution = [0, 0.2, 0.2, 0.4, 0.4, 0.6, 0.6, 0.8, 0.8, 1.0]
        self.set_age_distribution(ResultValues=age_values, DistributionValues=age_distribution,
                                  age_distribution_type=ConfigParameters.DISTRIBUTION_COMPLEX)

        base_year = 0
        self.set_base_year( base_year )

        graveyard = self.run_test(duration=duration, birth_by_age_year_flag=True, debug=debug)

        path = "./"
        test_name = "individual_pregnancies_by_age_year_3"
        self.save_files(graveyard, path, test_name, debug)
        error_msg = []
        succeed = True

        print ("accumulate data by age and year")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)

        if debug:
            category = os.path.join(path, test_name, "new_births")
            self.plot_new_births(baby_count, duration, label1="new births", label2="NA",
                                 xlabel="time step", ylabel="new births per day",
                                 title="new births per day",
                                 category=category)
            with open(os.path.join(path, test_name, "possible_mother_by_age_time.json"), "w") as file:
                file.write(json.dumps(self.possible_mom_by_age_time, indent=4, sort_keys=True))

        expected_new_pregnancies = {}
        actual_new_pregnancies = {}
        for year in range(base_year, int(base_year + math.ceil(float(duration) / Constant.days_per_year))):
            expected_new_pregnancies[int(year)] = {}
            actual_new_pregnancies[int(year)] = {}
        rates = {}
        for t in range(0, duration):
            possible_mother = self.possible_mom_by_age_time[t]
            sim_year = int(t / Constant.days_per_year + base_year)
            sim_year_float = float(t) / Constant.days_per_year + base_year
            possible_mother_by_age = {}
            if sim_year not in rates:
                rates[sim_year] = []
            for age in possible_mother:
                if int(age) in possible_mother_by_age:
                    possible_mother_by_age[int(age)] += possible_mother[age]
                else:
                    possible_mother_by_age[int(age)] = possible_mother[age]
            for age in possible_mother:
                i = 0
                for age_bucket in PopulationGroups[0]:
                    i += 1
                    if age <= age_bucket:
                        break
                if sim_year_float >= PopulationGroups[1][-1]:
                    birthrate = ResultValues[i - 1][-1]
                elif sim_year_float <= PopulationGroups[1][0]:
                    birthrate = ResultValues[i - 1][-1]
                else:
                    j = 0
                    for year_bucket in PopulationGroups[1]:
                        j += 1
                        if sim_year_float <= year_bucket:
                            break
                    year_right = j - 1
                    year_left = j - 2 if j - 2 >= 0 else 0
                    birthrate_right = ResultValues[i - 1][year_right]
                    birthrate_left = ResultValues[i - 1][year_left]
                    birthrate = (sim_year_float - PopulationGroups[1][year_left]) * (birthrate_right - birthrate_left) \
                                / (PopulationGroups[1][year_right] - PopulationGroups[1][year_left]) + birthrate_left
                    # print birthrate_left, birthrate, birthrate_right, sim_year_float, age
                rate_per_day = x_Birth * possible_mother[age] * birthrate * ResultScaleFactor
                rates[sim_year].append(x_Birth * birthrate * ResultScaleFactor)
                num_pregnancy = self.new_pregnancy[t][age] if t in self.new_pregnancy and age in self.new_pregnancy[
                    t] else 0
                if int(age) in expected_new_pregnancies[
                    sim_year]:  # age is float number like 17.01 here, since the birth_by_age_year_flag is true
                    expected_new_pregnancies[sim_year][int(age)] += rate_per_day
                else:
                    expected_new_pregnancies[sim_year][int(age)] = rate_per_day

                if int(age) in actual_new_pregnancies[sim_year]:
                    actual_new_pregnancies[sim_year][int(age)] += num_pregnancy
                else:
                    actual_new_pregnancies[sim_year][int(age)] = num_pregnancy
        if debug:
            with open(os.path.join(path, test_name, "expected_new_pregnancies.json"), "w") as file:
                json.dump(expected_new_pregnancies, file, indent=4, sort_keys=True)
            with open(os.path.join(path, test_name, "actual_new_pregnancies.json"), "w") as file:
                json.dump(actual_new_pregnancies, file, indent=4, sort_keys=True)

        print ("test accumulative new pregnancies for each year")
        for sim_year in expected_new_pregnancies:
            expected_pregnancy_list = list(expected_new_pregnancies[sim_year].values())
            actual_pregnancy_list = list(actual_new_pregnancies[sim_year].values())
            error_tolerance = math.ceil(2 * sft.cal_tolerance_binomial(sum(expected_pregnancy_list),
                                                                       np.mean(rates[sim_year])) *
                                        sum(expected_pregnancy_list))
            result = math.fabs(sum(expected_pregnancy_list) - sum(actual_pregnancy_list)) <= error_tolerance
            msg = "In year {0} actual new pregnancies count is {1}, while expected new pregnancies count is {2}, " \
                  "expected difference is less than {3}, test result is {4}.\n".format(sim_year,
                                                                                       sum(actual_pregnancy_list),
                                                                                       sum(expected_pregnancy_list),
                                                                                       error_tolerance, result)
            error_msg.append(msg)
            logging.info(msg)
            if not result:
                succeed = False
            sft.plot_data(dist1=actual_pregnancy_list,
                              dist2=expected_pregnancy_list,
                              label1="actual_new_pregnancies",
                              label2="expected_new_pregnancies",
                              xlabel="age bucket", ylabel="new pregnancies count",
                              title="Year {0}\nnew pregnancies count per year".format(sim_year),
                              category=os.path.join(path, test_name, "new_pregnancies_count_Year{0}".format(
                                  sim_year)), overlap=True,
                              line=True, show=self.show)

        print ("test male/female ratio")
        baby_count, boy_count, girl_count = self.get_baby_count(duration)
        result, msg = self.male_female_ratio_test(boy_count, girl_count)
        if not result:
            succeed = False
            error_msg.append(msg)

        print ("test birth is 280 days after pregnancy")
        result, msg = self.pregnancy_duration_test(duration, birth_by_age_year_flag=True)
        if not result:
            succeed = False
            for s in msg:
                error_msg.append(s)

        print ("test no death")
        result, msg = self.no_death_test(graveyard)
        if not result:
            error_msg.append(msg)
            succeed = False

        with open(os.path.join(path, test_name, "result.txt"), "a") as outfile:
            for line in error_msg:
                outfile.write(line)
        self.assertTrue(succeed, error_msg)
        logging.info("test individual pregnancies by age and year birth rate 3 passes")
        print("test individual pregnancies by age and year birth rate 3 passes")

if __name__ == "__main__":
    unittest.main()
