#include "outputcallbacks.h"

PyDebugOutputCallbacks::PyDebugOutputCallbacks()
{
    count = 0;
    OutputCallback = NULL;
}

PyDebugOutputCallbacks::~PyDebugOutputCallbacks()
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    Py_XDECREF(OutputCallback);
    PyGILState_Release(gstate);
}

void PyDebugOutputCallbacks::SetCallback(PyObject *cb)
{
    Py_XINCREF(cb);
    OutputCallback = cb;
}

HRESULT STDMETHODCALLTYPE 
PyDebugOutputCallbacks::Output(ULONG Mask, PCSTR Text)
{
    PyGILState_STATE gstate;
    PyObject *args = NULL;
    PyObject *result = NULL;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("(s)", Text);
    result = PyObject_CallObject(OutputCallback, args);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
    }

    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return S_OK;
}

