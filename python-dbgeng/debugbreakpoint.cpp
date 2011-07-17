#include "pydbgeng.h"

static PyObject *
dbgbp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugBreakpointObject *self;

    self = (PyDebugBreakpointObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->bp = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgbp_dealloc(PyDebugBreakpointObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);
    if (self->bp)
        self->bp = NULL;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgbp_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugBreakpointObject *self = (PyDebugBreakpointObject *)oself;
    if (self->bp)
        self->bp = NULL;
    return 0;
}

static PyObject *
dbgbp_GetType(PyDebugBreakpointObject *self)
{
    ULONG BreakType;
    ULONG ProcType;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetType(&BreakType, &ProcType)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("II", BreakType, ProcType);
}

static PyObject *
dbgbp_GetId(PyDebugBreakpointObject *self)
{
    ULONG Id;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetId(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgbp_AddFlags(PyDebugBreakpointObject *self, PyObject *args)
{
    ULONG Flags;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "I:AddFlags", &Flags))
        return NULL;
    if ((hr = self->bp->AddFlags(Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_RemoveFlags(PyDebugBreakpointObject *self, PyObject *args)
{
    ULONG Flags;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "I:RemoveFlags", &Flags))
        return NULL;
    if ((hr = self->bp->RemoveFlags(Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetFlags(PyDebugBreakpointObject *self)
{
    ULONG Flags;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetFlags(&Flags)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Flags);
}

static PyObject *
dbgbp_SetFlags(PyDebugBreakpointObject *self, PyObject *args)
{
    ULONG Flags;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "I:SetFlags", &Flags))
        return NULL;
    if ((hr = self->bp->SetFlags(Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetOffset(PyDebugBreakpointObject *self)
{
    ULONG64 Offset;
    HRESULT hr;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetOffset(&Offset)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The breakpoint is deferred and does "
                "not currently specify a location in the target's memory "
                "address space.");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgbp_SetOffset(PyDebugBreakpointObject *self, PyObject *args)
{
    ULONG64 Offset;
    HRESULT hr;
    PyObject *retval;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "K:SetOffset", &Offset))
        return NULL;
    hr = self->bp->SetOffset(Offset);
    if (hr == S_OK)
        retval = Py_True;
    else if (hr == E_UNEXPECTED)
        retval = Py_False;
    else
        return err_dbgeng(hr);

    return Py_BuildValue("O", retval);
}

static PyObject *
dbgbp_GetOffsetExpression(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG ExpressionSize;
    PyObject *ret = NULL;

    if (self->bp == NULL)
        return err_nobp();

    hr = self->bp->GetOffsetExpression(NULL, 0, &ExpressionSize);
    if (hr != S_OK && hr != S_FALSE)
            return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(ExpressionSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->bp->GetOffsetExpression(Buffer, 
                    ExpressionSize, NULL)) != S_OK)
    {
        PyMem_Free(Buffer);
        return err_dbgeng(hr);
    }

    ret = Py_BuildValue("s", Buffer);
    PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgbp_SetOffsetExpression(PyDebugBreakpointObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Expression;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "s:SetOffsetExpression", &Expression))
        return NULL;
    if ((hr = self->bp->SetOffsetExpression(Expression)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetParameters(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    DEBUG_BREAKPOINT_PARAMETERS bbp;
    PyDebugBreakPointParametersObject *obj;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetParameters(&bbp)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugBreakPointParametersObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakPointParametersType, NULL);
    obj->bpp = bbp;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgbp_GetCommand(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG CommandSize;
    PyObject *ret = NULL;

    if (self->bp == NULL)
        return err_nobp();
    hr = self->bp->GetCommand(NULL, 0, &CommandSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    Buffer = (PSTR)PyMem_Malloc(CommandSize * sizeof(*Buffer));
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->bp->GetCommand(Buffer, CommandSize, NULL)) != S_OK) {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgbp_SetCommand(PyDebugBreakpointObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Command;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "s:SetCommand", &Command))
        return NULL;
    if ((hr = self->bp->SetCommand(Command)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetDataParameters(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    ULONG Size;
    ULONG AccessType;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetDataParameters(&Size, &AccessType)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The breakpoint is not a processor breakpoint");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("II", Size, AccessType);
}

static PyObject *
dbgbp_SetDataParameters(PyDebugBreakpointObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Size;
    ULONG AccessType;
    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "II:SetDataParameters", &Size, &AccessType))
        return NULL;
    if ((hr = self->bp->SetDataParameters(Size, AccessType)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The breakpoint is not a processor breakpoint");
        else
            return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetPassCount(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    ULONG Count;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetPassCount(&Count)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Count);
}

static PyObject *
dbgbp_GetCurrentPassCount(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    ULONG Count;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetCurrentPassCount(&Count)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Count);
}

static PyObject *
dbgbp_SetPassCount(PyDebugBreakpointObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Count;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "I:SetPassCount", &Count))
        return NULL;
    if ((hr = self->bp->SetPassCount(Count)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgbp_GetMatchThreadId(PyDebugBreakpointObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->bp == NULL)
        return err_nobp();
    if ((hr = self->bp->GetMatchThreadId(&Id)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("No specific thread has been set for this "
                "breakpoint. Any thread can trigger the breakpoint.");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgbp_SetMatchThreadId(PyDebugBreakpointObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Id;

    if (self->bp == NULL)
        return err_nobp();
    if (!PyArg_ParseTuple(args, "I:SetMatchThreadId", &Id))
        return NULL;
    if ((hr = self->bp->SetPassCount(Id)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The thread that Thread specifies could "
                "not be found.");
        else if (hr == E_INVALIDARG)
            return err_invalidarg("The target is in a kernel and the "
                "breakpoint is a processor breakpoint. Processor breakpoints "
                "cannot be limited to threads in kernel mode.");
        else
            return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugBreakpoint_methods[] = {
    {"GetCommand", 
     (PyCFunction)dbgbp_GetCommand, METH_NOARGS, 
     "GetCommand() -> cmd\n" 
     "  return the command string executed on break"
    },
    {"SetCommand", 
     (PyCFunction)dbgbp_SetCommand, METH_VARARGS, 
     "SetCommand(cmd)\n"
     "  set the command string executed on break"
    },
    {"AddFlags", 
     (PyCFunction)dbgbp_AddFlags, METH_VARARGS, 
     "AddFlags(flags)\n"
     "  add flags to breakpoint.\n"
     "  Flags:\n"
     "    DEBUG_BREAKPOINT_ENABLED\n"
     "    DEBUG_BREAKPOINT_ADDER_ONLY\n"
     "    DEBUG_BREAKPOINT_GO_ONLY\n"
     "    DEBUG_BREAKPOINT_ONE_SHOT\n"
     "    DEBUG_BREAKPOINT_DEFERRED"
    },
    {"GetFlags", 
     (PyCFunction)dbgbp_GetFlags, METH_NOARGS, 
     "GetFlags() -> flags\n"
     "  return the breakpoint flags"
    },
    {"RemoveFlags", 
     (PyCFunction)dbgbp_RemoveFlags, METH_VARARGS, 
     "RemoveFlags(flags)\n"
     "  removes flags from breakpoint"
    },
    {"SetFlags", 
     (PyCFunction)dbgbp_SetFlags, METH_VARARGS, 
     "SetFlags(flags)\n"
     "  sets the breakpoint flags"
    },
    {"GetId", 
     (PyCFunction)dbgbp_GetId, METH_NOARGS, 
     "GetId() -> id\n"
     "  returns the breakpoint ID"
    },
    {"GetOffset", 
     (PyCFunction)dbgbp_GetOffset, METH_NOARGS, 
     "GetOffset() -> offset\n"
     "  returns the breakpoint offset"
    },
    {"SetOffset", 
     (PyCFunction)dbgbp_SetOffset, METH_VARARGS, 
     "SetOffset(Offset)\n"
     "  sets the location that triggers the breakpoint"
    },
    {"GetOffsetExpression", 
     (PyCFunction)dbgbp_GetOffsetExpression, METH_NOARGS, 
     "GetOffsetExpression() -> expression\n"
     "  gets the expression string that evaluates to the location that triggers\n" 
     "  the breakpoint"
    },
    {"SetOffsetExpression", 
     (PyCFunction)dbgbp_SetOffsetExpression, METH_VARARGS, 
     "SetOffsetExpression(expr)\n"
     "  sets the expression that evalutes to the location that triggers\n"
     "  the breakpoint"
    },
    {"GetParameters", 
     (PyCFunction)dbgbp_GetParameters, METH_NOARGS, 
     "GetParameters() -> DEBUG_BREAKPOINT_PARAMETERS\n"
     "  returns the breakpoint parameters"
    },
    {"GetDataParameters", 
     (PyCFunction)dbgbp_GetDataParameters, METH_NOARGS, 
     "GetDataParameters() -> (size, access_type)\n"
     "  returns the hardware breakpoint parameters"
    },
    {"SetDataParameters", 
     (PyCFunction)dbgbp_SetDataParameters, METH_VARARGS, 
     "SetDataParameters(size, access_type)\n"
     "  sets the hardware breakpoint parameters\n"
     "  access types:\n"
     "    DEBUG_BREAK_READ\n"
     "    DEBUG_BREAK_WRITE\n"
     "    DEBUG_BREAK_EXECUTE\n"
     "    DEBUG_BREAK_IO"
    },
    {"GetPassCount", 
     (PyCFunction)dbgbp_GetPassCount, METH_NOARGS, 
     "GetPassCount() -> count\n"
     "  returns number of times the target was originally required to\n"
     "  reach the breakpoint location before the breakpoint is triggered"
    },
    {"SetPassCount", 
     (PyCFunction)dbgbp_SetPassCount, METH_VARARGS, 
     "SetPassCount(count)\n"
     "  sets the number of times that the target must reach the\n"
     "  breakpoint location before the breakpoint is triggered"
    },
    {"GetCurrentPassCount", 
     (PyCFunction)dbgbp_GetCurrentPassCount, METH_NOARGS, 
     "GetCurrentPassCount() -> count\n"
     "  returns the remaining number of times that the target must reach\n" 
     "  the breakpoint location before the breakpoint is triggered"
    },
    {"GetMatchThreadId", 
     (PyCFunction)dbgbp_GetMatchThreadId, METH_NOARGS, 
     "GetMatchThreadId() -> ID\n"
     "  returns the engine ID of the thread that can trigger the breakpoint"
    },
    {"SetMatchThreadId", 
     (PyCFunction)dbgbp_SetMatchThreadId, METH_VARARGS, 
     "SetMatchThreadId(tid)\n" 
     "  sets the engine ID of the thread that can trigger the breakpoint"
    },
    {"GetType", 
     (PyCFunction)dbgbp_GetType, METH_NOARGS, 
     "GetType() -> type\n"
     "  returns the breakpoint type and the processor type"
    },
    {NULL, NULL}
};

PyTypeObject PyDebugBreakpointType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugBreakpoint",/*tp_name*/
    sizeof(PyDebugBreakpointObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgbp_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "DebugBreakpoint objects", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugBreakpointObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugBreakpoint_methods, /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgbp_init,                /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgbp_new,                 /* tp_new */
    PyObject_Del,              /* tp_free */
};
