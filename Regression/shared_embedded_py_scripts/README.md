# DTK SFT support package (dtk_test)

These scripts are created and supported by the IDM Test team as part of the Scientific Feature Testing work. 

All the scripts that make up the **dtk_test** package (that would get installed by doing a pip install) now live in the **dtk_test** subdirectory.

Any script that lives in this directory is considered "under active development". That means rt.py will copy it from here. The files in **dtk_test** do not get copied by rt.py. They should be installed and thus live in site-packages and are available to python by doing an `import dtk_test`.

Scripts/files that are in this directory, and thus under active development, and thus copied by rt.py are no longer copied into the shared Python directory specified in rt.cfg. They are copied to the simulation folder itself. Thus they are available by `import <file>`. The directory of the dtk_post_process.py (top level script) is always available in the sys path.

Some additional notes:
- The .py files here are ONLY copied if the Python_Script_Path in the config.json is "SHARED". That's literally what that means. It means we need python scripts from 'somewhere else' as well as '.'
- Files really should not live in this directory for very long in master. They should be bundled into a new version of **dtk_test**, uploaded, installed, etc. ASAP.
- Importing shared files under development will be done like `import my_new_shared_support_script` whereas once it's migrated into the dtk_test package/module, the import will be `import dtk_test.my_new_shared_support_script`.
- The shared Python directory specified in rt.cfg would appeared to be deprecated by this change at least as far as master is concerned.
- No, I am not worried about the additional disk usage of a few extra copies of python scripts in simulation folders on bayesian. If someone else is, feel free to add code to rt.py to remove them (and maybe the entire sim dir) upon completion.


