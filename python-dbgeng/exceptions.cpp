#include "exceptions.h"

PyObject *
err_notimpl(void)
{
    PyErr_SetString(PyExc_NotImplementedError, "Go away");
    return NULL;
}

PyObject *
err_noclient(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected client");
    return NULL;
}

PyObject *
err_nocontrol(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected control");
    return NULL;
}

PyObject *
err_nobp(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected bp");
    return NULL;
}

PyObject *
err_noadv(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected adv");
    return NULL;
}

PyObject *
err_noregs(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected regs");
    return NULL;
}

PyObject *
err_nodata(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected data");
    return NULL;
}

PyObject *
err_nosyms(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected syms");
    return NULL;
}

PyObject *
err_nosys(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected sys");
    return NULL;
}

PyObject *
err_nosg(void)
{
    PyErr_SetString(PyExc_ValueError, "Operation on Non-connected symgrp");
    return NULL;
}

static char *
err_strmap(HRESULT err)
{
    switch (err)
    {
    case S_OK:
        return "Success";
    case S_FALSE:
        return "Partial Success.";
    case E_FAIL:
        return "The operation could not be performed.";
    case E_INVALIDARG:
        return "One of the arguments passed was invalid.";
    case E_NOINTERFACE:
        return "The object searched for was not found.";
    case E_OUTOFMEMORY:
        return "A memory allocation attempt failed.";
    case E_UNEXPECTED:
        return "The target was not accessible.";
    case E_NOTIMPL:
        return "Not implemented.";
    case E_PENDING:
        return "Pending.";
    default:
        return NULL;
    }
}

static PyObject *
err_objmap(HRESULT err)
{
    PyObject *m;
    PyObject *dict;

    m = PyImport_AddModule("pybag.pydbgeng");
    if (m == NULL || (dict = PyModule_GetDict(m)) == NULL)
        return NULL;

    switch (err)
    {
    case S_FALSE:
        return PyDict_GetItemString(dict, "S_FALSE_Error");
    case E_FAIL:
        return PyDict_GetItemString(dict, "E_FAIL_Error");
    case E_INVALIDARG:
        return PyDict_GetItemString(dict, "E_INVALIDARG_Error");
    case E_NOINTERFACE:
        return PyDict_GetItemString(dict, "E_NOINTERFACE_Error");
    case E_OUTOFMEMORY:
        return PyDict_GetItemString(dict, "E_OUTOFMEMORY_Error");
    case E_UNEXPECTED:
        return PyDict_GetItemString(dict, "E_UNEXPECTED_Error");
    case E_NOTIMPL:
        return PyDict_GetItemString(dict, "E_NOTIMPL_Error");
    case E_PENDING:
        return PyDict_GetItemString(dict, "E_PENDING_Error");
    default:
        return NULL;
    }
}

PyObject *
err_dbgeng(HRESULT code)
{
    PyObject *exc;
    char *msg;

    msg = err_strmap(code);
    exc = err_objmap(code);
    if (exc)
        return PyErr_Format(exc, "%s\n", msg);
    else
        return PyErr_SetFromWindowsErr(code);
}

PyObject *
err_false(char *msg)
{
    return PyErr_Format(err_objmap(S_FALSE), "%s\n", msg);
}

PyObject *
err_fail(char *msg)
{
    return PyErr_Format(err_objmap(E_FAIL), "%s\n", msg);
}

PyObject *
err_invalidarg(char *msg)
{
    return PyErr_Format(err_objmap(E_INVALIDARG), "%s\n", msg);
}

PyObject *
err_nointerface(char *msg)
{
    return PyErr_Format(err_objmap(E_NOINTERFACE), "%s\n", msg);
}

PyObject *
err_pending(char *msg)
{
    return PyErr_Format(err_objmap(E_PENDING), "%s\n", msg);
}

PyObject *
err_unexpected(char *msg)
{
    return PyErr_Format(err_objmap(E_UNEXPECTED), "%s\n", msg);
}

void print_dict(PyObject *d)
{
    PyObject *k;
    Py_ssize_t size, i =0;


    k = PyDict_Keys(d);

    size = PyList_Size(k);

    printf("Size : %d\n", size);
    for (i = 0; i < size; i++) {
        PyObject *item = PyList_GET_ITEM(k, i);
        printf("-- %s -- \n", PyString_AsString(item));
    }


}
PyObject *
exc_timeout(char *msg)
{
    PyObject *m;
    PyObject *dict;
    PyObject *exc;
    m = PyImport_AddModule("pybag.pydbgeng");
    if (m == NULL) 
        return NULL;
    dict = PyModule_GetDict(m);
    if (dict == NULL)
        return NULL;

    exc = PyDict_GetItemString(dict, "DbgEngTimeout");
    if (exc == NULL)
        return PyErr_Format(PyExc_SystemError, "Error with internal exceptions");

    if (msg == NULL)
        msg = "";
    return PyErr_Format(exc, "%s\n", msg);
}

void AddExceptions(PyObject *m)
{
    PyObject *dict;
    PyObject *DbgEngErr = NULL;
    PyObject *DbgEngTimeout = NULL;
    PyObject *exc;

    if ((dict = PyModule_GetDict(m)) == NULL)
        return;

    DbgEngTimeout = PyErr_NewException("pydbgeng.DbgEngTimeout", NULL, NULL);
    PyDict_SetItemString(dict, "DbgEngTimeout", DbgEngTimeout);
    Py_XDECREF(DbgEngTimeout);

    DbgEngErr = PyErr_NewException("pydbgeng.DbgEngError", PyExc_WindowsError, NULL);
    PyDict_SetItemString(dict, "DbgEngError", DbgEngErr);

    exc = PyErr_NewException("pydbgeng.S_FALSE_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "S_FALSE_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_FAIL_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_FAIL_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_INVALIDARG_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_INVALIDARG_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_NOINTERFACE_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_NOINTERFACE_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_OUTOFMEMORY_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_OUTOFMEMORY_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_UNEXPECTED_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_UNEXPECTED_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_NOTIMPL_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_NOTIMPL_Error", exc);
    Py_XDECREF(exc);

    exc = PyErr_NewException("pydbgeng.E_PENDING_Error", DbgEngErr, NULL);
    PyDict_SetItemString(dict, "E_PENDING_Error", exc);
    Py_XDECREF(exc);

    Py_XDECREF(DbgEngErr);
}
