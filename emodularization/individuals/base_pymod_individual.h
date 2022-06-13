#include <Python.h>
#include <iostream>
#include <map>

template< class intrahost_type >
static PyObject*
getIndividual(PyObject* self, PyObject* args)
{
    extern std::map< int, intrahost_type * > population;
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getIndividual (as int)." << std::endl;
        PyErr_SetString(PyExc_RuntimeError, "Failed to parse valid id in getIndividual.");
        return NULL;
    }
    return PyCapsule_New((void*)population[ id ], nullptr, nullptr);
}

template< class intrahost_type >
static PyObject*
shouldInfect(PyObject* self, PyObject* args)
{
    extern std::map< int, intrahost_type * > population;
    // Get id. 
    int id;
    float contagion;
    if( !PyArg_ParseTuple(args, "(if)", &id, &contagion ) )
    {
        std::cout << "Failed to parse id and/or contagion for shouldInfect." << std::endl;
        PyErr_SetString(PyExc_RuntimeError, "Failed to parse valid id in shouldInfect.");
        return NULL;
    }
    bool should_infect = population.at( id )->ShouldAcquire( contagion, 1, 1.0, TransmissionRoute::CONTACT );
    return Py_BuildValue("b", should_infect );
}
