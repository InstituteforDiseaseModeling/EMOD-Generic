# DTK SFT support package (dtk_test)

These scripts are created and supported by the IDM Test team as part of the Scientific Feature Testing work. 

All the scripts that make up the **dtk_test** package (that would get installed by doing a pip install) now live in the **DtkTest/dtk_test/** subdirectory.

You can develop new **dtk_test** code (or modify existing) by changing the files here. You get rt.py to use this code instead of the installed package (which should be what's in this folder in master) by creating a symlnk from "Regression/shared_embedded_py_scripts/**dtk_test**" to "Regression/shared_embedded_py_scripts/DtkTest/**dtk_test**". I'm afraid on operating systems that don't support symlinks, you will have to copy instead. This is unfortunate to say the least.

Note that rt.py looks for "**dtk_test**" in "Reg/s_e_ps" and copies that module to the simdir IF PYTHON_SCRIPT_PATH is set to SHARED (and if you're running an SFT suite). If it doesn't find **dtk_test**, it silently moves on -- and the module from site-packages will be imported. Application (or client) code remains identical.

Note that python will prefer a local copy of a module you import over one found in site-packages.

Some additional notes:
- The .py files here are ONLY copied if the Python_Script_Path in the config.json is "SHARED". That's literally what that means. It means we need python scripts from 'somewhere else' as well as '.'
- Files really should not live in this directory for very long in master. They should be bundled into a new version of **dtk_test**, uploaded, installed, etc. ASAP.
- Importing shared files under development right in s_e_p_s can be done like `import my_new_shared_support_script` whereas once it's migrated into the **dtk_test** package/module, the import will be `import dtk_test.my_new_shared_support_script`.
- The shared Python directory specified in rt.cfg is gone.
- No, I am not worried about the additional disk usage of a few extra copies of python scripts in simulation folders on bayesian. If someone else is, feel free to add code to rt.py to remove them (and maybe the entire sim dir) upon completion.
- When merging a branch with modified **dtk_test** on Windows, copy all the files back to DtkTest/**dtk_test**, merge w/ master, update the version, and republish.


