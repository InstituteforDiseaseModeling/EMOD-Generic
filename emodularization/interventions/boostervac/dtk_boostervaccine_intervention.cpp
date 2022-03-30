#include <Python.h>
#include <iostream>
#include "SimpleBoosterVaccine.h"

using namespace Kernel;

// OK, we're using PyCapsules to return an opaque/void* pointer to the python
// layer that can get passed to another extension module.

// This is from tutorial, but I unhooked it because I don't yet understand
// exactly how to use it.
void destroy_intervention(PyObject* funkster) {
    //std::cout << "BoosterVaccine GOT DESTROYED I THINK." << std::endl << std::endl;
    delete (SimpleBoosterVaccine*)PyCapsule_GetPointer(funkster, "val");
}

// This just creates an object. it can be returned to py layer and passed to another
// module. I think. I don't really understand what val is yet. Apparently it's the name
// of the capsule and can used in an import statement.
// "val" is from the tutorial. Not really sure about what that does if anything for us.
static PyObject*
getIntervention(PyObject *self, PyObject *args) {
    auto new_BoosterVaccine = new SimpleBoosterVaccine();
    auto BoosterVaccine_config_json = Configuration::Load("sv.json");
    new_BoosterVaccine->Configure( BoosterVaccine_config_json );
    new_BoosterVaccine->ApplyVaccineTake( nullptr );
    // TBD: I haven't really investigated or thought about deallocation yet.
    return PyCapsule_New((void*)new_BoosterVaccine, nullptr, nullptr);
}

// This (Simple) BoosterVaccine module is going to expose two functions.
// One gets the BoosterVaccine in the first place. The second lets a 
// different module call functions on it in python. I think.
// PyMod contract code below
static PyMethodDef DtkBoosterVaccineIvIndividualMethods[] =
{
     {"get_intervention", getIntervention, METH_VARARGS, "Try to return an opaque void pointer that can be invoked from python."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_boostervaccine_intervention",
    nullptr,
    0,
    DtkBoosterVaccineIvIndividualMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_boostervaccine_intervention(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}

