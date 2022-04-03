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
#include "RANDOM.h"

static RANDOM test_random;

static PyObject*
init(PyObject* self, PyObject* args)
{
	Py_RETURN_NONE;
}

static PyObject*
next(PyObject* self, PyObject* args)
{
    float e = test_random.e();
    return Py_BuildValue("f", e);
}

PyObject*
gauss(PyObject* self, PyObject* args)
{
    float e = test_random.eGauss();
    return Py_BuildValue("f", e);
}

PyObject*
gaussNonNeg(PyObject* self, PyObject* args)
{
    float mu, sig;

    if (!PyArg_ParseTuple(args, "ff", &mu, &sig ))
    {
        std::cout << "Failed to parse mu and/or sigma param values." << std::endl;
        return NULL;
    }
    float e = test_random.eGaussNonNeg( mu, sig );
    return Py_BuildValue("f", e);
}

PyObject*
poisson(PyObject* self, PyObject* args)
{
    auto e = test_random.Poisson();
    return Py_BuildValue("l", e);
}

PyObject*
poissonTrue(PyObject* self, PyObject* args)
{
    auto e = test_random.Poisson_true();
    return Py_BuildValue("f", e);
}

PyObject*
expdist(PyObject* self, PyObject* args)
{
    auto e = test_random.expdist();
    return Py_BuildValue("f", e);
}

PyObject*
weibull(PyObject* self, PyObject* args)
{
    auto e = test_random.Weibull();
    return Py_BuildValue("f", e);
}

PyObject*
weibull2(PyObject* self, PyObject* args)
{
    auto e = test_random.Weibull2();
    return Py_BuildValue("f", e);
}

PyObject*
loglogistic(PyObject* self, PyObject* args)
{
    auto e = test_random.LogLogistic();
    return Py_BuildValue("f", e);
}

PyObject*
binom(PyObject* self, PyObject* args)
{
    auto e = test_random.binomial_true();
    return Py_BuildValue("f", e);
}

PyObject*
binomApprox(PyObject* self, PyObject* args)
{
    auto e = test_random.binomial_approx();
    return Py_BuildValue("f", e);
}

PyObject*
binomApprox2(PyObject* self, PyObject* args)
{
    auto e = test_random.binomial_approx2();
    return Py_BuildValue("f", e);
}

PyObject*
timeVaryingRateDist(PyObject* self, PyObject* args)
{
    //auto e = test_random.time_varying_rate_dist();
    //return Py_BuildValue("f", e);
    //double time_varying_rate_dist( std::vector <float> v_rate, float timestep, float rate);
    Py_RETURN_NONE;
}

 

static PyMethodDef DtkRANDOMMethods[] =
{
     {"init",   init, METH_VARARGS, "Initialize."},
     {"random", next, METH_VARARGS, "Next random number."},
     {"gauss",  gauss, METH_VARARGS, "Gaussian random number."},
     {"gauss_non_neg",  gaussNonNeg, METH_VARARGS, "Non-negative Gaussian random number."},
     {"expdist",  expdist, METH_VARARGS, "Exponential random number (mean=1)."},
     {"poisson",  poisson, METH_VARARGS, "Poisson random number (mean=1)."},
     {"weibull",  weibull, METH_VARARGS, "Weibull random number (mean=1)."},
     {"weibull2",  weibull2, METH_VARARGS, "Weibull random number (mean=1)."},
     {"loglogistic",  loglogistic, METH_VARARGS, "Loglogistic random number (mean=1)."},
     {"binomial",  binom, METH_VARARGS, "Binomial random number (mean=1)."},
     {"binomial_approx",  binomApprox, METH_VARARGS, "..."},
     {"binomial_approx2",  binomApprox2, METH_VARARGS, "..."},
     {"tvrd",  timeVaryingRateDist, METH_VARARGS, "..."},
     
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_random",
    nullptr,
    0,
    DtkRANDOMMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_random(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
