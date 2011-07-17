#include "pydbgeng.h"

static PyObject *
dbgsg_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugSymbolGroupObject *self;

    self = (PyDebugSymbolGroupObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->sg = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgsg_dealloc(PyDebugSymbolGroupObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->sg != NULL) {
        self->sg = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgsg_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugSymbolGroupObject *self = (PyDebugSymbolGroupObject *)oself;
    if (self->sg)
        self->sg = NULL;
    return 0;
}

static PyObject *
dbgsg_OutputAsType(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Type;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "Is:OutputAsType", &Index, &Type))
        return NULL;
    if ((hr = self->sg->OutputAsType(Index, Type)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsg_AddSymbol(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Name;
    ULONG Index;

    Index = DEBUG_ANY_ID;
    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "s|I:AddSymbol", &Name, &Index))
        return NULL;
    if ((hr = self->sg->AddSymbol(Name, &Index)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Index);
}

static PyObject *
dbgsg_GetSymbolEntryInformation(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    DEBUG_SYMBOL_ENTRY Entry;
    PyDebugSymbolEntryObject *obj;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolEntryInformation", &Index))
        return NULL;
    if ((hr = self->sg->GetSymbolEntryInformation(Index, &Entry)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugSymbolEntryObject *)PyObject_CallObject(
            (PyObject *)&PyDebugSymbolEntryType, NULL);
    if (obj == NULL)
        Py_RETURN_NONE;
    obj->sym = Entry;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsg_ExpandSymbol(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    int Expand;
    PyObject *obj;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "IO:ExpandSymbol", &Index, &obj))
        return NULL;
    Expand = PyObject_IsTrue(obj);
    if (Expand == -1)
        return NULL;
    hr = self->sg->ExpandSymbol(Index, (BOOL)Expand);
    if (hr == S_OK || hr == S_FALSE)
        Py_RETURN_NONE;
    else if (hr == E_INVALIDARG)
        return err_invalidarg("The depth of the symbol is "
                "DEBUG_SYMBOL_EXPANSION_LEVEL_MASK, which is the maximum depth. "
                "This depth prevented the specified symbol's children from "
                "being added to this symbol group.");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgsg_GetSymbolName(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Buffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolName", &Index))
        return NULL;
    hr = self->sg->GetSymbolName(Index, NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    
    Buffer = (PSTR)PyMem_Malloc(NameSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->sg->GetSymbolName(Index, Buffer, NameSize, NULL)) != S_OK) {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s", Buffer);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsg_GetNumberSymbols(PyDebugSymbolGroupObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->sg == NULL)
        return err_nosg();
    if ((hr = self->sg->GetNumberSymbols(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgsg_GetSymbolOffset(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG64 Offset;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolOffset", &Index))
        return NULL;
    hr = self->sg->GetSymbolOffset(Index, &Offset);
    if (hr == S_OK)
        return Py_BuildValue("K", Offset);
    else if (hr == E_NOINTERFACE)
        return err_nointerface("The symbol does not have an absolute address.");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgsg_OutputSymbols(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG OutputControl;
    ULONG Flags;
    ULONG Start;
    ULONG Count;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "III:OutputSymbols", &Flags, &Start, &Count))
        return NULL;
    if ((hr = self->sg->OutputSymbols(OutputControl, Flags, Start, Count)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsg_GetSymbolParameters(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Start;
    ULONG Count;
    DEBUG_SYMBOL_PARAMETERS Params;
    PySymbolParametersObject *obj;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "II:GetSymbolParameters", &Start, &Count))
        return NULL;
    if ((hr = self->sg->GetSymbolParameters(Start, Count, &Params)) != S_OK)
        return err_dbgeng(hr);

    obj = (PySymbolParametersObject *)PyObject_CallObject(
            (PyObject *)&PySymbolParametersType, NULL);
    if (obj == NULL)
        Py_RETURN_NONE;
    obj->sp = Params;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsg_GetSymbolRegister(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG Register;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolRegister", &Index))
        return NULL;
    hr = self->sg->GetSymbolRegister(Index, &Register);
    if (hr == S_OK)
        return Py_BuildValue("I", Register);
    else if (hr == E_NOINTERFACE)
        return err_nointerface("The symbol's value and a pointer to it "
                "are not contained in a register, or the register is not known.");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgsg_RemoveSymbolByIndex(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:RemoveSymbolByIndex", &Index))
        return NULL;
    if ((hr = self->sg->RemoveSymbolByIndex(Index)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsg_RemoveSymbolByName(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Name;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "s:RemoveSymbolByName", &Name))
        return NULL;
    if ((hr = self->sg->RemoveSymbolByName(Name)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsg_GetSymbolSize(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG Size;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolSize", &Index))
        return NULL;
    if ((hr = self->sg->GetSymbolSize(Index, &Size)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Size);
}

static PyObject *
dbgsg_GetSymbolTypeName(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Buffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolTypeName", &Index))
        return NULL;
    hr = self->sg->GetSymbolTypeName(Index, NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    
    Buffer = (PSTR)PyMem_Malloc(NameSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->sg->GetSymbolTypeName(Index, 
                    Buffer, NameSize, NULL)) != S_OK) 
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s", Buffer);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsg_GetSymbolValueText(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Buffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "I:GetSymbolValueText", &Index))
        return NULL;
    hr = self->sg->GetSymbolValueText(Index, NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    
    Buffer = (PSTR)PyMem_Malloc(NameSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->sg->GetSymbolValueText(Index, 
                    Buffer, NameSize, NULL)) != S_OK) 
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s", Buffer);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsg_WriteSymbol(PyDebugSymbolGroupObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Value;

    if (self->sg == NULL)
        return err_nosg();
    if (!PyArg_ParseTuple(args, "Is:WriteSymbol", &Index, &Value))
        return NULL;
    if ((hr = self->sg->WriteSymbol(Index, Value)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugSymbolGroup_methods[] = {
    {"OutputAsType", 
     (PyCFunction)dbgsg_OutputAsType, METH_VARARGS, 
     "OutputAsType(Index, Type)\n"
     "  Changes the type of a symbol in the symbol group."
    },
    {"AddSymbol", 
     (PyCFunction)dbgsg_AddSymbol, METH_VARARGS, 
     "AddSymbol(Name, Index=DEBUG_ANY_ID) -> index\n"
     "  Add a symbol to a symbol group."
    },
    {"GetSymbolEntryInformation", 
     (PyCFunction)dbgsg_GetSymbolEntryInformation, METH_VARARGS, 
     "GetSymbolEntryInformation(Index) -> DEBUG_SYMBOL_ENTRY\n"
     "  Returns the information about a symbol in the symbol group."
    },
    {"ExpandSymbol", 
     (PyCFunction)dbgsg_ExpandSymbol, METH_VARARGS, 
     "ExpandSymbol(Index, Expand)\n"
     "  Adds or removes the children of a symbol from a symbol group."
    },
    {"GetSymbolName", 
     (PyCFunction)dbgsg_GetSymbolName, METH_VARARGS, 
     "GetSymbolName(Index) -> name\n"
     "  Returns the name of a symbol in a symbol group."
    },
    {"GetNumberSymbols", 
     (PyCFunction)dbgsg_GetNumberSymbols, METH_NOARGS, 
     "GetNumberSymbols() -> number\n"
     "  Returns the number of symbols in the symbol group."
    },
    {"GetSymbolOffset", 
     (PyCFunction)dbgsg_GetSymbolOffset, METH_VARARGS, 
     "GetSymbolOffset(Index) -> offset\n"
     "  Retrieves the location in the process's virtual address space of a\n"
     "  symbol in the symbol group."
    },
    {"OutputSymbols", 
     (PyCFunction)dbgsg_OutputSymbols, METH_VARARGS, 
     "OutputSymbols(Flags, Start, Count)\n"
     "  Prints the specified symbols to the console."
    },
    {"GetSymbolParameters", 
     (PyCFunction)dbgsg_GetSymbolParameters, METH_VARARGS, 
     "GetSymbolParameters(Start, Count) -> DEBUG_SYMBOL_PARAMETERS\n"
     "  Returns the symbol parameters that describe the specified symbols\n"
     "  in the symbol group."
    },
    {"GetSymbolRegister", 
     (PyCFunction)dbgsg_GetSymbolRegister, METH_VARARGS, 
     "GetSymbolRegister(Index) -> register\n"
     "  Returns the register that contains the value or a pointer to the\n"
     "  value of a symbol in the symbol group."
    },
    {"RemoveSymbolByIndex", 
     (PyCFunction)dbgsg_RemoveSymbolByIndex, METH_VARARGS, 
     "RemoveSymbolByIndex(Index)\n"
     "  Removes the specified symbol from the symbol group."
    },
    {"RemoveSymbolByName", 
     (PyCFunction)dbgsg_RemoveSymbolByName, METH_VARARGS, 
     "RemoveSymbolByName(Name)\n"
     "  Removes the specified symbol from the symbol group."
    },
    {"GetSymbolSize", 
     (PyCFunction)dbgsg_GetSymbolSize, METH_VARARGS, 
     "GetSymbolSize(Index) -> size\n"
     "  Returns the size of a symbol's value."
    },
    {"GetSymbolTypeName", 
     (PyCFunction)dbgsg_GetSymbolTypeName, METH_VARARGS, 
     "GetSymbolTypeName(Index) -> name\n"
     "  Return the name of the specified symbol's type."
    },
    {"GetSymbolValueText", 
     (PyCFunction)dbgsg_GetSymbolValueText, METH_VARARGS, 
     "GetSymbolValueText(Index) -> text value\n"
     "  Return a string that represents the value of a symbol."
    },
    {"WriteSymbol", 
     (PyCFunction)dbgsg_WriteSymbol, METH_VARARGS, 
     "WriteSymbol(Index, Value)\n"
     "  Set the vlaue of the specified symbol."
    },
    {NULL, NULL}
};

PyTypeObject PyDebugSymbolGroupType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSymbolGroup", /*tp_name*/
    sizeof(PyDebugSymbolGroupObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgsg_dealloc, /*tp_dealloc*/
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
    "DebugSymbolGroup objects", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugSymbolGroupObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugSymbolGroup_methods, /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsg_init,                /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgsg_new,                 /* tp_new */
    PyObject_Del,              /* tp_free */
};

