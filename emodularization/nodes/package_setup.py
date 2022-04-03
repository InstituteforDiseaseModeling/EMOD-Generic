import setuptools
from setuptools.extension import Extension

from sources import *
from dtk_pymodbuild import *
import dtk_nd_ver

with open("README.md", "r") as fh:
    long_description = fh.read()
    ext_name = "dtk_nodedemog"

    module1 = Extension(
            ext_name,
            sources = sources,
            extra_compile_args = compiler_args,
            include_dirs= my_include_dirs,
            libraries = ['utils', 'campaign', 'cajun', 'baseReportLib' ] + extralibs,
            library_dirs = my_library_dirs 
            )

setuptools.setup(
    name=ext_name,
    version=dtk_nd_ver.version,
    author="Jonathan Bloedow",
    author_email="jbloedow@idmod.org",
    description="IDM's DTK Node Demographics 'pymod'",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/pypa/sampleproject",
    #packages=setuptools.find_packages(),
    packages=[ "dtk_app" ],
    package_data={
        "dtk_app": [ "*.json" ]
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    ext_modules = [module1],
    include_package_data=False
)
