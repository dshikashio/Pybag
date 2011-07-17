#ifndef _EXCEPTIONS_HPP
#define _EXCEPTIONS_HPP

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>

PyObject *err_notimpl(void);
PyObject *err_noclient(void);
PyObject *err_nocontrol(void);
PyObject *err_nobp(void);
PyObject *err_noadv(void);
PyObject *err_noregs(void);
PyObject *err_nodata(void);
PyObject *err_nosyms(void);
PyObject *err_nosys(void);
PyObject *err_nosg(void);
PyObject *err_dbgeng(HRESULT code);
PyObject *err_false(char *msg);
PyObject *err_fail(char *msg);
PyObject *err_invalidarg(char *msg);
PyObject *err_nointerface(char *msg);
PyObject *err_pending(char *msg);
PyObject *err_unexpected(char *msg);
PyObject *exc_timeout(char *msg);

void AddExceptions(PyObject *m);

#endif

