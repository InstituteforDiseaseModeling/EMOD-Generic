#include <Python.h>
#include <iostream>
#include "Vaccine.h"
#include "Individual.h"

using namespace Kernel;

// OK, we're using PyCapsules to return an opaque/void* pointer to the python
// layer that can get passed to another extension module.

void destroy_intervention(PyObject* intervention) {
    delete (Vaccine*)PyCapsule_GetPointer( intervention, "val" );
}

// This just creates an object. it can be returned to py layer and passed to another
// module. I think. I don't really understand what val is yet. Apparently it's the name
// of the capsule and can used in an import statement.
// "val" is from the tutorial. Not really sure about what that does if anything for us.
static PyObject*
getIntervention(PyObject *self, PyObject *args) {
    auto new_vaccine = new Vaccine();
    auto vaccine_config_json = Configuration::Load( "sv.json" );
    new_vaccine->Configure( vaccine_config_json );
    // TBD: I haven't really investigated or thought about deallocation yet.
    return PyCapsule_New( (void*) new_vaccine, nullptr, nullptr );
}

static PyObject*
distribute(PyObject *self, PyObject *args) {
    PyObject* pf = NULL;
    if( !PyArg_ParseTuple(args, "O", &pf ) )
    {
        std::cout << "Failed to parse id and/or pointer for distribute." << std::endl;
    }
    auto * man = (IndividualHuman *)PyCapsule_GetPointer(pf, nullptr );
    
    auto * new_iv = new Vaccine();
    Kernel::JsonConfigurable::_useDefaults = true;
    auto iv_config_json = Configuration::Load("sv.json");
    Kernel::JsonConfigurable::_useDefaults = false;
    new_iv->Configure( iv_config_json );
    assert( man->GetInterventionsContext() );
    new_iv->Distribute( man->GetInterventionsContext(), nullptr );
    Py_RETURN_NONE;
}

// This (Simple) vaccine module is going to expose two functions.
// One gets the vaccine in the first place. The second lets a 
// different module call functions on it in python. I think.
// PyMod contract code below
static PyMethodDef DtkvaccineIvIndividualMethods[] =
{
     {"get_intervention", getIntervention, METH_VARARGS, "Try to return an opaque void pointer that can be invoked from python."},
     {"distribute", distribute, METH_VARARGS, "Tell intervention to distribute itself to an individual."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_vaccine_intervention",
    nullptr,
    0,
    DtkvaccineIvIndividualMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_vaccine_intervention(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}

