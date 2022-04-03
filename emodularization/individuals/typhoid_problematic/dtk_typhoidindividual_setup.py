#!/usr/bin/python

from distutils.core import setup, Extension

import os
os.environ['CC'] = 'g++'
os.environ['CXX'] = 'g++'
os.environ['CPP'] = 'g++'
#os.environ['LDSHARED'] = 'g++'

module1 = Extension('dtk_typhoidindividual',
        sources = [
            'dtk_typhoidindividual_module.c',
            '../../../Eradication/Individual.cpp',
            '../../../Eradication/Susceptibility.cpp',
            '../../../Eradication/Infection.cpp',
            '../../../Eradication/InterventionsContainer.cpp',
            '../../../Eradication/IndividualTyphoid.cpp',
            '../../../Eradication/SusceptibilityTyphoid.cpp',
            '../../../Eradication/InfectionTyphoid.cpp',
            '../../../Eradication/TyphoidInterventionsContainer.cpp',
            '../../../Eradication/IndividualEnvironmental.cpp',
            '../../../Eradication/SusceptibilityEnvironmental.cpp',
            '../../../Eradication/InfectionEnvironmental.cpp',
            '../../../Eradication/NodeDemographics.cpp',
            '../../../Eradication/StrainIdentity.cpp',
            '../../../Eradication/TransmissionGroupsBase.cpp'
        ],
        extra_compile_args = ["-std=c++11", "-w", "-fpermissive", "-DENABLE_TYPHOID", "-DENABLE_LOG_VALID" ], 
        include_dirs=[
            '../../../interventions/',
            '../../../campaign/',
            '../../../utils/', 
            '../../../cajun/include/', 
            '../../../rapidjson/include/', 
            '../../../Eradication/', 
            ],
        libraries = ['utils', 'campaign', 'cajun' ],
        library_dirs = [
            '../../../build/x64/Release/utils/',
            '../../../build/x64/Release/campaign/',
            '../../../build/x64/Release/cajun/',
            ]
        )

setup (name = 'PackageName',
        version = '1.0',
        description = 'This is a demo package',
        ext_modules = [module1])
