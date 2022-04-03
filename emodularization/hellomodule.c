#include <Python.h>

static PyObject*
say_hello(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);

    Py_RETURN_NONE;
}

static PyObject*
say_bye(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Bye-bye %s!\n", name);

    Py_RETURN_NONE;
}

static PyMethodDef HelloMethods[] =
{
     {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},
     {"say_bye", say_bye, METH_VARARGS, "Un-greet somebody."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
inithello(void)
{
     (void) Py_InitModule("hello", HelloMethods);
}
