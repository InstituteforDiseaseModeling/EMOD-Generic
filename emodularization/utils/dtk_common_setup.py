from distutils.core import setup, Extension

import os

def doitall( ext_name, src_list ):
    compiler_args = None
    extralibs = []

    print( "\nThis assumes that utils.lib, campaign.lib, and cajun.lib (Release) have already been built.\n" )

    my_library_dirs = None
    if os.name == "posix":
        os.environ['CC'] = 'gcc'
        os.environ['CXX'] = 'g++'
        os.environ['CPP'] = 'g++'
        compiler_args = [ "-std=c++11", "-w", "-fpermissive" ]
        my_library_dirs = [ 
                '../../build/x64/Release/utils/',
                '../../build/x64/Release/campaign/',
                '../../build/x64/Release/cajun/',
            ]
    else:
        print( "If this doesn't work, try setting: 'SET VS90COMNTOOLS=%VS140COMNTOOLS%' for Visual Studio 2015 (VS14). " ) 
        extralibs = [ "ws2_32", "DbgHelp" ]
        compiler_args = [ 
            "/c", "/nologo", "/EHsc", "/W3", "/bigobj", "/errorReport:none", "/fp:strict", "/GS-", "/Oi", "/Ot", "/Zc:forScope", "/Zc:wchar_t", "/Z7", "/DIDM_EXPORT", "/O2", "/MD", "/DWIN32", "/D_UNICODE", "/DUNICODE"
            ]
        my_library_dirs = [ "../../x64/Release" ]

    my_include_dirs = [
                '../../interventions/',
                '../../campaign/',
                '../../utils/', 
                '../../cajun/include/', 
                '../../rapidjson/include/', 
                '../../Eradication/', 
                ]
    if 'IDM_BOOST_PATH' in os.environ:
        my_include_dirs.append( os.environ['IDM_BOOST_PATH'] )

    module1 = Extension(
            ext_name,
            sources = src_list,
            extra_compile_args = compiler_args,
            include_dirs= my_include_dirs,
            libraries = ['utils', 'campaign', 'cajun' ] + extralibs,
            library_dirs = my_library_dirs 
            )

    setup (name = 'PackageName',
            version = '1.0',
            description = 'This is a demo package',
            ext_modules = [module1]) 
