/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>

#include <iostream>
#include "Timers.h"

Kernel::CountdownTimer _instance = 25;
struct Handle {
    void operator()(float i) const
    {
        std::cout << "Expired!" << std::endl;
    }
};

static PyObject*
init(PyObject* self, PyObject* args)
{
    std::cout << "init" << std::endl;
    _instance.handle = Handle();
    std::cout << "init (done)" << std::endl; 
	Py_RETURN_NONE;
}

static PyObject*
dec(PyObject* self, PyObject* args)
{ 
    float dt;

    if (!PyArg_ParseTuple(args, "f", &dt))
        return NULL;

    _instance.Decrement( dt );

    Py_RETURN_NONE;
}

static PyObject*
expired(PyObject* self, PyObject* args)
{ 
    bool exp = _instance.IsDead();
    return Py_BuildValue("b", exp);
}


static PyMethodDef DtkTimersMethods[] =
{
     {"init",    init, METH_VARARGS, "Initialize."},
     {"dec",     dec, METH_VARARGS, "Decrement."},
     {"expired", expired, METH_VARARGS, "Expired."},
     
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef {
    PyModuleDef_HEAD_INIT,
    "dtk_timers",
    nullptr,
    0,
    DtkTimersMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_timers(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
