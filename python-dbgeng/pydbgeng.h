#ifndef _PYDBGENG_H
#define _PYDBGENG_H

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>
#include <structmember.h>
#include "constants.h"
#include "exceptions.h"
#include "eventcallbacks.h"
#include "outputcallbacks.h"
#include "winstructs.h"

typedef struct {
    PyObject_HEAD
    IDebugClient7 *client;
    PyObject *weakreflist;
} PyDebugClientObject;

typedef struct {
    PyObject_HEAD
    IDebugControl4 *control;
    PyObject *weakreflist;
} PyDebugControlObject;

typedef struct {
    PyObject_HEAD
    IDebugBreakpoint *bp;
    PyObject *weakreflist;
} PyDebugBreakpointObject;

typedef struct {
    PyObject_HEAD
    IDebugAdvanced2 *adv;
    PyObject *weakreflist;
} PyDebugAdvancedObject;

typedef struct {
    PyObject_HEAD
    IDebugRegisters2 *regs;
    PyObject *weakreflist;
} PyDebugRegistersObject;

typedef struct {
    PyObject_HEAD
    IDebugDataSpaces4 *data;
    PyObject *weakreflist;
} PyDebugDataSpacesObject;

typedef struct {
    PyObject_HEAD
    IDebugSymbols3 *syms;
    PyObject *weakreflist;
} PyDebugSymbolsObject;

typedef struct {
    PyObject_HEAD
    IDebugSystemObjects4 *sys;
    PyObject *weakreflist;
} PyDebugSystemsObjectsObject;

typedef struct {
    PyObject_HEAD
    IDebugSymbolGroup2 *sg;
    PyObject *weakreflist;
} PyDebugSymbolGroupObject;

extern PyTypeObject PyDebugControlType;
extern PyTypeObject PyDebugClientType;
extern PyTypeObject PyDebugBreakpointType;
extern PyTypeObject PyDebugAdvancedType;
extern PyTypeObject PyDebugRegistersType;
extern PyTypeObject PyDebugDataSpacesType;
extern PyTypeObject PyDebugSymbolsType;
extern PyTypeObject PyDebugSystemsObjectsType;
extern PyTypeObject PyDebugSymbolGroupType;

#endif

