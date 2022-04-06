/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>

#include "RANDOM.h"

Kernel::RANDOMBASE* p_RNG = nullptr;

void pyRandomInit()
{
    if( p_RNG == nullptr )
    {
        unsigned int randomseed[2];
        randomseed[0] = 0;
        randomseed[1] = 0;
        p_RNG = new Kernel::PSEUDO_DES(*reinterpret_cast<uint32_t*>(randomseed));
    }
}

static PyObject* rand(PyObject* self, PyObject* args)
{
    pyRandomInit();

    float retval = p_RNG->e();
    return Py_BuildValue("f", retval);
}

static PyObject* gauss(PyObject* self, PyObject* args)
{
    pyRandomInit();

    double retval = p_RNG->eGauss();
    return Py_BuildValue("d", retval);
}

static PyObject* gaussNonNeg(PyObject* self, PyObject* args)
{
    float mu  = 0.0f;
    float sig = 0.0f;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "ff", &mu, &sig ))
    {
        return nullptr;
    }

    float retval = p_RNG->eGaussNonNeg(mu,sig);
    return Py_BuildValue("f", retval);
}

static PyObject* poisson(PyObject* self, PyObject* args)
{
    double rate = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "d", &rate ))
    {
        return nullptr;
    }

    uint64_t retval = p_RNG->Poisson(rate);
    return Py_BuildValue("K", retval);
}

static PyObject* exponential(PyObject* self, PyObject* args)
{
    double rate = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "d", &rate ))
    {
        return nullptr;
    }

    double retval = p_RNG->expdist(rate);
    return Py_BuildValue("d", retval);
}

static PyObject* gamma(PyObject* self, PyObject* args)
{
    float k     = 0.0f;
    float theta = 0.0f;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "ff", &k, &theta ))
    {
        return nullptr;
    }

    float retval = p_RNG->rand_gamma(k,theta);
    return Py_BuildValue("f", retval);
}

static PyObject* weibull(PyObject* self, PyObject* args)
{
    double lambda = 0.0;
    double kappa  = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "dd", &lambda, &kappa ))
    {
        return nullptr;
    }

    double retval = p_RNG->Weibull(lambda,kappa);
    return Py_BuildValue("d", retval);
}

static PyObject* weibull2(PyObject* self, PyObject* args)
{
    double lambda    = 0.0;
    double inv_kappa = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "dd", &lambda, &inv_kappa ))
    {
        return nullptr;
    }

    double retval = p_RNG->Weibull2(lambda,inv_kappa);
    return Py_BuildValue("d", retval);
}

static PyObject* loglogistic(PyObject* self, PyObject* args)
{
    double alpha = 0.0;
    double beta  = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "dd", &alpha, &beta ))
    {
        return nullptr;
    }

    double retval = p_RNG->LogLogistic(alpha,beta);
    return Py_BuildValue("d", retval);
}

static PyObject* binom(PyObject* self, PyObject* args)
{
    uint64_t n = 0;
    double   p = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "Kd", &n, &p ))
    {
        return nullptr;
    }

    uint64_t retval = p_RNG->binomial_true(n,p);
    return Py_BuildValue("K", retval);
}

static PyObject* binomApprox(PyObject* self, PyObject* args)
{
    uint64_t n = 0;
    double   p = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "Kd", &n, &p ))
    {
        return nullptr;
    }

    uint64_t retval = p_RNG->binomial_approx(n,p);
    return Py_BuildValue("K", retval);
}

static PyObject* binomApprox2(PyObject* self, PyObject* args)
{
    uint64_t n = 0;
    double   p = 0.0;
    pyRandomInit();

    if (!PyArg_ParseTuple(args, "Kd", &n, &p ))
    {
        return nullptr;
    }

    uint64_t retval = p_RNG->binomial_approx2(n,p);
    return Py_BuildValue("K", retval);
}

static PyObject* timeVaryingRateDist(PyObject* self, PyObject* args)
{
    PyObject*          arg_ptr  = nullptr;
    PyObject*          itr_ptr  = nullptr;
    PyObject*          val_ptr  = nullptr;
    std::vector<float> v_rate;
    float              timestep = 0.0f;
    float              rate     = 0.0f;
    pyRandomInit();

    if(!PyArg_ParseTuple(args, "Off", &arg_ptr, &timestep, &rate ))
    {
        return nullptr;
    }
    itr_ptr = PyObject_GetIter(arg_ptr);
    if(!itr_ptr)
    {
        return nullptr;
    }
    val_ptr = PyIter_Next(itr_ptr);
    while(val_ptr)
    {
        v_rate.push_back(static_cast<float>(PyFloat_AsDouble(val_ptr)));
        val_ptr = PyIter_Next(itr_ptr);
    }

    double retval = p_RNG->time_varying_rate_dist(v_rate,timestep,rate);
    return Py_BuildValue("d", retval);
}

 

static PyMethodDef DtkRANDOMMethods[] =
{
     {"random",            rand,                 METH_VARARGS,  "Floating point random number."},
     {"gauss",             gauss,                METH_VARARGS,  "Gaussian random number."},
     {"gauss_non_neg",     gaussNonNeg,          METH_VARARGS,  "Non-negative Gaussian random number."},
     {"poisson",           poisson,              METH_VARARGS,  "Poisson random number."},
     {"exponential",       exponential,          METH_VARARGS,  "Exponential random number."},
     {"gamma",             gamma,                METH_VARARGS,  "Gamma random number."},
     {"weibull",           weibull,              METH_VARARGS,  "Weibull random number."},
     {"weibull2",          weibull2,             METH_VARARGS,  "Weibull random number."},
     {"loglogistic",       loglogistic,          METH_VARARGS,  "Loglogistic random number."},
     {"binomial",          binom,                METH_VARARGS,  "Binomial random number."},
     {"binomial_approx",   binomApprox,          METH_VARARGS,  "Binomial random number."},
     {"binomial_approx2",  binomApprox2,         METH_VARARGS,  "Binomial random number."},
     {"tvrd",              timeVaryingRateDist,  METH_VARARGS,  "TBHIV Stuff."},
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
