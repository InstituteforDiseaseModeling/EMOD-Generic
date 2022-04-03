/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>

#include "IDistribution.h"
#include "DistributionFactory.h"
#include "Environment.h"
#include "RANDOM.h"

Kernel::RANDOMBASE* p_RNG = nullptr;

void
pyMathFuncInit()
{
    if( p_RNG == nullptr )
    {
        unsigned int randomseed[2];
        randomseed[0] = 0;
        randomseed[1] = 0;
        p_RNG = new Kernel::PSEUDO_DES(*reinterpret_cast<uint32_t*>(randomseed));
    }
}

static PyObject*
getFixedDraw(PyObject* self, PyObject* args)
{
    float param;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "f", &param))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::CONSTANT_DISTRIBUTION );
    distribution->SetParameters( param, 0.0, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getUniformDraw(PyObject* self, PyObject* args)
{
    float left, right;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &left, &right))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::UNIFORM_DISTRIBUTION );
    distribution->SetParameters( left, right, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getGaussianDraw(PyObject* self, PyObject* args)
{
    float mean, stddev;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &mean, &stddev))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::GAUSSIAN_DISTRIBUTION );
    distribution->SetParameters( mean, stddev, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getExponentialDraw(PyObject* self, PyObject* args)
{
    float param;
    pyMathFuncInit();

    if( !PyArg_ParseTuple(args, "f", &param ) )
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::EXPONENTIAL_DISTRIBUTION );
    distribution->SetParameters( param, 0.0, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getPoissonDraw(PyObject* self, PyObject* args)
{
    float param;
    pyMathFuncInit();

    if( !PyArg_ParseTuple(args, "f", &param ) )
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::POISSON_DISTRIBUTION );
    distribution->SetParameters( param, 0.0, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getLogNormalDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::LOG_NORMAL_DISTRIBUTION );
    distribution->SetParameters( param1, param2, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getBimodalDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::DUAL_CONSTANT_DISTRIBUTION);
    distribution->SetParameters( param1, param2, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyObject*
getWeibullDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    Kernel::IDistribution* distribution = Kernel::DistributionFactory::CreateDistribution( Kernel::DistributionFunction::WEIBULL_DISTRIBUTION );
    distribution->SetParameters( param1, param2, 0.0 );
    float value = distribution->Calculate( p_RNG );

    return Py_BuildValue("f", value);
}

static PyMethodDef DtkMathFuncMethods[] =
{
     {"get_fixed_draw", getFixedDraw, METH_VARARGS, "Draw from FIXED distribution."},
     {"get_uniform_draw", getUniformDraw, METH_VARARGS, "Draw from UNIFORM distribution."},
     {"get_gaussian_draw", getGaussianDraw, METH_VARARGS, "Draw from GAUSSIAN distribution."},
     {"get_exponential_draw", getExponentialDraw, METH_VARARGS, "Draw from EXPONENTIAL distribution."},
     {"get_poisson_draw", getPoissonDraw, METH_VARARGS, "Draw from POISSON distribution."},
     {"get_lognormal_draw", getLogNormalDraw, METH_VARARGS, "Draw from LOGNORMAL distribution."},
     {"get_bimodal_draw", getBimodalDraw, METH_VARARGS, "Draw from BIMODAL distribution."},
     {"get_weibull_draw", getWeibullDraw, METH_VARARGS, "Draw from WEIBULL distribution."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_mathfunc",
    nullptr,
    0,
    DtkMathFuncMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_mathfunc(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
