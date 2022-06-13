/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "PythonSupport.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "Debug.h"
#include "Log.h"

#define ENV_VAR_PYTHON              "IDM_PYTHON3X_PATH"
#define PYTHON_SCRIPT_PATH_NOT_SET  ""

SETUP_LOGGING("PythonSupport")

namespace Kernel
{
    std::string PythonSupport::SCRIPT_PRE_PROCESS         = "dtk_pre_process";
    std::string PythonSupport::SCRIPT_POST_PROCESS        = "dtk_post_process";
    std::string PythonSupport::SCRIPT_POST_PROCESS_SCHEMA = "dtk_post_process_schema";
    std::string PythonSupport::SCRIPT_IN_PROCESS          = "dtk_in_process";
    std::string PythonSupport::SCRIPT_PYTHON_FEVER        = "dtk_pydemo_individual";
    std::string PythonSupport::SCRIPT_TYPHOID             = "dtk_typhoid_individual";

    std::string PythonSupport::FUNCTION_NAME              = "application";

    bool PythonSupport::m_PythonInitialized               = false; 
    std::string PythonSupport::m_PythonScriptPath         = PYTHON_SCRIPT_PATH_NOT_SET;

    // PyObjectsMap[ModuleName][MemberName]
    std::map<std::string, std::map<std::string, void*>> 
                              PythonSupport::PyObjectsMap = {{PythonSupport::SCRIPT_PRE_PROCESS,         std::map<std::string, void*>()},
                                                             {PythonSupport::SCRIPT_POST_PROCESS,        std::map<std::string, void*>()},
                                                             {PythonSupport::SCRIPT_POST_PROCESS_SCHEMA, std::map<std::string, void*>()},
                                                             {PythonSupport::SCRIPT_IN_PROCESS,          std::map<std::string, void*>()},
                                                             {PythonSupport::SCRIPT_PYTHON_FEVER,        std::map<std::string, void*>()},
                                                             {PythonSupport::SCRIPT_TYPHOID,             std::map<std::string, void*>()}};

    PythonSupport::PythonSupport()
    { }

    PythonSupport::~PythonSupport()
    {
        cleanPython();
    }

    bool PythonSupport::IsPythonInitialized()
    {
        return m_PythonInitialized;
    }

    void PythonSupport::SetupPython( const std::string& pythonScriptPath )
    {
        m_PythonScriptPath = pythonScriptPath;

#ifdef ENABLE_PYTHON
        if( m_PythonScriptPath == PYTHON_SCRIPT_PATH_NOT_SET )
        {
            LOG_INFO( "Python not initialized because --python-script-path (-P) not set.\n" );
            return;
        }
        else
        {
            LOG_INFO_F( "Python script path: %s\n", m_PythonScriptPath.c_str() );
        }

#ifdef WIN32
        // Get path to python installation
        char* c_python_path = getenv(ENV_VAR_PYTHON);
        if( c_python_path == nullptr )
        {
            std::stringstream msg;
            msg << "Cannot find environmental variable " << ENV_VAR_PYTHON << ".\n";
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        std::string python_home = c_python_path;
        python_home = FileSystem::RemoveTrailingChars( python_home );
        LOG_INFO_F( "Python home path: %s\n", python_home.c_str() );

        if( !FileSystem::DirectoryExists( python_home ) )
        {
            std::stringstream msg;
            msg << ENV_VAR_PYTHON << "=" << python_home << ". Directory was not found.";
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        SetDllDirectoryA(c_python_path);
        Py_SetPythonHome( Py_DecodeLocale( python_home.c_str(), nullptr ) );
#endif

        // Open a python instance; remember to cleanPython before throwing an exception
        Py_Initialize();
        m_PythonInitialized = true;

        // Prepend python-script-path and current working directory to current python instance.
        PyObject* sys_path = PySys_GetObject("path");
        release_assert( sys_path );

        PyObject* path_user = PyUnicode_FromString( m_PythonScriptPath.c_str() );
        release_assert( path_user );
        release_assert( !PyList_Insert(sys_path, 0, path_user) );

        PyObject* path_default = PyUnicode_FromString( "" );
        release_assert( path_default );
        release_assert( !PyList_Insert(sys_path, 0, path_default) );

        // Import all possible modules and module contents on setup; re-assign nullptr if script not present
        bool found_py_module = false;
        for (auto PyModuleEntry : PyObjectsMap)
        {
            bool module_found  = ImportPyModule( PyModuleEntry.first );
            found_py_module   |= module_found;
        }

        // Simple business-logic check to see if python initialized but no modules.
        if( m_PythonInitialized && !found_py_module )
        {
            std::stringstream msg;
            msg << "--python-script-path specified but no python scripts found.";
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }
#endif
        return;
    }

    void PythonSupport::cleanPython()
    {
#ifdef ENABLE_PYTHON
        // Close python instance (flush stdout/stderr)
        if( m_PythonInitialized )
        {
            if(PyErr_Occurred())
            {
                PyErr_Print();
            }
            Py_FinalizeEx();
            m_PythonInitialized = false;
        }
        // Best practice is to open (Py_Initialize) and close (Py_FinalizeEx) the interpreter 
        // around every script execution. Unfortunately, some dynamically imported modules
        // (see numpy/issues/8097) aren't cleaned up properly, and importing a second time 
        // after a restart will crash everything. Instead, Py_Initialize is invoked once during
        // SetupPython and Py_FinalizeEx is only invoked in cleanPython. 
#endif
        return;
    }

    std::string PythonSupport::RunPyFunction( const std::string& arg_string, const std::string& python_module_name, const std::string& python_function_name )
    {
        std::string returnString = arg_string;

        // Return immediately if no python
        if( !m_PythonInitialized )
        {
            return returnString;
        }

#ifdef ENABLE_PYTHON
        // Flush output in preparation for processing
        EnvPtr->Log->Flush();

        // Check if module present
        if( PyObjectsMap.count(python_module_name) == 0 || PyObjectsMap[python_module_name].count(python_function_name) == 0 )
        {
            return returnString;
        }

        // Get function pointer from script
        PyObject* pFunc = static_cast<PyObject*>(GetPyFunction( python_module_name, python_function_name ));

        // Call python function with arguments
        PyObject* vars   = Py_BuildValue( "(s)", arg_string.c_str() );
        PyObject* retVal = PyObject_CallObject( pFunc, vars );

        if(!retVal)
        {
            std::stringstream msg;
            msg << "Python function '" << python_function_name << "' in " << python_module_name << ".py failed.";
            cleanPython();
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // Local instance of NoneType
        PyObject* none_type = Py_BuildValue("");

        // Handle return value
        if(PyUnicode_Check(retVal))
        {
            wchar_t*    utf_chars;
            Py_ssize_t  num_chars = PyUnicode_GetSize(retVal) + 1;

            // Possible data loss if return string is non-UTF8
            utf_chars = static_cast<wchar_t*>(malloc(num_chars*sizeof(wchar_t)));
            PyUnicode_AsWideChar(retVal, utf_chars, num_chars);
            std::wstring temp_wstr(utf_chars);
            free(utf_chars);
            returnString.assign(temp_wstr.begin(), temp_wstr.end());
        }
        else if (retVal != none_type)
        {
            std::stringstream msg;
            msg << "Python function '" << python_function_name << "' in " << python_module_name
                << ".py did not provide an expected return value. Expected: string or None.";
            cleanPython();
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // Ensure output
        PyObject* sys_stdout = PySys_GetObject("stdout");
        release_assert( sys_stdout );
        release_assert(PyObject_CallMethod(sys_stdout, "flush", nullptr));

        // Memory cleanup
        Py_XDECREF(vars);
        Py_XDECREF(retVal);
        Py_XDECREF(none_type);
#endif
        return returnString;
    }

    void* PythonSupport::GetPyFunction( const std::string& python_module_name, const std::string& python_function_name )
    {
        void* retVal = nullptr;

#ifdef ENABLE_PYTHON
        // Check if module present
        if( PyObjectsMap.count(python_module_name) == 0 )
        {
            std::stringstream msg;
            msg << "Python module " << python_module_name << " does not exist.";
            cleanPython();
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // Check if module present
        if( PyObjectsMap[python_module_name].count(python_function_name) == 0 )
        {
            std::stringstream msg;
            msg << "Python module " << python_module_name << " does not contain function '"
                << python_function_name << "'.";
            cleanPython();
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        retVal = PyObjectsMap[python_module_name][python_function_name];
#endif
        // Return pointer to an unverified python function; can't verify here with a delay-load dll
        return retVal;
    }

    bool PythonSupport::ImportPyModule( const std::string& python_module_name )
    {
#ifdef ENABLE_PYTHON
        LOG_DEBUG_F( "Checking current working directory and python script path for embedded python script %s.\n", python_module_name.c_str() );
        // Check for script on two paths: python-script-path and current directory
        if( !FileSystem::FileExistsInPath( ".", python_module_name+".py" ) &&
            !FileSystem::FileExistsInPath( m_PythonScriptPath, python_module_name+".py" ) )
        {
            LOG_DEBUG("File not found.\n");
            return false;
        }

        // Loads contents of script as a module into current python instance
        PyObject* pName = PyUnicode_FromString( python_module_name.c_str() );
        release_assert( pName );
        PyObject* pModule = PyImport_Import( pName );
        if(PyErr_Occurred())
        {
            std::stringstream msg;
            msg << "Python script '" << python_module_name << "' failed to import as module.";
            cleanPython();
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // Get dictionary of module contents
        PyObject* pDict = PyModule_GetDict( pModule );
        release_assert( pDict );

        PyObject *dict_key, *dict_value;
        Py_ssize_t dict_pos = 0;

        // Store pointers to everything within the python module
        while (PyDict_Next(pDict, &dict_pos, &dict_key, &dict_value))
        {
            wchar_t*    utf_chars;
            Py_ssize_t  num_chars = PyUnicode_GetSize(dict_key) + 1;

            // Possible data loss if function name is non-UTF8
            utf_chars = static_cast<wchar_t*>(malloc(num_chars*sizeof(wchar_t)));
            PyUnicode_AsWideChar(dict_key, utf_chars, num_chars);
            std::wstring temp_wstr(utf_chars);
            free(utf_chars);
            std::string python_dict_entry(temp_wstr.begin(), temp_wstr.end());

            if(PyCallable_Check(dict_value))
            {
                PyObjectsMap[python_module_name][python_dict_entry] = dict_value;
            }
        }
#endif
        return true;
    }

}