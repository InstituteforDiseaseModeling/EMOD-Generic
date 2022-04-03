# DTK regression/commission support package including pre- and post-processing scripts.

These scripts are created and supported by the IDM Dev team as part of the Regression Testing work. This module should house generally useful utility scripts and/or pre- and post-processing (EP4) scripts for the DTK. This is an IDM-internal module at this time.

To turn a param_overrides.json into a config.json, do:

```
python -m dtk_utils.flatten_config /path/to/param_overrides.json
```

If you don't have dtk_utils, you can install from prod via:
```
python -m pip install dtk_utils --index-url https://packages.idmod.org/api/pypi/idm-pypi-production/simple --upgrade
```
