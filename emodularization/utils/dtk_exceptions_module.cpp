/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>
#include <climits>

#include "Exceptions.h"

static PyObject*
basic(PyObject* self, PyObject* args)
{
    /*
    float increment;

    if (!PyArg_ParseTuple(args, "f", &increment))
        return NULL;

    _instance.Update( increment );
    */
    throw Kernel::DetailedException( __FILE__, __LINE__, __FUNCTION__ );

    Py_RETURN_NONE;
}

static PyObject*
ice_float(PyObject* self, PyObject* args)
{
    abort();
}

static PyObject*
ice_long(PyObject* self, PyObject* args)
{
    try {
        throw Kernel::IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Existing Variable A", (unsigned long)(LONG_MAX), "Test Variable B", (unsigned long)(INT_MAX), "Can't use these values together." );
    }
    catch( Kernel::DetailedException &exc )
    {
        std::cout << exc.GetMsg() << std::endl;
    }

}

static PyMethodDef DtkExceptionsMethods[] =
{
     {"basic", basic, METH_VARARGS, "Base class."}, 
     {"ice_float", ice_float, METH_VARARGS, "IceFloat."}, 
     {"ice_long", ice_long, METH_VARARGS, "IceLong."}, 
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_exceptions",
    nullptr,
    0,
    DtkExceptionsMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_exceptions(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
