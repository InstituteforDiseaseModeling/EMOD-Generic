from distutils.core import setup, Extension

import os
os.environ['CC'] = 'g++'
os.environ['CXX'] = 'g++'
os.environ['CPP'] = 'g++'

module1 = Extension('dtk_tyvac',
        sources = [
            'dtk_tyvac_module.c',
            '../../interventions/TyphoidVaccine.cpp',
            '../../Eradication/InterventionsContainer.cpp',
            '../../Eradication/EventTrigger.cpp',
            '../../interventions/WaningEffectBox.cpp',
            '../../interventions/WaningEffectExponential.cpp',
            '../../interventions/WaningEffectBoxExponential.cpp',
            '../../interventions/WaningEffectFactory.cpp'
        ],
        extra_compile_args = ["-std=c++11", "-w", "-fpermissive", "-save-temps", "-DENABLE_TYPHOID" ], 
        include_dirs=[
            '../../interventions/',
            '../../campaign/',
            '../../utils/', 
            '../../cajun/include/', 
            '../../rapidjson/include/', 
            '../../Eradication/', 
            ],
        libraries = ['utils', 'campaign', 'cajun' ],
        library_dirs = [
            '../../build/x64/Release/utils/',
            '../../build/x64/Release/campaign/',
            '../../build/x64/Release/cajun/',
            ]
        )

setup (name = 'PackageName',
        version = '1.0',
        description = 'Typhoid Vaccine',
        ext_modules = [module1])
