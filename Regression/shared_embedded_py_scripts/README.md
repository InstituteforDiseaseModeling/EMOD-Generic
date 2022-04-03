# SHARED_EMBEDDED_PY_SCRIPTS

The python scripts in this directory are generally used as embedded Python pre- and post-processing scripts when running the DTK, part of the overall EP4 vision.

Originally they were just scripts used by SFTs. But now we are moving away from just copying individual python scripts around and instead moving towards properly packaged (and versioned and deployed) python modules.

All the SFT support scripts are now part of the dtk_test package or module. As such, their proper home (exceptions to follow) is in the dtk_test subdirectory.

There are other scripts that will become part of the dtk_utils module (which exists in a very rudimentary form at this time) or maybe they will become part of a dtk_ep4 module. We are in the process of figuring this out.

At the moment, the algorithm in rt.py does the following:
- IF Python_Script_Path is set to "SHARED" then:
	- Any file starting with 'dtk_pre_' or 'dtk_post_' gets copied to the simulation directory.
	- Any other file starting with 'dtk_' gets copied to a dtk_test subdirectory in the simulation directory.
		- An __init__.py touch file is created in the simdir/dtk_test subdirectory so it is priority importable by python scripts in the simdir.
	- Ideally the dtk_test python scripts in shared_embedded_py_scripts/ are symlinks to the actual files in shared_embedded_py_scripts/dtk_test/...
		- Except new ones; and
		- Except ones that are being edited/developed in this branch.
		- One says 'ideally' because there are windows issues with this solution and a final decision about whether to go with a one-size-fits-all nasty copying solution or an OS-optimized solution is pending.
	- In master, shared_embedded_py_scripts should really be emptied out and the files put into dtk_test, dtk_utils, dtk_ep4 (or whatever) package subdirectories, and new modules published, etc.
