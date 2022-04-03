@ECHO OFF

REM include two directories
set Include_Dir="C:\Users\tfischle\Github\DtkInput\MigrationTest;."

REM create config.json, flatten
python -m dtk_utils.flatten_config param_overrides.json

REM copy param_overides file
copy config.json config_create_sp.json

REM run python script and set parameters for serialization
python set_serialize_parameters.py

REM run eradictaion with changed config
..\..\..\Eradication\x64\Release\eradication.exe -C config_create_sp.json -I %Include_Dir%

REM copy created dtk file
move output\state-00015.dtk .

REM run eradictaion with original config
..\..\..\Eradication\x64\Release\eradication.exe -C config.json -I %Include_Dir%

PAUSE