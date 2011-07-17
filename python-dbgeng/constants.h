#ifndef _CONSTANTS_HPP
#define _CONSTANTS_HPP

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>

typedef enum {
    DEBUG_CREATE,
    DEBUG_CONNECT,
} E_PyDebugClientMethod;

void AddConstants(PyObject *m);

#endif

