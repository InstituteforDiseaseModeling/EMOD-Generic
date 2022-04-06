#!/usr/bin/python3.6
import sys
#sys.path.append( "/var/www/wsgi/" ) # this no scale
#sys.path.append( "../../individuals/build/lib.win-amd64-2.7" )
#sys.path.append('../build/lib.win-amd64-2.7')
#sys.path.append('../../../Python/')
import os
import logging
import json
import random
import math
import datetime
import unittest
import numpy as np
from scipy import stats
import shutil
# import glob
# import csv
import pandas as pd
import dtk_test.dtk_sft as sft
import dtk_nodedemog as nd
import dtk_generic_intrahost as gi
import pdb

"""
In this test, we test the following features with DTK PyMod code.

Incubation:
    Incubation_Period_Distribution:
        Constant_Duration
        Uniform_Duration
        Gaussian_Duration
        Exponential_Duration
        Poisson_Duration
        lower priority:
            "NOT_INITIALIZED"
            "LOG_NORMAL_DISTRIBUTION"
            "BIMODAL_DISTRIBUTION"
            "PIECEWISE_CONSTANT"
            "PIECEWISE_LINEAR"
            "WEIBULL_DISTRIBUTION"
            "DUAL_TIMESCALE_DISTRIBUTION"

Infectiousness:
    Base_infectivity_Distribution:
        Constant_Distribution
    Shedding and Decay on contagion pool
    Infectious_Period_Distribution:
        Constant_Duration
        Uniform_Duration
        Gaussian_Duration
        Exponential_Duration
        Poisson_Duration
        lower priority:
            "NOT_INITIALIZED"
            "LOG_NORMAL_DISTRIBUTION"
            "BIMODAL_DISTRIBUTION"
            "PIECEWISE_CONSTANT"
            "PIECEWISE_LINEAR"
            "WEIBULL_DISTRIBUTION"
            "DUAL_TIMESCALE_DISTRIBUTION"
    Infectivity_Scale_Type:????
        CONSTANT_INFECTIVITY
        FUNCTION_OF_TIME_AND_LATITUDE # need more information
        FUNCTION_OF_CLIMATE # need more information
        SINUSOIDAL_FUNCTION_OF_TIME # this is covered in Generic SFT
        ANNUAL_BOXCAR_FUNCTION # this is covered in Generic SFT
        EXPONENTIAL_FUNCTION_OF_TIME # this is covered in Generic SFT

Immunity:
    Post_Infection_Acquisition_Multiplier:
        Natural Immunity only?
    Post_Infection_Transmission_Multiplier
        Natural Immunity only?
    Post_Infection_Mortality_Multiplier
        Natural Immunity only?
    Immunity_Decay:
        Decay Rate and Duration before decay
        Decay for all three immunity factors:
            Mod_acquire
            Mod_Transmit
            Mod_mortality
    Susceptibility_Initialization_Distribution_Type:
            DISTRIBUTION_OFF
            DISTRIBUTION_SIMPLE
            DISTRIBUTION_COMPLEX

Mortality:
    Enable_Disease_Mortality:
        Off
        On, Mortality_Time_Course set to
            Daily_Mortality
            Mortality_After_Infectious

"""


class DemographicsParameters():
    Nodes = "Nodes"
    NodeAttributes = "NodeAttributes"
    InitialPopulation = "InitialPopulation"
    IndividualAttributes = "IndividualAttributes"
    AgeDistribution= "AgeDistribution"
    DistributionValues = "DistributionValues"
    ImmunityDistributionFlag = "ImmunityDistributionFlag"
    ImmunityDistribution1 = "ImmunityDistribution1"
    ImmunityDistribution2 = "ImmunityDistribution2"


class ConfigParameters():
    SIMULATION_TIMESTEP = "SIMULATION_TIMESTEP"

    # incubation
    Incubation_Period_Distribution = "Incubation_Period_Distribution"
    NOT_INITIALIZED = "NOT_INITIALIZED"

    CONSTANT_DISTRIBUTION = "CONSTANT_DISTRIBUTION"
    Incubation_Period_Constant = "Incubation_Period_Constant"

    UNIFORM_DISTRIBUTION = "UNIFORM_DISTRIBUTION"
    Incubation_Period_Max = "Incubation_Period_Max"
    Incubation_Period_Min = "Incubation_Period_Min"

    GAUSSIAN_DISTRIBUTION = "GAUSSIAN_DISTRIBUTION"
    Incubation_Period_Gaussian_Mean = "Incubation_Period_Gaussian_Mean"
    Incubation_Period_Gaussian_Std_Dev = "Incubation_Period_Gaussian_Std_Dev"

    EXPONENTIAL_DISTRIBUTION = "EXPONENTIAL_DISTRIBUTION"
    Incubation_Period_Exponential = "Incubation_Period_Exponential"

    POISSON_DISTRIBUTION = "POISSON_DISTRIBUTION"
    Incubation_Period_Poisson_Mean = "Incubation_Period_Poisson_Mean"

    LOG_NORMAL_DISTRIBUTION = "LOG_NORMAL_DISTRIBUTION"
    Incubation_Period_Log_Normal_Mu = "Incubation_Period_Log_Normal_Mu"
    Incubation_Period_Log_Normal_Sigma = "Incubation_Period_Log_Normal_Sigma"

    BIMODAL_DISTRIBUTION = "BIMODAL_DISTRIBUTION"
    PIECEWISE_CONSTANT = "PIECEWISE_CONSTANT"
    PIECEWISE_LINEAR = "PIECEWISE_LINEAR"
    WEIBULL_DISTRIBUTION = "WEIBULL_DISTRIBUTION"
    DUAL_TIMESCALE_DISTRIBUTION = "DUAL_TIMESCALE_DISTRIBUTION"

    # age dependence
    Age_Initialization_Distribution_Type = "Age_Initialization_Distribution_Type"
    DISTRIBUTION_SIMPLE = "DISTRIBUTION_SIMPLE"
    DISTRIBUTION_COMPLEX = "DISTRIBUTION_COMPLEX"
    DISTRIBUTION_OFF = "DISTRIBUTION_OFF"

    # Mortality
    Enable_Vital_Dynamics = "Enable_Vital_Dynamics"
    Enable_Disease_Mortality = "Enable_Disease_Mortality"
    Mortality_Time_Course = "Mortality_Time_Course"
    Enable_Natural_Mortality = "Enable_Natural_Mortality"
    Base_Mortality = "Base_Mortality"

    # Infectivity
    Base_Infectivity_Distribution = "Base_Infectivity_Distribution"
    Base_Infectivity_Constant = "Base_Infectivity_Constant"

    Infectious_Period_Distribution = "Infectious_Period_Distribution"
    Infectious_Period_Constant = "Infectious_Period_Constant"
    Infectious_Period_Min = "Infectious_Period_Min"
    Infectious_Period_Max = "Infectious_Period_Max"
    Infectious_Period_Gaussian_Mean = "Infectious_Period_Gaussian_Mean"
    Infectious_Period_Gaussian_Std_Dev = "Infectious_Period_Gaussian_Std_Dev"
    Infectious_Period_Exponential = "Infectious_Period_Exponential"
    Infectious_Period_Log_Normal_Mu  = "Infectious_Period_Log_Normal_Mu"
    Infectious_Period_Log_NormalSigma = "Infectious_Period_Log_Normal_Sigma"
    Infectious_Period_Poisson_Mean = "Infectious_Period_Poisson_Mean"

    # these are covered by Generic SFTs already
    Infectivity_Scale_Type = "Infectivity_Scale_Type"
    CONSTANT_INFECTIVITY = "CONSTANT_INFECTIVITY"

    SINUSOIDAL_FUNCTION_OF_TIME = "SINUSOIDAL_FUNCTION_OF_TIME"
    Infectivity_Sinusoidal_Forcing_Amplitude = "Infectivity_Sinusoidal_Forcing_Amplitude"
    Infectivity_Sinusoidal_Forcing_Phase = "Infectivity_Sinusoidal_Forcing_Phase"

    ANNUAL_BOXCAR_FUNCTION = "ANNUAL_BOXCAR_FUNCTION"
    Infectivity_Boxcar_Forcing_Amplitude = "Infectivity_Boxcar_Forcing_Amplitude"
    Infectivity_Boxcar_Forcing_End_Time = "Infectivity_Boxcar_Forcing_End_Time"
    Infectivity_Boxcar_Forcing_Start_Time = "Infectivity_Boxcar_Forcing_Start_Time"

    EXPONENTIAL_FUNCTION_OF_TIME = "EXPONENTIAL_FUNCTION_OF_TIME"
    Infectivity_Exponential_Baseline = "Infectivity_Exponential_Baseline"
    Infectivity_Exponential_Delay = "Infectivity_Exponential_Delay"
    Infectivity_Exponential_Rate = "Infectivity_Exponential_Rate"

    # Need more information for these two scale type:
    FUNCTION_OF_TIME_AND_LATITUDE = "FUNCTION_OF_TIME_AND_LATITUDE"
    FUNCTION_OF_CLIMATE = "FUNCTION_OF_CLIMATE"

    # Immunity
    Enable_Immunity = "Enable_Immunity"
    Enable_Immune_Decay = "Enable_Immune_Decay"
    Immunity_Acquisition_Factor = "Post_Infection_Acquisition_Multiplier"
    Immunity_Mortality_Factor = "Post_Infection_Mortality_Multiplier"
    Immunity_Transmission_Factor = "Post_Infection_Transmission_Multiplier"
    Enable_Initial_Susceptibility_Distribution = "Enable_Initial_Susceptibility_Distribution"
    Susceptibility_Initialization_Distribution_Type = "Susceptibility_Initialization_Distribution_Type"
    DISTRIBUTION_OFF = "DISTRIBUTION_OFF"
    DISTRIBUTION_SIMPLE = "DISTRIBUTION_SIMPLE"
    DISTRIBUTION_COMPLEX = "DISTRIBUTION_COMPLEX"
    Acquisition_Blocking_Immunity_Decay_Rate = "Acquisition_Blocking_Immunity_Decay_Rate"
    Acquisition_Blocking_Immunity_Duration_Before_Decay = "Acquisition_Blocking_Immunity_Duration_Before_Decay"
    Transmission_Blocking_Immunity_Decay_Rate = "Transmission_Blocking_Immunity_Decay_Rate"
    Transmission_Blocking_Immunity_Duration_Before_Decay = "Transmission_Blocking_Immunity_Duration_Before_Decay"
    Mortality_Blocking_Immunity_Decay_Rate = "Mortality_Blocking_Immunity_Decay_Rate"
    Mortality_Blocking_Immunity_Duration_Before_Decay = "Mortality_Blocking_Immunity_Duration_Before_Decay"


class Constant():
    days_per_year = 365
    hum_id = "individual_id"
    is_infected = "is_infected"
    infections = "infections"
    infectiousness = "infectiousness"
    num_infected = "num_infected"
    incubation_timer = "incubation_timer"
    infectious_timer = "infectious_timer"
    susceptibility = "susceptibility"
    mod_acquire = "mod_acquire"
    mod_transmit = "mod_transmit"
    mod_mortality = "mod_mortality"
    daily_mortality = "DAILY_MORTALITY"
    mortality_after_infectious = "MORTALITY_AFTER_INFECTIOUS"

class IntrahostTest(unittest.TestCase):
    human_pop = {}
    my_persisten_vars = {}
    timestep = 0
    debug = False
    statistical_population = []
    well_mixed_contagion_pool = []
    infect_num = 0
    outbreak_timestep = []

    # region class method
    def setUp(self):
        # reload(nd)
        # reload(gi)
        self.human_pop = {}
        self.my_persisten_vars = {}
        self.timestep = 0
        self.statistical_population = []
        self.well_mixed_contagion_pool = []
        self.infect_num = 0
        self.outbreak_timestep = []
        now = datetime.datetime.now()
        now_string = now.strftime("%H-%M-%S")
        logging.basicConfig(filename="{0}.log".format(now_string),
                            format='%(asctime)s:%(levelname)s:%(message)s',
                            level=logging.INFO)
        pass

    def tearDown(self):
        pass
    # endregion

    def create_person_callback( self, mcw, age, gender ):
        person = {}
        person["mcw"]=mcw
        person["age"]=age
        person["sex"]=gender

        new_id = gi.create( (gender, age, mcw) )
        person["id"]=new_id
        if new_id in self.human_pop:
            raise Exception(" individual {0} is already created.".format(new_id))
        else:
            self.human_pop[new_id] = person
        # logging.info( "Human population now = {0}.".format( len( self.human_pop ) ) )

        # if age == 0:
        #     logging.info( "Made a baby with id = {0} on timestep {1}.".format( new_id, self.timestep ) )
        #     if self.timestep not in self.nursery:
        #         self.nursery[self.timestep] = ( 0, 0 )
        #     boys = self.nursery[self.timestep][0]
        #     girls = self.nursery[self.timestep][1]
        #     if gender == 0:
        #         boys += mcw
        #     else:
        #         girls += mcw
        #     self.nursery[self.timestep] = ( boys, girls )

    def conceive_baby_callback(self,individual_id, duration):
        logging.info("{0} just got pregnant".format(individual_id))
        # if individual_id not in human_pop:
        # print( "Yikes! {0} is supposed to get pregnant, but she's not in our population!".format( individual_id ) )
        # else:
        # print( str( gi.is_dead( individual_id ) ) );
        # if self.timestep in self.new_pregnancy:
        #     self.new_pregnancy[self.timestep] += 1
        # else:
        #     self.new_pregnancy[self.timestep] = 1
        gi.initiate_pregnancy(individual_id)

    def update_pregnancy_callback(self,individual_id, dt):
        # if individual_id not in human_pop:
        # print( "Yikes! {0} not in our population!".format( individual_id ) )
        # else:
        # print( "{0} updating pregnancy (dt = {1}).".format( individual_id, dt ) )
        return gi.update_pregnancy(individual_id, int(dt))

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
        if individual_id <= self.infect_num and self.timestep in self.outbreak_timestep:
            return 1
        return 0
    
    def deposit(self, contagion, individual):
        self.well_mixed_contagion_pool[-1] += contagion
        logging.info("Depositing {0} contagion creates total of {1}.".format(contagion, self.well_mixed_contagion_pool[-1]))
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

    def enable_birth(self, enable_birth = True, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        if enable_birth:
            print("Enable birth")
            config[ConfigParameters.Enable_Birth] = 1
            config[ConfigParameters.Enable_Vital_Dynamics] = 1
        else:
            print("Disable birth")
            config[ConfigParameters.Enable_Birth] = 0
        self.set_config_file(config)
        self.set_gi_file(config)

    def enable_death(self, enable_disease_death=True, enable_natural_death=False, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        if enable_natural_death:
            print("Enable natural death")
            config[ConfigParameters.Enable_Natural_Mortality] = 1
            config[ConfigParameters.Enable_Vital_Dynamics] = 1
        else:
            print("Disable natural death")
            config[ConfigParameters.Enable_Natural_Mortality] = 0
        if enable_disease_death:
            print("Enable disease death")
            config[ConfigParameters.Enable_Disease_Mortality] = 1
            config[ConfigParameters.Enable_Vital_Dynamics] = 1
        else:
            print("Disable disease death")
            config[ConfigParameters.Enable_Disease_Mortality] = 0
        self.set_config_file(config)
        self.set_gi_file(config)

    def enable_immunity(self, enable_immunity=True, json_filename="nd.json"):
        config = self.get_json_template(json_filename=json_filename)
        if enable_immunity:
            print("Enable immunity")
            config[ConfigParameters.Enable_Immunity] = 1
        else:
            print("Disable immunity")
            config[ConfigParameters.Enable_Immunity] = 0
        self.set_config_file(config)
        self.set_gi_file(config)

    def configure_incubation_infectious_distribution(self,
                                                     incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                     infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                     base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                     other_config_params={}, initial_population=10000,
                                                     demo_template_filename="demographics_template.json",
                                                     nd_template_filename="nd_template.json"):
        print("configure demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_template_filename)
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes][DemographicsParameters.InitialPopulation] = initial_population
        self.set_demographics_file(demographics)

        print("configure nd.json and gi.json.\n")
        config = self.get_json_template(json_filename=nd_template_filename)
        config[ConfigParameters.Incubation_Period_Distribution] = incubation_distribution_type
        config[ConfigParameters.Infectious_Period_Distribution] = infectious_distribution_type
        config[ConfigParameters.Base_Infectivity_Distribution]  = base_infectivity_distribution_type

        for param in other_config_params:
            config[param] = other_config_params[param]
        self.set_config_file(config)
        self.set_gi_file(config)

    def configure_immunity_distribution(self, flag, param1, param2=None, demo_template_filename="demographics_template.json"):
        print("configure demographics.json.\n")
        demographics = self.get_json_template(json_filename=demo_template_filename)
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes]
        [DemographicsParameters.IndividualAttributes][DemographicsParameters.ImmunityDistributionFlag] = flag
        demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes]
        [DemographicsParameters.IndividualAttributes][DemographicsParameters.ImmunityDistribution1] = param1
        if param2:
            demographics[DemographicsParameters.Nodes][0][DemographicsParameters.NodeAttributes]
            [DemographicsParameters.IndividualAttributes][DemographicsParameters.ImmunityDistribution2] = param2
        self.set_demographics_file(demographics)

    def run_test(self, duration=365, serial_properties=[], properties_name=[], serial_timestep=[], debug=False):
        """
        This is the method to run the tested pymod code and generate output for testing. It is called in every test.
        If a simulation is not run due to Pymod error, this method will fail with assertion.
        :param duration: simulation duration
        :param serial_properties: extra properties to be serialized. It's a list of lists.
        :param properties_name: at list of names for the extra properties.
        :param serial_timestep: timestep to serialize population, if not given, serialization will happen during all time steps.
        :param debug: debug flag
        :return: graveyard: dict of graveyard per timestep
        :return: output_df: data frame contains other data to be tested.
        """
        gi.reset()
        logging.info( "We cleared out human_pop. Should get populated via populate_from_files and callback..." )
        # set creation callback
        nd.set_callback( self.create_person_callback )
        nd.populate_from_files()

        # set vital dynamics callbacks
        gi.set_mortality_callback( self.mortality_callback )
        gi.my_set_callback( self.expose )
        gi.set_deposit_callback(self.deposit)
        nd.set_conceive_baby_callback( self.conceive_baby_callback )
        nd.set_update_preg_callback( self.update_pregnancy_callback )

        columns = [ConfigParameters.SIMULATION_TIMESTEP, Constant.hum_id, Constant.is_infected, Constant.infectiousness, Constant.num_infected]
        columns += properties_name
        index = 0

        if not serial_timestep:
            serial_timestep = range(duration)

        logging.info( "Update shedding, exposure and vital dynamic of population of size {0} for {1} "
                      "year.".format(len(self.human_pop), duration/365))
        graveyard = {}
        time_people_dictionary = {}
        for t in range(duration):
            self.timestep = t
            # logging.info("Updating individuals at timestep {0}.".format(t))
            graveyard_per_timestep = []
            self.statistical_population.append(len(self.human_pop))
            self.well_mixed_contagion_pool.append(0) # 100% decay at the end of every time step

            # this is for shedding and infection only
            logging.info("Updating individuals (shedding) at timestep {0}.".format(t))
            infected           = dict()
            tot_infectiousness = dict()
            for hum_id in self.human_pop:
                nd.update_node_stats(
                    (1.0, 0.0, gi.is_possible_mother(hum_id), 0))  # mcw, infectiousness, is_poss_mom, is_infected
                gi.update1(hum_id)  # this should do shedding

            if self.well_mixed_contagion_pool[-1] > 0:
                self.well_mixed_contagion_pool[-1] /= len(self.human_pop)
            logging.info("well_mixed_contagion_pool = {0}.".format(self.well_mixed_contagion_pool[-1]))

            logging.info("Updating individuals (exposing) at timestep {0}.".format(t))
            num_infected = 0
            for hum_id in list(self.human_pop.keys()): # avoid "RuntimeError: dictionary changed size during iteration"
                human = self.human_pop.get(hum_id)
                gi.update2(hum_id)  # this should do exposure & vital-dynamics
                gender = self.human_pop[hum_id]["sex"]
                if gi.is_infected(hum_id):
                    num_infected += 1
                    infected[hum_id] = 1
                else:
                    infected[hum_id] = 0
                tot_infectiousness[hum_id] = round(gi.get_infectiousness(hum_id),6)

            for hum_id in list(self.human_pop.keys()):  # avoid "RuntimeError: dictionary changed size during iteration"
                # for death
                if gi.is_dead(hum_id):
                    logging.info("Individual {0} died at timestep {1}.".format(hum_id, self.timestep))
                    graveyard_per_timestep.append(human)
                    try:
                        self.human_pop.pop(hum_id)
                    except Exception as ex:
                        logging.info("Exception trying to remove individual {0} from python list: "
                                     "{1}".format(hum_id, ex))
                # serialize all population
                if t in serial_timestep:
                    serial_man = gi.serialize(hum_id)
                    if debug and hum_id == 1:
                        logging.info(json.dumps(json.loads(serial_man), indent=4))
                    # the timestep, human id, is_infected and num of total infections are the headers to be stored
                    # in the output data frame by default.
                    properties = [t, hum_id, infected[hum_id], tot_infectiousness[hum_id], num_infected]
                    # collect other properties that we want to store in the output data frame
                    for property in serial_properties:
                        value = json.loads(serial_man)["individual"][property[0]]
                        if value:
                            for i in range(1, len(property)):
                                if type(value).__name__ == 'list':
                                    value = value[0][property[i]]
                                elif type(value).__name__ == 'dict':
                                    value = value[property[i]]
                        else:
                            value = 0
                        properties.append(str(value))
                    time_people_dictionary[index] = properties
                    index += 1
            logging.info("num_infected = {0}.".format(num_infected))

            if graveyard_per_timestep:
                graveyard[self.timestep] = graveyard_per_timestep

        output_df = pd.DataFrame.from_dict(time_people_dictionary, orient="index")
        output_df.columns = columns
        output_df.index.name = "index"

        self.assertTrue(len(output_df.index) > 0, "BAD: Simulation is not running.")

        if debug:
            with open("df.csv", 'w') as file:
                output_df.to_csv(file)
        return graveyard, output_df

    def save_files(self, graveyard, path, test_name, debug=False):
        """
        This method is called by every test. It moves all intermediate and result files to each test folder.
        :param path: usually it's the current directory
        :param test_name: string of test name
        :param debug: debug flag
        :return: NA
        """
        path_name = os.path.join(path, test_name)
        if os.path.exists(path_name):
            shutil.rmtree(path_name)
            print ("Delete folder: {0}".format(path_name))
        os.makedirs(path_name)
        print ("Create folder: {0}".format(path_name))
        if debug:
            files_to_save = ["graveyard.json", "statistical_population.json"]
            data_to_save = [graveyard, self.statistical_population]
            for i in range(len(files_to_save)):
                with open(os.path.join(path_name, files_to_save[i]), "w") as file:
                    file.write(json.dumps(data_to_save[i], indent=4, sort_keys=True))

        # files_to_copy = glob.glob("*.png")
        # files_to_copy.append(["nd.json", "gi.json","demographics.json"])
        files_to_copy = ["nd.json", "gi.json", "demographics.json"]
        if debug:
            files_to_copy.append("df.csv")
        for file in files_to_copy:
            shutil.move(file, os.path.join(path_name, file))

    def test_incubation_infectious_recovered(self, initial_population=10, Incubation_Period_Constant=3,
                                             Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5, duration=10,
                                             debug=debug):
        logging.info("test incubation, infectious and recovered begins")
        print("test incubation, infectious and recovered begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                            Incubation_Period_Constant,
                                                                        ConfigParameters.Infectious_Period_Constant:
                                                                            Infectious_Period_Constant,
                                                                        ConfigParameters.Base_Infectivity_Constant:
                                                                            Base_Infectivity_Constant},
                                               initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, debug=debug)
        path = "./"
        test_name = "incubation_infectious_recovered"
        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True

        for t in range(duration):
                df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t]
                for index in df.index.tolist():
                    hum_id = df[Constant.hum_id][index]
                    infectiousness = float(df[Constant.infectiousness][index])
                    is_infected = int(df[Constant.is_infected][index])
                    if t <= self.outbreak_timestep[0] + Incubation_Period_Constant or \
                                    t > self.outbreak_timestep[0] + Incubation_Period_Constant + Infectious_Period_Constant:
                        if infectiousness:
                            succeed = False
                            error_msg.append("Bad: at time {0} individual {1} infectiousness = {2}, expected 0 "
                                             "infectiousness.".format(t, hum_id, infectiousness))
                    else:
                        if infectiousness != Base_Infectivity_Constant:
                            succeed = False
                            error_msg.append("Bad: at time {0} individual {1} infectiousness = {2}, expected {3} "
                                             "infectiousness.".format(t, hum_id, infectiousness, Base_Infectivity_Constant))
                    if t < self.outbreak_timestep[0] or \
                        t > self.outbreak_timestep[0] + Incubation_Period_Constant + Infectious_Period_Constant:
                        if is_infected:
                            succeed = False
                            error_msg.append("Bad: at time {0} individual {1} get infected, "
                                             "expected is_infected=False.".format(t, hum_id))
                    else:
                        if not is_infected:
                            succeed = False
                            error_msg.append("Bad: at time {0} individual {1} is not infected, "
                                             "expected is_infected=True.".format(t, hum_id))

        self.assertTrue(succeed, error_msg)
        logging.info("test incubation, infectious and recovered passes")
        print("test incubation, infectious and recovered passes")

    def NYI_test_incubation_off(self, initial_population=100, duration=10, debug=debug):
        logging.info("test incubation off begins")
        print("test incubation off begins")

        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.NOT_INITIALIZED,
                                               initial_population=initial_population)

        graveyard = self.run_test(duration=duration,debug=debug)
        path = "./"
        test_name = "incubation_off"
        self.save_files(graveyard, path, test_name, debug)

    def test_incubation_fixed(self, initial_population=10, Incubation_Period_Constant=3, Infectious_Period_Constant=2,
                              Base_Infectivity_Constant=2.5, duration=10, debug=debug):
        logging.info("test incubation fixed duration begins")
        print("test incubation fixed duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                            Incubation_Period_Constant,
                                                                        ConfigParameters.Infectious_Period_Constant:
                                                                            Infectious_Period_Constant,
                                                                        ConfigParameters.Base_Infectivity_Constant:
                                                                            Base_Infectivity_Constant},
                                               initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.infections, Constant.incubation_timer]],
                                             properties_name=[Constant.incubation_timer],
                                             debug=debug)
        path = "./"
        test_name = "incubation_fixed"
        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True

        df_t1 = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == 1]
        for index in df_t1.index.tolist():
            incubation_timer = float(df_t1[Constant.incubation_timer][index])
            hum_id = int(df_t1[Constant.hum_id][index])
            if incubation_timer != Incubation_Period_Constant:
                succeed = False
                error_msg.append("BAD: hum_id = {0} has incubation timer = {1} expected fixed duration {2}.".format(
                    hum_id, incubation_timer, Incubation_Period_Constant
                ))

        for hum_id in range(1, initial_population + 1):
            df = output_df.loc[output_df[Constant.hum_id] == hum_id]
            incubation_duration = 0
            pre_infection_state = 0
            flag = 0
            for index in df.index.tolist():
                infectiousness = float(df[Constant.infectiousness][index])
                is_infected = int(df[Constant.is_infected][index])
                t = int(df[ConfigParameters.SIMULATION_TIMESTEP][index])
                if not pre_infection_state and is_infected:
                    flag = 1
                if infectiousness > 0:
                    flag = 0
                if t not in self.outbreak_timestep:
                    incubation_duration += flag
                pre_infection_state = is_infected
                # if hum_id == 1:
                #     print incubation_duration
                #     print infectiousness, is_infected, t
            if incubation_duration != Incubation_Period_Constant:
                succeed = False
                error_msg.append("BAD: hum_id = {0} has incubation duration = {1} expected fixed duration {2}.".format(
                    hum_id, incubation_duration, Incubation_Period_Constant
                ))
        logging.info("test incubation fixed duration result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test incubation fixed duration passes")
        print("test incubation fixed duration passes")

    def incubation_test(self, duration, initial_population, path, test_name, debug):
        # This function is used by incubation uniform, gaussian, exponential and poisson tests. Not for fixed and
        # lognormal test
        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.infections, Constant.incubation_timer]],
                                             properties_name=[Constant.incubation_timer],
                                             debug=debug)

        self.save_files(graveyard, path, test_name, debug)
        succeed = True
        error_msg = []
        incubation_timer_dict = {}
        incubation_duration_dict = {}

        df_t1 = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == 1]
        for index in df_t1.index.tolist():
            incubation_timer = float(df_t1[Constant.incubation_timer][index])
            hum_id = int(df_t1[Constant.hum_id][index])
            incubation_timer_dict[hum_id] = incubation_timer

        for hum_id in range(1, initial_population + 1):
            df = output_df.loc[output_df[Constant.hum_id] == hum_id]
            incubation_duration = 0
            pre_infection_state = 0
            flag = 0
            for index in df.index.tolist():
                infectiousness = float(df[Constant.infectiousness][index])
                is_infected = int(df[Constant.is_infected][index])
                t = int(df[ConfigParameters.SIMULATION_TIMESTEP][index])
                if not pre_infection_state and is_infected:
                    flag = 1
                if flag and infectiousness > 0:
                    incubation_duration_dict[hum_id] = incubation_duration
                    incubation_timer = incubation_timer_dict[hum_id]
                    if int(incubation_timer) != incubation_duration:
                    #if incubation_timer - incubation_duration < 0 or incubation_timer - incubation_duration >= 1:
                        succeed = False
                        error_msg.append(
                            "BAD: hum_id = {0} has incubation timer = {1} but its incubation duration is {2} days.".format(
                                hum_id, incubation_timer, incubation_duration
                            ))
                    flag = 0
                    incubation_duration = 0
                if t not in self.outbreak_timestep:
                    incubation_duration += flag
                pre_infection_state = is_infected

        if debug:
            with open(os.path.join(path, test_name, "incubation_duration.json"), "w") as file:
                json.dump(incubation_duration_dict, file)
            with open(os.path.join(path, test_name, "incubation_timer.json"), "w") as file:
                json.dump(incubation_timer_dict, file)
        sft.plot_data(incubation_duration_dict.values(), incubation_timer_dict.values(),
                          label1="incubation duration", label2="incubation timer",
                          title= "test {} incubation duration".format(test_name),
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_{}_duration'.format(test_name)),
                          show=True, line=False, sort=True)

        self.assertTrue(len(incubation_duration_dict) > 0, "BAD: Found no incubation period.")

        logging.info("test for incubation duration and incubation timer result is {}.".format(succeed))
        print("test for incubation duration and incubation timer result is {}.".format(succeed))

        return succeed, error_msg, incubation_timer_dict

    def test_incubation_uniform(self, initial_population=150, incubation_period_max=50, incubation_period_min=0,
                                Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5, duration=60, debug=debug):
        logging.info("test incubation uniform duration begins")
        print("test incubation uniform duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.UNIFORM_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Max:
                                                                        incubation_period_max,
                                                                    ConfigParameters.Incubation_Period_Min:
                                                                        incubation_period_min,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant},
                                               initial_population=initial_population)

        path = "./"
        test_name = "incubation_uniform"

        succeed, error_msg, incubation_timer_dict = self.incubation_test(duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name, debug=debug)

        with open(os.path.join(path,test_name,"test_{}.txt".format(test_name)), "w") as report_file:
            result = sft.test_uniform(list(incubation_timer_dict.values()), incubation_period_min, incubation_period_max,
                                          report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))

        if not result:
            succeed = False
            error_msg.append("Bad: ks test for incubation timers fails.")

        self.assertTrue(succeed, error_msg)
        logging.info("test incubation uniform duration passes")
        print("test incubation uniform duration passes")

    def test_incubation_gaussian(self, initial_population=500, incubation_period_gaussian_std_dev=6, incubation_period_gaussian_mean=10,
                                 Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5, duration=30, enable_immunity=0,
                                 debug=debug):
        logging.info("test incubation gaussian duration begins")
        print("test incubation gaussian duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.GAUSSIAN_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Gaussian_Mean:
                                                                        incubation_period_gaussian_mean,
                                                                    ConfigParameters.Incubation_Period_Gaussian_Std_Dev:
                                                                        incubation_period_gaussian_std_dev,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant,
                                                                    ConfigParameters.Enable_Immunity:
                                                                        enable_immunity},
                                               initial_population=initial_population)

        path = "./"
        test_name = "incubation_gaussian"

        succeed, error_msg, incubation_timer_dict = self.incubation_test(duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name, debug=debug)

        dist_gaussian_scipy = stats.norm.rvs(incubation_period_gaussian_mean, incubation_period_gaussian_std_dev,
                                             len(incubation_timer_dict))
        sft.plot_data(list(incubation_timer_dict.values()), dist_gaussian_scipy, label1="test duration",
                          label2="Scipy Gaussian sample",
                          title="test gaussian incubation duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_gaussian_incubation_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(incubation_timer_dict.values(), dist_gaussian_scipy, precision=0, label1="test duration",
                                 label2="Scipy Gaussian sample",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_gaussian_incubation_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path, test_name, "test_gaussian.txt"), "w") as report_file:
            result = sft.test_eGaussNonNeg(list(incubation_timer_dict.values()), incubation_period_gaussian_mean, incubation_period_gaussian_std_dev,
                                           report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))
        if not result:
            succeed = False
            error_msg.append("Bad: ks test for incubation timers fails.")

        self.assertTrue(succeed, error_msg)
        logging.info("test incubation gaussian duration passes")
        print("test incubation gaussian duration passes")

    def test_incubation_exponential(self, initial_population=500, incubation_period_exponential=5,
                                    Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5, duration=30, enable_immunity=0,
                                    debug=debug):
        logging.info("test incubation exponential duration begins")
        print("test incubation exponential duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.EXPONENTIAL_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Exponential:
                                                                        incubation_period_exponential,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant,
                                                                    ConfigParameters.Enable_Immunity:
                                                                        enable_immunity},
                                               initial_population=initial_population)

        path = "./"
        test_name = "incubation_exponential"

        succeed, error_msg, incubation_timer_dict = self.incubation_test(duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name, debug=debug)

        dist_exponential_np = np.random.exponential(incubation_period_exponential, len(incubation_timer_dict))
        sft.plot_data(list(incubation_timer_dict.values()), dist_exponential_np, label1="test duration",
                          label2="np data",
                          title="test exponential incubation duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_exponential_incubation_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(incubation_timer_dict.values(), dist_exponential_np, precision=0,
                                 label1="test duration", label2="np data",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_exponential_incubation_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path, test_name, "test_exponential.txt"), "w") as report_file:
            result = sft.test_exponential(list(incubation_timer_dict.values()), 1.0/incubation_period_exponential,
                                              report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))
        if not result:
            succeed = False
            error_msg.append("Bad: ks test for incubation timers fails.")

        self.assertTrue(succeed, error_msg)
        logging.info("test incubation exponential duration passes")
        print("test incubation exponential duration passes")

    def test_incubation_poisson(self, initial_population=500, incubation_period_poisson_mean=5,
                                Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5, duration=30, enable_immunity=0,
                                debug=debug):
        logging.info("test incubation poisson duration begins")
        print("test incubation poisson duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.POISSON_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Poisson_Mean:
                                                                        incubation_period_poisson_mean,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant,
                                                                    ConfigParameters.Enable_Immunity:
                                                                        enable_immunity},
                                               initial_population=initial_population)

        path = "./"
        test_name = "incubation_poisson"

        succeed, error_msg, incubation_timer_dict = self.incubation_test(duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name, debug=debug)

        dist_poisson_np = np.random.poisson(incubation_period_poisson_mean, len(incubation_timer_dict))
        sft.plot_data(incubation_timer_dict.values(), dist_poisson_np, label1="test duration", label2="numpy poisson sample",
                          title="test poisson incubation duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_poisson_incubation_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(incubation_timer_dict.values(), dist_poisson_np, precision=0, label1="test duration",
                                 label2="numpy poisson sample",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_poisson_incubation_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path, test_name, "test_poisson.txt"), "w") as report_file:
            result = sft.test_poisson(list(incubation_timer_dict.values()), incubation_period_poisson_mean, report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))
        if not result:
            succeed = False
            error_msg.append("Bad: ks test for incubation timers fails.")

        self.assertTrue(succeed, error_msg)
        logging.info("test incubation poisson duration passes")
        print("test incubation poisson duration passes")

    def test_incubation_lognormal(self, initial_population=1000, incubation_period_log_normal_mean=math.exp(1),
                                  incubation_period_log_normal_width=0.5, Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5,
                                  duration=2, enable_immunity=0, debug=debug):
        logging.info("test incubation log normal duration begins")
        print("test incubation log normal duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.LOG_NORMAL_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Log_Normal_Mu:
                                     incubation_period_log_normal_mean,
                                 ConfigParameters.Incubation_Period_Log_Normal_Sigma:
                                     incubation_period_log_normal_width,
                                 ConfigParameters.Infectious_Period_Constant:
                                     Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant:
                                     Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity:
                                     enable_immunity},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, serial_properties=[[Constant.infections,
                                                                                    Constant.incubation_timer]],
                                             properties_name=[Constant.incubation_timer], serial_timestep=[1],
                                             debug=debug)
        path = "./"
        test_name = "incubation_log_normal"
        self.save_files(graveyard, path, test_name, debug)

        lognormal_duration = []

        for index in output_df.index.tolist():
            incubation_timer = float(output_df[Constant.incubation_timer][index])
            lognormal_duration.append(incubation_timer)

        if debug:
            with open(os.path.join(path, test_name, "lognormal_duration.txt"), "w") as file:
                for data in sorted(lognormal_duration):
                    file.write("{}\n".format(data))
        dist_lognormal = stats.lognorm.rvs(incubation_period_log_normal_width, 0, math.exp(incubation_period_log_normal_mean),
                                           len(lognormal_duration))
        sft.plot_data(lognormal_duration, dist_lognormal, label1="test duration",
                          label2="numpy lognormal sample",
                          title="test lognormal incubation duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_lognormal_incubation_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(lognormal_duration, dist_lognormal, precision=0, label1="test duration",
                                 label2="numpy lognormal sample",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_lognormal_incubation_PDF'),
                                 show=True, line=False)

        self.assertTrue(len(lognormal_duration) > 0, "BAD: Found no incubation period.")

        with open(os.path.join(path, test_name, "test_lognormal.txt"), "w") as report_file:
            succeed = sft.test_lognorm(lognormal_duration, incubation_period_log_normal_mean,
                                           incubation_period_log_normal_width,
                                           category="log_mean={0}, log_width={1}".format(incubation_period_log_normal_mean,
                                                                                         incubation_period_log_normal_width),
                                           report_file=report_file)
        logging.info("test incubation lognormal duration result is {}.".format(succeed))
        print("test incubation lognormal duration result is {}.".format(succeed))

        self.assertTrue(succeed, "test incubation lognormal duration failed")
        logging.info("test incubation lognormal duration passes")
        print("test incubation lognormal duration passes")

    def NYI_test_incubation_bimodal(self, initial_population=1000, Infectious_Period_Constant=2, Base_Infectivity_Constant=2.5,
                                    duration=2, enable_immunity=0, debug=debug):
        logging.info("test incubation bimodal duration begins")
        print("test incubation bimodal duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.BIMODAL_DISTRIBUTION,
                                                          infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          other_config_params={ConfigParameters.Infectious_Period_Constant:
                                                                                   Infectious_Period_Constant,
                                                                               ConfigParameters.Base_Infectivity_Constant:
                                                                                   Base_Infectivity_Constant,
                                                                               ConfigParameters.Enable_Immunity:
                                                                                   enable_immunity},
                                                          initial_population=initial_population)
        try:
            graveyard, output_df = self.run_test(duration=duration, serial_properties=[[Constant.infections,
                                                                                        Constant.incubation_timer]],
                                                 properties_name=[Constant.incubation_timer], serial_timestep=[1],
                                                 debug=debug)
        except ex:
            print("expected NYI error: {}".format(ex))

    def NYI_test_incubation_piecewise_constant(self, initial_population=1000, Infectious_Period_Constant=2,
                                               Base_Infectivity_Constant=2.5, duration=2, enable_immunity=0, debug=debug):
        logging.info("test incubation piecewise constant duration begins")
        print("test incubation piecewise constant duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.PIECEWISE_CONSTANT,
                                                          infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          other_config_params={ConfigParameters.Infectious_Period_Constant:
                                                                                   Infectious_Period_Constant,
                                                                               ConfigParameters.Base_Infectivity_Constant:
                                                                                   Base_Infectivity_Constant,
                                                                               ConfigParameters.Enable_Immunity:
                                                                                   enable_immunity},
                                                          initial_population=initial_population)
        try:
            graveyard, output_df = self.run_test(duration=duration, serial_properties=[[Constant.infections,
                                                                                        Constant.incubation_timer]],
                                                 properties_name=[Constant.incubation_timer], serial_timestep=[1],
                                                 debug=debug)
        except ex:
            print("expected NYI error: {}".format(ex))

    def test_infectious_fixed(self, initial_population=10, Incubation_Period_Constant=3, Infectious_Period_Constant=5,
                              Base_Infectivity_Constant=2.5, duration=10, debug=debug):
        logging.info("test infectious fixed duration begins")
        print("test infectious fixed duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant},
                                               initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.infections, Constant.infectious_timer]],
                                             properties_name=[Constant.infectious_timer],
                                             debug=debug)
        path = "./"
        test_name = "infectious_fixed"
        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True

        df_t1 = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == 1]
        for index in df_t1.index.tolist():
            infectious_timer = float(df_t1[Constant.infectious_timer][index])
            hum_id = int(df_t1[Constant.hum_id][index])
            if infectious_timer != Infectious_Period_Constant:
                succeed = False
                error_msg.append("BAD: hum_id = {0} has infectious timer = {1} expected fixed duration {2}.".format(
                    hum_id, infectious_timer, Infectious_Period_Constant
                ))

        for hum_id in range(1, initial_population + 1):
            df = output_df.loc[output_df[Constant.hum_id] == hum_id]
            infectious_duration = 0
            for index in df.index.tolist():
                infectiousness = float(df[Constant.infectiousness][index])
                is_infected = int(df[Constant.is_infected][index])
                t = int(df[ConfigParameters.SIMULATION_TIMESTEP][index])
                if infectiousness > 0 and is_infected:
                    infectious_duration += 1
                    if infectiousness != Base_Infectivity_Constant:
                        succeed = False
                        error_msg.append("BAD: at time step {0} hum_id = {1} has infectiousness = {2} expected "
                                         "infectiousness = {3}.".format(t, hum_id, infectiousness, Base_Infectivity_Constant))
            if infectious_duration != Infectious_Period_Constant:
                succeed = False
                error_msg.append("BAD: hum_id = {0} has infectious duration = {1} expected fixed duration {2}.".format(
                    hum_id, infectious_duration, Infectious_Period_Constant
                ))
        logging.info("test infectious fixed duration result is {}.".format(succeed))
        print("test infectious fixed duration result is {}.".format(succeed))

        self.assertTrue(succeed, error_msg)
        logging.info("test infectious fixed duration passes")
        print("test infectious fixed duration passes")

    def infectious_test(self, Base_Infectivity_Constant, duration, initial_population, path, test_name, debug):
        # This function is used by infectious uniform, gaussian, exponential and poisson tests. Not for fixed test.
        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.infections, Constant.infectious_timer]],
                                             properties_name=[Constant.infectious_timer],
                                             debug=debug)

        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True
        infectious_timer_dict = {}
        infectious_duration_dict = {}

        df_t1 = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == 1]
        for index in df_t1.index.tolist():
            infectious_timer = float(df_t1[Constant.infectious_timer][index])
            hum_id = int(df_t1[Constant.hum_id][index])
            infectious_timer_dict[hum_id] = infectious_timer

        for hum_id in range(1, initial_population + 1):
            df = output_df.loc[output_df[Constant.hum_id] == hum_id]
            infectious_duration = 0
            for index in df.index.tolist():
                infectiousness = float(df[Constant.infectiousness][index])
                is_infected = int(df[Constant.is_infected][index])
                t = int(df[ConfigParameters.SIMULATION_TIMESTEP][index])
                if infectiousness > 0 and is_infected:
                    infectious_duration += 1
                    if infectiousness != Base_Infectivity_Constant:
                        succeed = False
                        error_msg.append("BAD: at time step {0} hum_id = {1} has infectiousness = {2} expected "
                                         "infectiousness = {3}.".format(t, hum_id, infectiousness, Base_Infectivity_Constant))
            infectious_duration_dict[hum_id] = infectious_duration
            infectious_timer = infectious_timer_dict[hum_id]
            if int(infectious_timer) != infectious_duration:
                succeed = False
                error_msg.append(
                    "BAD: hum_id = {0} has infectious timer = {1} but its infectious duration is {2} days.".format(
                        hum_id, infectious_timer, infectious_duration))

        if debug:
            with open(os.path.join(path, test_name, "infectious_duration.json"), "w") as file:
                json.dump(infectious_duration_dict, file)
            with open(os.path.join(path, test_name, "infectious_timer.json"), "w") as file:
                json.dump(infectious_timer_dict, file)
        sft.plot_data(infectious_duration_dict.values(), infectious_timer_dict.values(),
                          label1="infectious duration", label2="infectious timer",
                          title="test {} infectious duration".format(test_name),
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_{}_duration'.format(test_name)),
                          show=True, line=False, sort=True)

        self.assertTrue(len(infectious_duration_dict) > 0, "BAD: Found no infectious period.")

        logging.info("test for infectious duration and infectious timer result is {}.".format(succeed))
        print("test for infectious duration and infectious timer result is {}.".format(succeed))
        return succeed, error_msg, infectious_timer_dict

    def test_infectious_uniform(self, initial_population=200, Incubation_Period_Constant=1, infectious_period_min=5,
                                infectious_period_max=40, Base_Infectivity_Constant=3.5, duration=45, debug=debug):
        logging.info("test infectious uniform duration begins")
        print("test infectious uniform duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          infectious_distribution_type=ConfigParameters.UNIFORM_DISTRIBUTION,
                                                          base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                                   Incubation_Period_Constant,
                                                                               ConfigParameters.Infectious_Period_Max:
                                                                                   infectious_period_max,
                                                                               ConfigParameters.Infectious_Period_Min:
                                                                                   infectious_period_min,
                                                                               ConfigParameters.Base_Infectivity_Constant:
                                                                                   Base_Infectivity_Constant},
                                                          initial_population=initial_population)
        path = "./"
        test_name = "infectious_uniform"

        succeed, error_msg, infectious_timer_dict = self.infectious_test(Base_Infectivity_Constant=Base_Infectivity_Constant,
                                                                         duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name,
                                                                         debug=debug)

        with open(os.path.join(path,test_name,"test_{}.txt".format(test_name)), "w") as report_file:
            result = sft.test_uniform(list(infectious_timer_dict.values()), infectious_period_min, infectious_period_max,
                                          report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))
        if not result:
            succeed = False
            error_msg.append("BAD: kstest for infectious timers fails.")

        logging.info("test infectious uniform duration result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test infectious uniform duration passes")
        print("test infectious uniform duration passes")

    def test_infectious_gaussian(self, initial_population=500, Incubation_Period_Constant=3, infectious_period_mean=5,
                                infectious_period_std_dev=10, Base_Infectivity_Constant=4.5, duration=45, debug=debug):
        logging.info("test infectious gaussian duration begins")
        print("test infectious gaussian duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.GAUSSIAN_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Gaussian_Mean:
                                                                        infectious_period_mean,
                                                                    ConfigParameters.Infectious_Period_Gaussian_Std_Dev:
                                                                        infectious_period_std_dev,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant},
                                               initial_population=initial_population)

        path = "./"
        test_name = "infectious_gaussian"
        succeed, error_msg, infectious_timer_dict = self.infectious_test(Base_Infectivity_Constant=Base_Infectivity_Constant,
                                                                         duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name,
                                                                         debug=debug)

        dist_gaussian_scipy = stats.norm.rvs(infectious_period_mean, infectious_period_std_dev,
                                             len(infectious_timer_dict))
        sft.plot_data(infectious_timer_dict.values(), dist_gaussian_scipy, label1="test duration",
                          label2="Scipy Gaussian sample",
                          title="test gaussian infectious duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_gaussian_infectious_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(infectious_timer_dict.values(), dist_gaussian_scipy, precision=0,
                                 label1="test duration",
                                 label2="Scipy Gaussian sample",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_gaussian_infectious_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path,test_name,"test_gaussian.txt"), "w") as report_file:
            result = sft.test_eGaussNonNeg(list(infectious_timer_dict.values()), infectious_period_mean,
                                           infectious_period_std_dev, report_file=report_file)

        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))

        if not result:
            succeed = False
            error_msg.append("BAD: kstest for gaussian distribution fails.")

        logging.info("test infectious gaussian duration result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test infectious gaussian duration passes")
        print("test infectious gaussian duration passes")

    def test_infectious_exponential(self, initial_population=500, Incubation_Period_Constant=3, infectious_period_exponential=4.8,
                                    Base_Infectivity_Constant=5.5, duration=40, debug=debug):
        logging.info("test infectious exponential duration begins")
        print("test infectious exponential duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.EXPONENTIAL_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Exponential:
                                                                        infectious_period_exponential,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant},
                                               initial_population=initial_population)

        path = "./"
        test_name = "infectious_exponential"
        succeed, error_msg, infectious_timer_dict = self.infectious_test(Base_Infectivity_Constant=Base_Infectivity_Constant,
                                                                         duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name,
                                                                         debug=debug)

        dist_exponential_np = np.random.exponential(infectious_period_exponential, len(infectious_timer_dict))
        sft.plot_data(infectious_timer_dict.values(), dist_exponential_np, label1="test duration", label2="np data",
                          title="test exponential infectious duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_exponential_infectious_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(infectious_timer_dict.values(), dist_exponential_np, precision=0, label1="test duration",
                                 label2="np data",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_exponential_infectious_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path, test_name, "test_exponential.txt"), "w") as report_file:
            result = sft.test_exponential(list(infectious_timer_dict.values()), 1.0/infectious_period_exponential,
                                              report_file=report_file, integers=False)

        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))

        if not result:
            succeed = False
            error_msg.append("BAD: kstest for exponential distribution fails.")

        logging.info("test infectious exponential duration result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test infectious exponential duration passes")
        print("test infectious exponential duration passes")

    def test_infectious_poisson(self, initial_population=500, Incubation_Period_Constant=3, infectious_period_poisson_mean=5.8,
                                Base_Infectivity_Constant=6.9, duration=30, debug=debug):
        logging.info("test infectious poisson duration begins")
        print("test infectious poisson duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.POISSON_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Poisson_Mean:
                                                                        infectious_period_poisson_mean,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant},
                                               initial_population=initial_population)

        path = "./"
        test_name = "infectious_poisson"
        succeed, error_msg, infectious_timer_dict = self.infectious_test(Base_Infectivity_Constant=Base_Infectivity_Constant,
                                                                         duration=duration,
                                                                         initial_population=initial_population,
                                                                         path=path, test_name=test_name,
                                                                         debug=debug)

        dist_poisson_np = np.random.poisson(infectious_period_poisson_mean, len(infectious_timer_dict))
        sft.plot_data(infectious_timer_dict.values(), dist_poisson_np, label1="test duration", label2="numpy poisson sample",
                          title="test poisson incubation duration",
                          xlabel="data point", ylabel="days",
                          category=os.path.join(path, test_name, 'test_poisson_infectious_duration'),
                          show=True, line=False, sort=True)
        sft.plot_probability(infectious_timer_dict.values(), dist_poisson_np, precision=0, label1="test duration",
                                 label2="numpy poisson sample",
                                 title='probability mass function', xlabel='k', ylabel='probability',
                                 category=os.path.join(path, test_name, 'test_poisson_incubation_PDF'),
                                 show=True, line=False)

        with open(os.path.join(path, test_name, "test_poisson.txt"), "w") as report_file:
            result = sft.test_poisson(list(infectious_timer_dict.values()), infectious_period_poisson_mean, report_file=report_file)
        logging.info("ks test for {0} duration result is {1}.".format(test_name, result))
        print("ks test for {0} duration result is {1}.".format(test_name, result))
        if not result:
            succeed = False
            error_msg.append("BAD: kstest for poisson distribution fails.")
        logging.info("test infectious poisson duration result is {}.".format(succeed))

        self.assertTrue(succeed, error_msg)
        logging.info("test infectious poisson duration passes")
        print("test infectious poisson duration passes")

    def NYI_test_infectious_off(self, initial_population=100, duration=10, debug=debug):
        logging.info("test infectious off begins")
        print("test infectious off begins")

        self.configure_incubation_infectious_distribution(infectious_distribution_type=ConfigParameters.NOT_INITIALIZED,
                                               initial_population=initial_population)

        graveyard = self.run_test(duration=duration,debug=debug)
        path = "./"
        test_name = "incubation_off"
        self.save_files(graveyard, path, test_name, debug)

    def NYI_test_infectious_lognormal(self, initial_population=1000, Incubation_Period_Constant=3,
                                      infectious_period_mean=1, infectious_period_width=0.5,
                                      Base_Infectivity_Constant=1.9, duration=2, enable_immunity=0, debug=debug):
        logging.info("test infectious log normal duration begins")
        print("test infectious log normal duration begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.LOG_NORMAL_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant:
                                     Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Log_Normal_Mu:
                                     infectious_period_mean,
                                 ConfigParameters.Infectious_Period_Log_Normal_Sigma:
                                     infectious_period_width,
                                 ConfigParameters.Base_Infectivity_Constant:
                                     Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity:
                                     enable_immunity},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, serial_properties=[[Constant.infections,
                                                                                    Constant.infectious_timer]],
                                             properties_name=[Constant.infectious_timer], serial_timestep=[1],
                                             debug=debug)
        path = "./"
        test_name = "infectious_log_normal"
        self.save_files(graveyard, path, test_name, debug)

        lognormal_duration = []

        for index in output_df.index.tolist():
            infectious_timer = float(output_df[Constant.infectious_timer][index])
            lognormal_duration.append(infectious_timer)

        if debug:
            with open(os.path.join(path, test_name, "lognormal_duration.txt"), "w") as file:
                for data in sorted(lognormal_duration):
                    file.write("{}\n".format(data))
            dist_lognormal = stats.lognorm.rvs(infectious_period_width, 0, infectious_period_mean,
                                               len(lognormal_duration))
            sft.plot_data(lognormal_duration, dist_lognormal, label1="test duration",
                              label2="numpy lognormal sample",
                              title="test lognormal infectious duration",
                              xlabel="data point", ylabel="days",
                              category=os.path.join(path, test_name, 'test_lognormal_infectious_duration'),
                              show=True, line=False, sort=True)
            sft.plot_probability(lognormal_duration, dist_lognormal, precision=0, label1="test duration",
                                     label2="numpy lognormal sample",
                                     title='probability mass function', xlabel='k', ylabel='probability',
                                     category=os.path.join(path, test_name, 'test_lognormal_infectious_PDF'),
                                     show=True, line=False)

        self.assertTrue(len(lognormal_duration) > 0, "BAD: Found no infectious period.")

        with open(os.path.join(path, test_name, "test_lognormal.txt"), "w") as report_file:
            succeed = sft.test_lognorm(lognormal_duration, math.log(infectious_period_mean),
                                           infectious_period_width,
                                           category="log_mean={0}, log_width={1}".format(infectious_period_mean,
                                                                                         infectious_period_width),
                                           report_file=report_file)
        logging.info("test infectious lognormal duration result is {}.".format(succeed))
        self.assertTrue(succeed, "test infectious lognormal duration failed")
        logging.info("test infectious lognormal duration passes")
        print("test infectious lognormal duration passes")

    def NMI_test_infectious_fixed_scale_time(self, initial_population=1, Incubation_Period_Constant=0,
                                             Infectious_Period_Constant=365, Base_Infectivity_Constant=2.5, duration=365,
                                             debug=debug):
        logging.info("test infectious fixed duration with scale type = {} begins".
                     format(ConfigParameters.FUNCTION_OF_TIME_AND_LATITUDE))
        print("test infectious fixed duration with scale type = {} begins".
                     format(ConfigParameters.FUNCTION_OF_TIME_AND_LATITUDE))
        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant,
                                                                    ConfigParameters.Infectivity_Scale_Type:
                                                                    ConfigParameters.FUNCTION_OF_TIME_AND_LATITUDE},
                                               initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, debug=debug)
        path = "./"
        test_name = "infectious_fixed_scale_time"
        self.save_files(graveyard, path, test_name, debug)

    def NMI_test_infectious_fixed_scale_climate(self, initial_population=1, Incubation_Period_Constant=0,
                                                Infectious_Period_Constant=365, Base_Infectivity_Constant=2.5, duration=365,
                                                debug=debug):
        logging.info("test infectious fixed duration with scale type = {} begins".
                     format(ConfigParameters.FUNCTION_OF_CLIMATE))
        print("test infectious fixed duration with scale type = {} begins".
                     format(ConfigParameters.FUNCTION_OF_CLIMATE))
        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                                          other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                                   Incubation_Period_Constant,
                                                                               ConfigParameters.Infectious_Period_Constant:
                                                                                   Infectious_Period_Constant,
                                                                               ConfigParameters.Base_Infectivity_Constant:
                                                                                   Base_Infectivity_Constant,
                                                                               ConfigParameters.Infectivity_Scale_Type:
                                                                                   ConfigParameters.FUNCTION_OF_CLIMATE},
                                                          initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, debug=debug)
        path = "./"
        test_name = "infectious_fixed_scale_climate"
        self.save_files(graveyard, path, test_name, debug)

        if debug:
            with open(os.path.join(path, test_name, "contagion_pool.txt"), "w") as file:
                for data in self.well_mixed_contagion_pool:
                    file.write("{}\n".format(data))
            sft.plot_data(self.well_mixed_contagion_pool,
                                       label1="actual contagion pool", label2="expected contagion pool",
                                       title="contagion pool, actual vs. expected",
                                       xlabel="time step", ylabel=None, category='contagion_pool', show=True, line=False
                                       )

    def test_shedding(self, initial_population=50, Incubation_Period_Constant=1, infectious_period_exponential=2,
                      Base_Infectivity_Constant=1.5, duration=50, immunity_transmission_factor=0.8, decay_rate=0.1,
                      decay_offset=3, debug=debug):
        logging.info("test shedding begins")
        print("test shedding begins")

        self.outbreak_timestep = [0, 10, 30]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.EXPONENTIAL_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Exponential: infectious_period_exponential,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: 1,
                                 ConfigParameters.Enable_Immune_Decay: 1,
                                 ConfigParameters.Immunity_Transmission_Factor: immunity_transmission_factor,
                                 ConfigParameters.Transmission_Blocking_Immunity_Decay_Rate: decay_rate,
                                 ConfigParameters.Transmission_Blocking_Immunity_Duration_Before_Decay: decay_offset},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration, serial_properties=
                                             [[Constant.susceptibility, Constant.mod_transmit]],
                                             properties_name=[Constant.mod_transmit],
                                             debug=debug)
        path = "./"
        test_name = "shedding"
        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True
        expected_well_mixed_contagion_pool = []
        pre_expected_contagion_pool = 0
        for t in range(duration):
            df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t]
            expected_contagion_pool = 0
            for index in df.index.tolist():
                infectiousness = float(df[Constant.infectiousness][index])
                is_shedding = 1 if infectiousness > 0 else 0
                mod_transmit = float(df[Constant.mod_transmit][index])
                expected_contagion_pool += Base_Infectivity_Constant * is_shedding * mod_transmit
            contagion_pool = self.well_mixed_contagion_pool[t]
            expected_contagion_pool /= self.statistical_population[t]
            expected_well_mixed_contagion_pool.append(pre_expected_contagion_pool)

            if math.fabs(contagion_pool - pre_expected_contagion_pool) > 1e-2 * pre_expected_contagion_pool:
                succeed = False
                error_msg.append("BAD: at timestep {0}, the contagion pool is {1}, expected {2}.".format(
                    t, contagion_pool, pre_expected_contagion_pool
                ))
            pre_expected_contagion_pool = expected_contagion_pool
        if debug:
            with open(os.path.join(path, test_name, "contagion_pool.txt"), "w") as file:
                for data in self.well_mixed_contagion_pool:
                    file.write("{}\n".format(data))
        sft.plot_data(self.well_mixed_contagion_pool, expected_well_mixed_contagion_pool,
                                   label1="actual contagion pool", label2="expected contagion pool",
                                   title="contagion pool, actual vs. expected",
                                   xlabel="time step", ylabel=None,
                                   category=os.path.join(path, test_name, 'contagion_pool'), show=True, line=False
                                   )
        logging.info("test shedding result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test shedding passes")
        print("test shedding passes")


    def draw_random_immunity_factor(self):
        """
        Draw a random immunity factor between[0, 1]
        :return: immunity_factor
        """
        if random.random() > 0.01:
            immunity_factor = random.random()  # [0. 1)
        else:
            immunity_factor = 1
        return immunity_factor

    # modify this method based on #2111, Immunity should be range bound between 0 and 1
    # def draw_random_immunity_factor(self):
    #     """
    #     Draw a random immunity factor:
    #     1/10 will return a random number between(0, 1)
    #     9/10 will return a random number between(1, 1000)
    #     :return: immunity_factor
    #     """
    #     if random.random() > 0.1:
    #         immunity_factor = random.random()
    #     else:
    #         immunity_factor = random.uniform(1, 1000)
    #     return immunity_factor

    def natural_immunity_test(self, output_df, Incubation_Period_Constant, Infectious_Period_Constant,
                              duration, immunity_factor, immu_factor_name, initial_population):
        """
        Test three types of immunity factor after multiple infections.
        :param output_df:
        :param Incubation_Period_Constant:
        :param Infectious_Period_Constant:
        :param duration:
        :param immunity_factor:
        :param immu_factor_name:
        :return:
        """
        error_msg = []
        succeed = True

        self.assertTrue(initial_population == len(self.human_pop), "Bad: initial population is {0}, expected no change "
                                                                   "in population, but we have {1} population at the "
                                                                   "end of the simulation".format(initial_population,
                                                                                                  len(self.human_pop)))
        mod_time = []
        for t in self.outbreak_timestep:
            mod_time.append(t + Incubation_Period_Constant + Infectious_Period_Constant + 1)

        expected_mods = []
        actual_mods = []
        pre_mod = 1.0
        for t in range(duration):
            df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t]
            mod = float(df[immu_factor_name][df.index[0]])
            if t in mod_time:
                multiplier = immunity_factor
            else:
                multiplier = 1.0
            expected_mod = pre_mod * multiplier
            expected_mods.append(expected_mod)
            actual_mods.append(mod)
            if math.fabs(mod - expected_mod) > 0.01*expected_mod:
                succeed = False
                error_msg.append("BAD: at timestep {0}, the {1} is {2}, expected {3}.".format(
                    t, immu_factor_name, mod, expected_mod))
            pre_mod = mod

        return succeed, error_msg, expected_mods, actual_mods

    def test_natural_immunity_mod_acquire(self, initial_population=1, Incubation_Period_Constant=1,
                                          Infectious_Period_Constant=1, Base_Infectivity_Constant=1.5, duration=30,
                                          enable_immunity=1, immunity_acquisition_factor=None, debug=debug):
        logging.info("test natural immunity mod acquire begins")
        print("test natural immunity mod acquire begins")

        if not immunity_acquisition_factor:
            immunity_acquisition_factor = self.draw_random_immunity_factor()

        self.outbreak_timestep = [0, 5, 10, 15, 20, 25]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
                                               other_config_params={ConfigParameters.Incubation_Period_Constant:
                                                                        Incubation_Period_Constant,
                                                                    ConfigParameters.Infectious_Period_Constant:
                                                                        Infectious_Period_Constant,
                                                                    ConfigParameters.Base_Infectivity_Constant:
                                                                        Base_Infectivity_Constant,
                                                                    ConfigParameters.Enable_Immunity:
                                                                    enable_immunity,
                                                                    ConfigParameters.Enable_Immune_Decay: 0,
                                                                    ConfigParameters.Immunity_Acquisition_Factor:
                                                                    immunity_acquisition_factor},
                                               initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_acquire]],
                                             properties_name=[Constant.mod_acquire],
                                             debug=debug)
        path = "./"
        test_name = "natural_immu_mod_acquire"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_acquire, actual_mods_acquire = self.natural_immunity_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_acquisition_factor, immu_factor_name=Constant.mod_acquire,
            initial_population=initial_population
        )

        sft.plot_data(actual_mods_acquire, expected_mods_acquire,
                                   label1="actual mod_acquire", label2="expected mod_acquire",
                                   title="actual vs. expected, immunity_acquisition_factor = {}".format(
                                       immunity_acquisition_factor
                                   ),
                                   xlabel="time step", ylabel="mod_acquire",
                                   category=os.path.join(path, test_name, 'mod_acquire'), show=True,
                                   line=False
                                   )
        logging.info("test natural immunity mod acquire result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test natural immunity mod acquire passes")
        print("test natural immunity mod acquire passes")

    def test_natural_immunity_mod_transmit(self, initial_population=1, Incubation_Period_Constant=1,
                                           Infectious_Period_Constant=1, Base_Infectivity_Constant=1.5, duration=30,
                                           enable_immunity=1, immunity_transmission_factor=None, debug=debug):
        logging.info("test natural immunity mod transmit begins")
        print("test natural immunity mod transmit begins")

        if not immunity_transmission_factor:
            immunity_transmission_factor = self.draw_random_immunity_factor()

        self.outbreak_timestep = [0, 5, 10, 15, 20, 25]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant:
                                     Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant:
                                     Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant:
                                     Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity:
                                     enable_immunity,
                                 ConfigParameters.Enable_Immune_Decay: 0,
                                 ConfigParameters.Immunity_Transmission_Factor:
                                     immunity_transmission_factor},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_transmit]],
                                             properties_name=[Constant.mod_transmit],
                                             debug=debug)
        path = "./"
        test_name = "natural_immu_mod_transmit"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_transmit, actual_mods_transmit = self.natural_immunity_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_transmission_factor, immu_factor_name=Constant.mod_transmit,
            initial_population=initial_population
        )

        sft.plot_data(actual_mods_transmit, expected_mods_transmit,
                                   label1="actual mod_transmit", label2="expected mod_transmit",
                                   title="actual vs. expected, immunity_transmission_factor = {}".format(
                                       immunity_transmission_factor
                                   ),
                                   xlabel="time step", ylabel="mod_transmit",
                                   category=os.path.join(path, test_name, 'mod_transmit'), show=True,
                                   line=False
                                   )
        logging.info("test natural immunity mod transmit result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test natural immunity mod transmit passes")
        print("test natural immunity mod transmit passes")

    def test_natural_immunity_mod_mortality(self, initial_population=1, Incubation_Period_Constant=1,
                                            Infectious_Period_Constant=1, Base_Infectivity_Constant=1.5, duration=30,
                                            enable_immunity=1, immunity_mortality_factor=None, debug=debug):
        logging.info("test natural immunity mod mortality begins")
        print("test natural immunity mod mortality begins")

        if not immunity_mortality_factor:
            immunity_mortality_factor = self.draw_random_immunity_factor()

        self.outbreak_timestep = [0, 5, 10, 15, 20, 25]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant:
                                     Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant:
                                     Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant:
                                     Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity:
                                     enable_immunity,
                                 ConfigParameters.Enable_Immune_Decay: 0,
                                 ConfigParameters.Immunity_Mortality_Factor:
                                     immunity_mortality_factor},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_mortality],
                                             debug=debug)
        path = "./"
        test_name = "natural_immu_mod_mortality"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_mortality, actual_mods_mortality = self.natural_immunity_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_mortality_factor, immu_factor_name=Constant.mod_mortality,
            initial_population=initial_population
        )

        sft.plot_data(actual_mods_mortality, expected_mods_mortality,
                                   label1="actual mod_mortality", label2="expected mod_mortality",
                                   title="actual vs. expected, immunity_mortality_factor = {}".format(
                                       immunity_mortality_factor
                                   ),
                                   xlabel="time step", ylabel="mod_mortality",
                                   category=os.path.join(path, test_name, 'mod_mortality'), show=True,
                                   line=False
                                   )
        logging.info("test natural immunity mod mortality result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test natural immunity mod mortality passes")
        print("test natural immunity mod mortality passes")

    def immu_decay_test(self, output_df, Incubation_Period_Constant, Infectious_Period_Constant,
                        duration, immunity_factor, immu_factor_name, decay_rate, decay_offset):

        error_msg = []
        succeed = True

        mod_time = []
        for t in self.outbreak_timestep:
            mod_time.append(t + Incubation_Period_Constant + Infectious_Period_Constant + 1)

        expected_mods = []
        actual_mods = []
        pre_mod = 1.0
        for t in range(duration):
            df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t]
            mod = float(df[immu_factor_name][df.index[0]])
            if t in mod_time:
                multiplier = immunity_factor
            else:
                multiplier = 1.0
            expected_mod = pre_mod * multiplier
            for i in range(len(mod_time)):
                upper_bound = mod_time[i + 1] if i + 1 < len(mod_time) else duration
                # can't simplify comparison as a<t<b, because a maybe larger than b
                if mod_time[i] + decay_offset <= t and t < upper_bound:
                    expected_mod += (1.0 - pre_mod) * decay_rate
            if expected_mod > 1:
                expected_mod = 1.0
            expected_mods.append(expected_mod)
            actual_mods.append(mod)
            if math.fabs(mod - expected_mod) > 0.01:
                succeed = False
                error_msg.append("BAD: at timestep {0}, the {1} is {2}, expected {3}.".format(
                    t, immu_factor_name, mod, expected_mod))
            pre_mod = expected_mod

        return succeed, error_msg, expected_mods, actual_mods

    def test_immu_decay_mod_acquire(self, initial_population=1, Incubation_Period_Constant=1, Infectious_Period_Constant=1,
                                    Base_Infectivity_Constant=1.5, duration=250, enable_immunity=1,
                                    immunity_acquisition_factor=0.65, decay_rate=0.1, decay_offset=30, debug=debug):
        msg = "test immunity decay mod acquire begins.\n" \
              "immnity factor is {0}, decay rate is {1} and offset is {2}.".format(immunity_acquisition_factor,
                                                                                   decay_rate, decay_offset)
        logging.info(msg)
        print(msg)

        self.outbreak_timestep = [0, 100, 150, 170]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: enable_immunity,
                                 ConfigParameters.Enable_Immune_Decay: 1,
                                 ConfigParameters.Immunity_Acquisition_Factor: immunity_acquisition_factor,
                                 ConfigParameters.Acquisition_Blocking_Immunity_Decay_Rate: decay_rate,
                                 ConfigParameters.Acquisition_Blocking_Immunity_Duration_Before_Decay: decay_offset},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_acquire]],
                                             properties_name=[Constant.mod_acquire], debug=debug)
        path = "./"
        test_name = "immu_decay_mod_acquire"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_acquire, actual_mods_acquire= self.immu_decay_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_acquisition_factor, immu_factor_name=Constant.mod_acquire,
            decay_rate=decay_rate, decay_offset=decay_offset
        )
        sft.plot_data(actual_mods_acquire, expected_mods_acquire,
                                   label1="actual mod_acquire", label2="expected mod_acquire",
                                   title="actual vs. expected, offset={0}, decay_rate={1}, immunity_acquisition_factor="
                                         "{2}".format(decay_offset, decay_rate, immunity_acquisition_factor),
                                   xlabel="time step", ylabel="mod_acquire",
                                   category=os.path.join(path, test_name, 'mod_acquire'), show=True,
                                   line=False
                                   )
        logging.info("test immunity decay mod acquire result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test immunity decay mod acquire passes")
        print("test immunity decay mod acquire passes")

    def test_immu_decay_mod_transmit(self, initial_population=1, Incubation_Period_Constant=1, Infectious_Period_Constant=1,
                                     Base_Infectivity_Constant=1.5, duration=2000, enable_immunity=1,
                                     immunity_transmission_factor=0.55, decay_rate=0.01, decay_offset=10, debug=debug,
                                     test_name="immu_decay_mod_transmit"):
        logging.info("test immunity decay mod transmit begins")
        print("test immunity decay mod transmit begins")
        if not len(self.outbreak_timestep):
            self.outbreak_timestep = [0, 100, 130, 200, 1000, 1130, 1200, 1250, 1500, 1700, 1890]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: enable_immunity,
                                 ConfigParameters.Enable_Immune_Decay: 1,
                                 ConfigParameters.Immunity_Transmission_Factor: immunity_transmission_factor,
                                 ConfigParameters.Transmission_Blocking_Immunity_Decay_Rate: decay_rate,
                                 ConfigParameters.Transmission_Blocking_Immunity_Duration_Before_Decay: decay_offset},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_transmit]],
                                             properties_name=[Constant.mod_transmit],
                                             debug=debug)
        path = "./"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_transmit, actual_mods_transmit = self.immu_decay_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_transmission_factor, immu_factor_name=Constant.mod_transmit,
            decay_rate=decay_rate, decay_offset=decay_offset
        )

        sft.plot_data(actual_mods_transmit, expected_mods_transmit,
                                   label1="actual mod_transmit", label2="expected mod_transmit",
                                   title="actual vs. expected, offset={0}, decay_rate={1}, immunity_transmission_factor="
                                         "{2}".format(decay_offset, decay_rate, immunity_transmission_factor),
                                   xlabel="time step", ylabel="mod_transmit",
                                   category=os.path.join(path, test_name, 'mod_transmit'), show=True,
                                   line=False
                                   )
        logging.info("test immunity decay mod transmit result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test immunity decay mod transmit passes")
        print("test immunity decay mod transmit passes")

    def _test_immu_decay_mod_transmit_2(self, immunity_transmission_factor=1.65, decay_rate=0.05, duration=100):
        # disable this test based on #2111, Immunity should be range bound between 0 and 1
        self.outbreak_timestep = [1]
        self.test_immu_decay_mod_transmit(immunity_transmission_factor=immunity_transmission_factor,
                                          decay_rate=decay_rate, test_name="immu_decay_mod_transmit_2",
                                          duration=duration)

    def test_immu_decay_mod_transmit_3(self, immunity_transmission_factor=0, decay_rate=0.15, duration=100):
        self.outbreak_timestep = [1]
        self.test_immu_decay_mod_transmit(immunity_transmission_factor=immunity_transmission_factor,
                                          decay_rate=decay_rate, test_name="immu_decay_mod_transmit_3",
                                          duration=duration)

    def test_immu_decay_mod_transmit_4(self, immunity_transmission_factor=1, decay_rate=0.15, duration=100):
        self.outbreak_timestep = [1]
        self.test_immu_decay_mod_transmit(immunity_transmission_factor=immunity_transmission_factor,
                                          decay_rate=decay_rate, test_name="immu_decay_mod_transmit_4",
                                          duration=duration)

    def test_immu_decay_mod_mortality(self, initial_population=1, Incubation_Period_Constant=1, Infectious_Period_Constant=1,
                                      Base_Infectivity_Constant=1.5, duration=50, enable_immunity=1,
                                      immunity_mortality_factor=0.4, decay_rate=0.6, decay_offset=10, debug=debug,
                                      test_name="immu_decay_mod_mortality"):
        logging.info("test immunity decay mod mortality begins")
        print("test immunity decay mod mortality begins")

        self.outbreak_timestep = [0]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: enable_immunity,
                                 ConfigParameters.Enable_Immune_Decay: 1,
                                 ConfigParameters.Immunity_Mortality_Factor: immunity_mortality_factor,
                                 ConfigParameters.Mortality_Blocking_Immunity_Decay_Rate: decay_rate,
                                 ConfigParameters.Mortality_Blocking_Immunity_Duration_Before_Decay: decay_offset},
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_mortality], debug=debug)
        path = "./"
        self.save_files(graveyard, path, test_name, debug)

        succeed, error_msg, expected_mods_mortality, actual_mods_mortality = self.immu_decay_test(
            output_df=output_df, Incubation_Period_Constant=Incubation_Period_Constant,
            Infectious_Period_Constant=Infectious_Period_Constant, duration=duration,
            immunity_factor=immunity_mortality_factor, immu_factor_name=Constant.mod_mortality,
            decay_rate=decay_rate, decay_offset=decay_offset
        )

        sft.plot_data(actual_mods_mortality, expected_mods_mortality,
                                   label1="actual mod_mortality", label2="expected mod_mortality",
                                   title="actual vs. expected, offset={0}, decay_rate={1}, immunity_mortality_factor="
                                         "{2}".format(decay_offset, decay_rate, immunity_mortality_factor),
                                   xlabel="time step", ylabel="mod_mortality",
                                   category=os.path.join(path, test_name, 'mod_mortality'), show=True,
                                   line=True
                                   )
        logging.info("test immunity decay mod mortality result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test immunity decay mod mortality passes")
        print("test immunity decay mod mortality passes")

    def _test_immu_decay_mod_mortality_2(self, immunity_mortality_factor=0.6, decay_rate=1.6):
        self.test_immu_decay_mod_mortality(immunity_mortality_factor=immunity_mortality_factor, decay_rate=decay_rate,
                                           test_name="immu_decay_mod_mortality_2")

    def test_immu_decay_mod_mortality_3(self, immunity_mortality_factor=0.6, decay_rate=1):
        self.test_immu_decay_mod_mortality(immunity_mortality_factor=immunity_mortality_factor, decay_rate=decay_rate,
                                           test_name="immu_decay_mod_mortality_3")

    def test_immu_decay_mod_mortality_4(self, immunity_mortality_factor=0.6, decay_rate=0):
        self.test_immu_decay_mod_mortality(immunity_mortality_factor=immunity_mortality_factor, decay_rate=decay_rate,
                                           test_name="immu_decay_mod_mortality_4")

    def test_immu_decay_mod_mortality_5(self, immunity_mortality_factor=0.6, decay_rate=0.8):
        self.test_immu_decay_mod_mortality(immunity_mortality_factor=immunity_mortality_factor, decay_rate=decay_rate,
                                           test_name="immu_decay_mod_mortality_2")

    def test_disease_mortality_off(self, duration=30, initial_population=100, Incubation_Period_Constant=2,
                                   Infectious_Period_Constant=5, Base_Infectivity_Constant=2, base_mortality=0.8,
                                   immunity_mortality_factor=0.9, debug=debug):
        logging.info("test disease mortality off begins")
        print("test disease mortality off begins")

        self.outbreak_timestep = [0, 10, 20]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: 1,
                                 ConfigParameters.Enable_Immune_Decay: 0,
                                 ConfigParameters.Enable_Vital_Dynamics: 0,
                                 ConfigParameters.Enable_Disease_Mortality: 0,
                                 ConfigParameters.Enable_Natural_Mortality: 0,
                                 ConfigParameters.Base_Mortality: base_mortality,
                                 ConfigParameters.Immunity_Mortality_Factor: immunity_mortality_factor
                                 },
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_mortality],
                                             debug=debug)
        path = "./"
        test_name = "disease_mortality_off"
        self.save_files(graveyard, path, test_name, debug)

        death_count = 0
        for t in graveyard:
            death_count += len(graveyard[t])

        self.assertTrue(death_count == 0, "BAD: found {0} individuals in graveyard while expected death "
                                          "is 0".format(death_count))
        logging.info("test disease mortality off passes")
        print("test disease mortality off passes")

    def test_daily_mortality(self, duration=20, initial_population=1000, Incubation_Period_Constant=0,
                             Infectious_Period_Constant=5, Base_Infectivity_Constant=0.00001, base_mortality=0.1,
                             immunity_mortality_factor=0.4, debug=debug):
        logging.info("test disease mortality daily mortality begins")
        print("test disease mortality daily mortality begins")

        self.outbreak_timestep = [0, 10]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: 1,
                                 ConfigParameters.Enable_Immune_Decay: 0,
                                 ConfigParameters.Enable_Vital_Dynamics: 1,
                                 ConfigParameters.Enable_Disease_Mortality: 1,
                                 ConfigParameters.Enable_Natural_Mortality: 0,
                                 ConfigParameters.Mortality_Time_Course: Constant.daily_mortality,
                                 ConfigParameters.Base_Mortality: base_mortality,
                                 ConfigParameters.Immunity_Mortality_Factor: immunity_mortality_factor
                                 },
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_mortality],
                                             debug=debug)
        path = "./"
        test_name = "daily_mortality"
        self.save_files(graveyard, path, test_name, debug)

        succeed = True
        error_msg = []

        death_count = []
        expected_death_count = []
        with open(os.path.join(path, test_name, "test_binomial.txt"), "w") as report_file:
            for t in range(duration):
                actual_death = len(graveyard[t]) if t in graveyard else 0
                df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t]
                mod_mortality = float(df[Constant.mod_mortality][df.index[0]])
                num_infected = int(df[Constant.num_infected][df.index[0]])
                death_probability = base_mortality * mod_mortality
                expected_death = num_infected * death_probability if t not in self.outbreak_timestep else 0.0
                death_count.append(actual_death)
                expected_death_count.append(expected_death)
                if expected_death != 0.0:
                    report_file.write("at time step {0}, the expected death is {1}, actual death is {2}, num of total "
                                      "infected population is {3}.\n".format(t, expected_death, actual_death, num_infected))
                    result = sft.test_binomial_95ci(actual_death, num_infected, death_probability,
                                                        report_file=report_file, category="daily mortality")
                    report_file.write("test_binomial_95ci result is {}.\n".format(result))
                else:
                    result = actual_death == 0

                if not result:
                    succeed = False
                    error_msg.append("BAD: at time step {0} the expected death is {1} "
                                     "while actual death is {2}.".format(t, expected_death, actual_death))

        sft.plot_data(death_count, expected_death_count, label1="actual death",
                                   label2="expected death", title="actual vs. expected, base_mortality={0}, "
                                                                  "immunity_mortality_factor="
                                                                  "{1}".format(base_mortality, immunity_mortality_factor),
                                   xlabel="time step", ylabel="death per day",
                                   category=os.path.join(path, test_name, 'death'), show=True,
                                   line=False)

        logging.info("test disease mortality daily mortality result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test disease mortality daily mortality passes")
        print("test disease mortality daily mortality passes")


    def test_mortality_after_infectious(self, duration=20, initial_population=500, Incubation_Period_Constant=0,
                             Infectious_Period_Constant=3, Base_Infectivity_Constant=0.00001, base_mortality=0.25,
                             immunity_mortality_factor=0.65, debug=debug):
        logging.info("test disease mortality after infectious begins")
        print("test disease mortality after infectious begins")

        self.outbreak_timestep = [0, 5, 10, 15]
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            incubation_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            infectious_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            base_infectivity_distribution_type=ConfigParameters.CONSTANT_DISTRIBUTION,
            other_config_params={ConfigParameters.Incubation_Period_Constant: Incubation_Period_Constant,
                                 ConfigParameters.Infectious_Period_Constant: Infectious_Period_Constant,
                                 ConfigParameters.Base_Infectivity_Constant: Base_Infectivity_Constant,
                                 ConfigParameters.Enable_Immunity: 1,
                                 ConfigParameters.Enable_Immune_Decay: 0,
                                 ConfigParameters.Enable_Vital_Dynamics: 1,
                                 ConfigParameters.Enable_Disease_Mortality: 1,
                                 ConfigParameters.Enable_Natural_Mortality: 0,
                                 ConfigParameters.Mortality_Time_Course: Constant.mortality_after_infectious,
                                 ConfigParameters.Base_Mortality: base_mortality,
                                 ConfigParameters.Immunity_Mortality_Factor: immunity_mortality_factor
                                 },
            initial_population=initial_population)

        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_mortality],
                                             debug=debug)
        path = "./"
        test_name = "mortality_after_infectious"
        self.save_files(graveyard, path, test_name, debug)

        succeed = True
        error_msg = []

        infection_clear_timestep = []
        for t in self.outbreak_timestep:
            infection_clear_timestep.append(t + Incubation_Period_Constant + Infectious_Period_Constant + 1)

        death_count = []
        expected_death_count = []
        with open(os.path.join(path, test_name, "test_binomial.txt"), "w") as report_file:
            for t in range(duration):
                actual_death = len(graveyard[t]) if t in graveyard else 0
                if t in infection_clear_timestep:
                    df = output_df.loc[output_df[ConfigParameters.SIMULATION_TIMESTEP] == t - 1]
                    mod_mortality = float(df[Constant.mod_mortality][df.index[0]])
                    death_probability = base_mortality * mod_mortality
                    num_infected = int(df[Constant.num_infected][df.index[0]])
                    expected_death = num_infected * death_probability
                else:
                    expected_death = 0.0
                death_count.append(actual_death)
                expected_death_count.append(expected_death)
                if expected_death != 0.0:
                    report_file.write("at time step {0}, the expected death is {1}, actual death is {2}, num of total "
                                      "infected population is {3}.".format(t, expected_death, actual_death, num_infected))
                    result = sft.test_binomial_95ci(actual_death, num_infected, death_probability,
                                                        report_file=report_file, category="daily mortality")
                    report_file.write("test_binomial_95ci result is {}.\n".format(result))
                else:
                    result = actual_death == 0

                if not result:
                    succeed = False
                    error_msg.append("BAD: at time step {0} the expected death is {1} "
                                     "while actual death is {2}.".format(t, expected_death, actual_death))

        sft.plot_data(death_count, expected_death_count, label1="actual death",
                                   label2="expected death", title="actual vs. expected, base_mortality={0}, "
                                                                  "immunity_mortality_factor="
                                                                  "{1}".format(base_mortality, immunity_mortality_factor),
                                   xlabel="time step", ylabel="death per day",
                                   category=os.path.join(path, test_name, 'death'), show=True,
                                   line=False)

        logging.info("test disease mortality after infectious result is {}.".format(succeed))
        self.assertTrue(succeed, error_msg)
        logging.info("test disease mortality after infectious passes")
        print("test disease mortality after infectious passes")

    def test_immu_init_dist_s_1(self, initial_population=100, duration=1, debug=debug):
        # disable due to #2026 Pymod: BadEnumInSwitchStatementException with Susceptibility_Initialization_Distribution_Type
        logging.info("test immunity initial distribution simple 1 begins")
        print("test immunity initial distribution simple 1 begins")

        self.outbreak_timestep = []
        self.infect_num = initial_population
        self.configure_incubation_infectious_distribution(
            other_config_params={ConfigParameters.Enable_Immunity: 1,
                                 ConfigParameters.Enable_Initial_Susceptibility_Distribution: 1,
                                 ConfigParameters.Susceptibility_Initialization_Distribution_Type:
                                 ConfigParameters.DISTRIBUTION_SIMPLE},
            initial_population=initial_population)
        # self.configure_immunity_distribution(flag=flag, param1=param1,param2=param2, demo_template_filename="demographics.json")
        graveyard, output_df = self.run_test(duration=duration,
                                             serial_properties=[[Constant.susceptibility, Constant.mod_acquire],
                                                                [Constant.susceptibility, Constant.mod_transmit],
                                                                [Constant.susceptibility, Constant.mod_mortality]],
                                             properties_name=[Constant.mod_acquire, Constant.mod_transmit, Constant.mod_mortality],
                                             debug=debug)
        path = "./"
        test_name = "immu_init_dist_s_1"
        self.save_files(graveyard, path, test_name, debug)

        error_msg = []
        succeed = True

        mod_acquire_list = []
        mod_transmit_list = []
        mod_mortality_list = []
        for hum_id in range(1, initial_population):
            df = output_df.loc[output_df[Constant.hum_id] == hum_id]
            mod_acquire = float(df[Constant.mod_acquire][df.index[0]])
            mod_transmit = float(df[Constant.mod_transmit][df.index[0]])
            mod_mortality = float(df[Constant.mod_mortality][df.index[0]])
            mod_acquire_list.append(mod_acquire)
            mod_transmit_list.append(mod_transmit)
            mod_mortality_list.append(mod_mortality)

        # ToDo: write the test part

        # sft.plot_data(output_df[Constant.mod_mortality].tolist(), expected_mod_mortality,
        #                            label1="actual mod_mortality", label2="expected mod_mortality",
        #                            title="mod_mortality, actual vs. expected",
        #                            xlabel="time step", ylabel=None, category='mod_mortality', show=True,
        #                            line=False
        #                            )
        # logging.info("test immunity initial distribution simple 1 result is {}.".format(succeed))
        # self.assertTrue(succeed, error_msg)
        # logging.info("test immunity initial distribution simple 1 passes")
        # print("test immunity initial distribution simple 1 passes")

if __name__ == "__main__":
    unittest.main()
