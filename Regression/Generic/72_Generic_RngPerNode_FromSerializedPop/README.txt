This test loads a simulation from a serialized population file, runs for some time steps and saves the output.

If some internals of the dtk change e.g. renaming of variables or new variables, the *.dtk files need to be updated.
Running update_sp_file.bat will create config_create_sp.json with the necessary paramters to create a new *.dtk file.
Furthermore, it will move the created file to the correct position and run eradication.exe.

The script assumes that
- eradication.exe is located at ..\..\..\Eradication\x64\Release\eradication.exe
- include directory is given
- dtk_utils are installed (https://wiki.idmod.org/pages/viewpage.action?pageId=89719199)
