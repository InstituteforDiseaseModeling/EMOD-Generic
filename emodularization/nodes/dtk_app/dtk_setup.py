#!/usr/bin/python

import sys
import os
import site
import shutil

#pkgdir = sys.modules['dtk_nodedemog'].__path__[0]
for sp in site.getsitepackages():
    moddir = os.path.join( sp, "dtk_app" )
    if os.path.exists( moddir ):
        for copyfile in [ "transmission_demo.py", "nd.json", "gi.json", "demographics.json", "sv.json" ]:
            fullpath = os.path.join( moddir, copyfile )
            shutil.copy(fullpath, os.getcwd())
