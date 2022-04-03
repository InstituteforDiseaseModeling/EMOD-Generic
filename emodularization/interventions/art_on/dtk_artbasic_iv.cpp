#include <Python.h>
#include <iostream>
#include "ARTBasic.h"
#include "IndividualHIV.h"

using namespace Kernel;

// OK, we're using PyCapsules to return an opaque/void* pointer to the python
// layer that can get passed to another extension module.

// This is from tutorial, but I unhooked it because I don't yet understand
// exactly how to use it.
void destroy_intervention(PyObject* funkster) {
    //std::cout << "VACCINE GOT DESTROYED I THINK." << std::endl << std::endl;
    delete (ARTBasic*)PyCapsule_GetPointer(funkster, "val");
}

// This just creates an object. it can be returned to py layer and passed to another
// module. I think. I don't really understand what val is yet. Apparently it's the name
// of the capsule and can used in an import statement.
// "val" is from the tutorial. Not really sure about what that does if anything for us.
static PyObject*
getIntervention(PyObject *self, PyObject *args) {
    auto * new_iv = new ARTBasic();
    Kernel::JsonConfigurable::_useDefaults = true;
    auto config_json = Configuration::Load("artbasic.json");
    Kernel::JsonConfigurable::_useDefaults = false;
    new_iv->Configure( config_json );
    IDistributableIntervention * return_iv = new_iv;
    return PyCapsule_New((void*)return_iv, nullptr, nullptr);
}

static PyObject*
distribute(PyObject *self, PyObject *args) {
    PyObject* pf = NULL;
    if( !PyArg_ParseTuple(args, "O", &pf ) )
    {
        std::cout << "Failed to parse id and/or pointer for distribute." << std::endl;
    }
    auto * man = (IndividualHumanHIV *)PyCapsule_GetPointer(pf, nullptr );
    
    auto * new_iv = new ARTBasic();
    Kernel::JsonConfigurable::_useDefaults = true;
    auto iv_config_json = Configuration::Load("artbasic.json");
    Kernel::JsonConfigurable::_useDefaults = false;
    new_iv->Configure( iv_config_json );
    assert( man->GetInterventionsContext() );
    new_iv->Distribute( man->GetInterventionsContext(), nullptr );
    Py_RETURN_NONE;
}


// This (Simple) iv module is going to expose two functions.
// One gets the ARTBasic in the first place. The second lets a 
// different module call functions on it in python. I think.
// PyMod contract code below
static PyMethodDef DtkArtBasicIvIndividualMethods[] =
{
     {"get_intervention", getIntervention, METH_VARARGS, "Try to return an opaque void pointer that can be invoked from python."},
     {"distribute", distribute, METH_VARARGS, "Tell intervention to distribute itself to an individual."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_artbasic_iv",
    nullptr,
    0,
    DtkArtBasicIvIndividualMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_artbasic_iv(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
