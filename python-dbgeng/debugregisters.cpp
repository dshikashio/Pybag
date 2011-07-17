#include "pydbgeng.h"

static PyObject *
dbgregs_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugRegistersObject *self;

    self = (PyDebugRegistersObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->regs = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgregs_dealloc(PyDebugRegistersObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->regs != NULL) {
        self->regs->Release();
        self->regs = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgregs_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugRegistersObject *self = (PyDebugRegistersObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugRegisters2 *debugRegisters = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->regs) {
        self->regs->Release();
        self->regs = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugRegisters",
                kwlist, &PyDebugClientType, &client))
        return -1;
    if (client->client == NULL) {
        err_noclient();
        return -1;
    }
    if ((hr = client->client->QueryInterface(__uuidof(IDebugRegisters2),
                    (void **)&debugRegisters)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->regs = debugRegisters;
    return 0;
}

static PyObject *
dbgregs_GetDescription(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Register;
    char NameBuffer[512] = {0};
    DEBUG_REGISTER_DESCRIPTION Desc;
    PyDebugRegisterDescriptionObject *ret = NULL;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I:GetDescription", &Register))
        return NULL;
    if ((hr = self->regs->GetDescription(Register, NameBuffer, 
                    512, NULL, &Desc)) != S_OK) 
    {
        if (hr == E_UNEXPECTED)
            return err_unexpected("No target machine has been specified, "
                    "or a description of the register could not be found.");
        else if (hr == E_INVALIDARG)
            return err_invalidarg("The index of the register requested is "
                    "greater than the total number of registers on the "
                    "target's machine.");
        else
            return err_dbgeng(hr);
    }
    ret = (PyDebugRegisterDescriptionObject *)PyObject_CallObject(
            (PyObject *)&PyDebugRegisterDescriptionType, NULL);
    if (ret == NULL)
        Py_RETURN_NONE;
    ret->reg = Desc;
    return Py_BuildValue("(sN)", NameBuffer, ret);
}

static PyObject *
dbgregs_GetIndexByName(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Name;
    ULONG Index;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "s:GetIndexByName", &Name))
        return NULL;
    if ((hr = self->regs->GetIndexByName(Name, &Index)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The register was not found.");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("I", Index);
}

static PyObject *
dbgregs_GetFrameOffset(PyDebugRegistersObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if ((hr = self->regs->GetFrameOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_GetFrameOffset2(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Source;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I:GetFrameOffset2", &Source))
        return NULL;
    if ((hr = self->regs->GetFrameOffset2(Source, &Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_GetInstructionOffset(PyDebugRegistersObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if ((hr = self->regs->GetInstructionOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_GetInstructionOffset2(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Source;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I:GetInstructionOffset2", &Source))
        return NULL;
    if ((hr = self->regs->GetInstructionOffset2(Source, &Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_GetStackOffset(PyDebugRegistersObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if ((hr = self->regs->GetStackOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_GetStackOffset2(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Source;
    ULONG64 Offset;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I:GetStackOffset2", &Source))
        return NULL;
    if ((hr = self->regs->GetStackOffset2(Source, &Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgregs_OutputRegisters(PyDebugRegistersObject *self)
{
    HRESULT hr;
    ULONG OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    ULONG Flags = DEBUG_REGISTERS_DEFAULT;

    // XXX - Flags should probably be a non-required arg
    if (self->regs == NULL)
        return err_noregs();
    if ((hr = self->regs->OutputRegisters(OutputControl, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgregs_OutputRegisters2(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Source;
    ULONG OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    ULONG Flags = DEBUG_REGISTERS_DEFAULT;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I|I:OutputRegisters2", &Source, &Flags))
        return NULL;
    if ((hr = self->regs->OutputRegisters2(OutputControl, Source, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgregs_GetNumberRegisters(PyDebugRegistersObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->regs == NULL)
        return err_noregs();
    if ((hr = self->regs->GetNumberRegisters(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgregs_GetValue(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Register;
    DEBUG_VALUE Value;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "I:GetValue", &Register))
        return NULL;
    if ((hr = self->regs->GetValue(Register, &Value)) != S_OK) {
        if (hr == E_UNEXPECTED)
            return err_unexpected("The target is not accessible, "
                "or the register could not be accessed.");
        else if (hr == E_INVALIDARG)
            return err_invalidarg("The value of Register is greater "
                "than the number of registers on the target machine.");
        else
            return err_dbgeng(hr);
    }

    switch (Value.Type)
    {
    case DEBUG_VALUE_INT8:
        return Py_BuildValue("B", Value.I8);
    case DEBUG_VALUE_INT16:
        return Py_BuildValue("H", Value.I16);
    case DEBUG_VALUE_INT32:
        return Py_BuildValue("I", Value.I32);
    case DEBUG_VALUE_INT64:
        return Py_BuildValue("K", Value.I64);
    case DEBUG_VALUE_FLOAT32:
        return Py_BuildValue("f", Value.F32);
    case DEBUG_VALUE_FLOAT64:
        return Py_BuildValue("d", Value.F64);
    case DEBUG_VALUE_FLOAT80:
        return Py_BuildValue("z#", Value.F80Bytes, 10);
    case DEBUG_VALUE_FLOAT128:
        return Py_BuildValue("z#", Value.F80Bytes, 16);
    default:
        PyErr_SetString(PyExc_TypeError, "Non-supported Type");
        return NULL;
    }
}

static PyObject *
dbgregs_SetValue(PyDebugRegistersObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Register;
    ULONG64 InVal;
    DEBUG_VALUE Value;

    if (self->regs == NULL)
        return err_noregs();
    if (!PyArg_ParseTuple(args, "IK:SetValue", &Register, &InVal))
        return NULL;
    Value.Type = DEBUG_VALUE_INT64;
    Value.I64 = InVal;
    if ((hr = self->regs->SetValue(Register, &Value)) != S_OK) {
        if (hr == E_UNEXPECTED)
            return err_unexpected("The target is not accessible, "
                "or the register could not be accessed.");
        else if (hr == E_INVALIDARG)
            return err_invalidarg("The value of Register is greater "
                "than the number of registers on the target machine.");
        else
            return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugRegisters_methods[] = {
    {"GetDescription", 
     (PyCFunction)dbgregs_GetDescription, METH_VARARGS, 
     "GetDescription(Register) -> (name, DEBUG_REGISTER_DESCRIPTION)\n"
     "  Returns the description of a register."
    },
    {"GetIndexByName", 
     (PyCFunction)dbgregs_GetIndexByName, METH_VARARGS, 
     "GetIndexByName(Name) -> index\n"
     "  Returns the index of the named register."
    },
    {"GetFrameOffset", 
     (PyCFunction)dbgregs_GetFrameOffset, METH_NOARGS, 
     "GetFrameOffset() -> offset\n"
     "  Returns the location of the stack frame for the current function."
    },
    {"GetFrameOffset2", 
     (PyCFunction)dbgregs_GetFrameOffset2, METH_VARARGS, 
     "GetFrameOffset2(Source) -> offset\n"
     "  Returns the location of the stack frame for the current function."
     "  Source:\n"
     "    DEBUG_REGSRC_DEBUGGEE\n"
     "    DEBUG_REGSRC_EXPLICIT\n"
     "    DEBUG_REGSRC_FRAME"
    },
    {"GetInstructionOffset", 
     (PyCFunction)dbgregs_GetInstructionOffset, METH_NOARGS, 
     "GetInstructionOffset() -> offset\n"
     "  Returns the location of the current thread's current instruction."
    },
    {"GetInstructionOffset2", 
     (PyCFunction)dbgregs_GetInstructionOffset2, METH_VARARGS, 
     "GetInstructionOffset2(Source) -> offset\n"
     "  Returns the location of the current thread's current instruction.\n"
     "  Source:\n"
     "    DEBUG_REGSRC_DEBUGGEE\n"
     "    DEBUG_REGSRC_EXPLICIT\n"
     "    DEBUG_REGSRC_FRAME"
    },
    {"GetStackOffset", 
     (PyCFunction)dbgregs_GetStackOffset, METH_NOARGS, 
     "GetStackOffset() -> offset\n"
     "  Returns the current thread's current stack location."
    },
    {"GetStackOffset2", 
     (PyCFunction)dbgregs_GetStackOffset2, METH_VARARGS, 
     "GetStackOffset2(Source) -> offset\n"
     "  Returns the current thread's current stack location.\n"
     "  Source:\n"
     "    DEBUG_REGSRC_DEBUGGEE\n"
     "    DEBUG_REGSRC_EXPLICIT\n"
     "    DEBUG_REGSRC_FRAME"
    },
// GetPseudoDescription
// GetPseudoIndexByName
// GetNumberPseudoRegisters
// GetPseudoValues
// SetPseudoValues
    {"OutputRegisters", 
     (PyCFunction)dbgregs_OutputRegisters, METH_NOARGS, 
     "OutputRegisters()\n"
     "  Formats and sends the target's registers to the clients as output."
    },
    {"OutputRegisters2", 
     (PyCFunction)dbgregs_OutputRegisters2, METH_VARARGS, 
     "OutputRegisters2(Source, Flags=DEBUG_REGISTERS_DEFAULT)\n"
     "  Formats and sends the target's registers to the clients as output.\n"
     "  Source:\n"
     "    DEBUG_REGSRC_DEBUGGEE\n"
     "    DEBUG_REGSRC_EXPLICIT\n"
     "    DEBUG_REGSRC_FRAME\n"
     "  Flags:\n"
     "    DEBUG_REGISTERS_DEFAULT\n"
     "    DEBUG_REGISTERS_ALL\n"
     "    DEBUG_REGISTERS_INT32\n"
     "    DEBUG_REGISTERS_INT64\n"
     "    DEBUG_REGISTERS_FLOAT"
    },
    {"GetNumberRegisters", 
     (PyCFunction)dbgregs_GetNumberRegisters, METH_NOARGS, 
     "GetNumberRegisters() -> number\n"
     "  Returns the number of registers on the target computer."
    },
    {"GetValue", 
     (PyCFunction)dbgregs_GetValue, METH_VARARGS, 
     "GetValue(Register) -> value\n"
     "  Gets the value of one of the target's registers."
    },
// GetValues
// GetValues2
    {"SetValue", 
     (PyCFunction)dbgregs_SetValue, METH_VARARGS, 
     "SetValue(Register, Value)\n"
     "  Sets the value of one of the target's registers."
    },
// SetValues
// SetValues2
    {NULL, NULL}
};


PyTypeObject PyDebugRegistersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugRegisters", /*tp_name*/
    sizeof(PyDebugRegistersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgregs_dealloc, /*tp_dealloc*/
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
    "DebugRegisters objects",  /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugRegistersObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugRegisters_methods,  /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgregs_init,              /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgregs_new,               /* tp_new */
    PyObject_Del,              /* tp_free */
};

