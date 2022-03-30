import setuptools
from setuptools.extension import Extension

with open("README.md", "r") as fh:
    long_description = fh.read()
    ext_name = "dtk_utils"

setuptools.setup(
    name=ext_name,
    version="0.0.2",
    author="Jonathan Bloedow",
    author_email="jbloedow@idmod.org",
    description="IDM's DTK pre and post-processing support scripts",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/pypa/sampleproject",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ]
)
