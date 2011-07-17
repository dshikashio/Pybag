#include "pydbgeng.h"

static PyObject *
dbgadv_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugAdvancedObject *self;

    self = (PyDebugAdvancedObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->adv = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgadv_dealloc(PyDebugAdvancedObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->adv != NULL) {
        self->adv->Release();
        self->adv = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgadv_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugAdvancedObject *self = (PyDebugAdvancedObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugAdvanced2 *debugAdvanced = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->adv) {
        self->adv->Release();
        self->adv = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugAdvanced",
                kwlist, &PyDebugClientType, &client))
        return -1;
    if (client->client == NULL) {
        err_noclient();
        return -1;
    }
    if ((hr = client->client->QueryInterface(__uuidof(IDebugAdvanced2),
                    (void **)&debugAdvanced)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->adv = debugAdvanced;
    return 0;
}

static PyObject *
dbgadv_GetThreadContext(PyDebugAdvancedObject *self, PyObject *args)
{
    HRESULT hr;
    PyObject *ret = NULL;
    ULONG Type;
    ULONG ContextSize;
    PVOID Context;
    CONTEXT32 ctx32;
    CONTEXT64 ctx64;

    if (self->adv == NULL)
        return err_noadv();
    if (!PyArg_ParseTuple(args, "I:GetThreadContext", &Type))
        return NULL;
    if (Type == IMAGE_FILE_MACHINE_I386) {
        ContextSize = sizeof(CONTEXT32);
        Context = (PVOID)&ctx32;
    }
    else if (Type == IMAGE_FILE_MACHINE_AMD64) {
        ContextSize = sizeof(CONTEXT64);
        Context = (PVOID)&ctx64;
    }
    else {
        return PyErr_Format(PyExc_TypeError, "Invalid processor type");
    }

    if ((hr = self->adv->GetThreadContext(Context, ContextSize)) != S_OK)
        return err_dbgeng(hr);

    if (Type == IMAGE_FILE_MACHINE_I386) {
        ret = PyObject_CallObject((PyObject *)&PyContext32Type, NULL);
        if (ret == NULL)
            Py_RETURN_NONE;
        ((PyContext32Object *)ret)->ctx = ctx32;
    }
    else {
        ret = PyObject_CallObject((PyObject *)&PyContext64Type, NULL);
        if (ret == NULL)
            Py_RETURN_NONE;
        ((PyContext64Object *)ret)->ctx = ctx64;
    }
    return Py_BuildValue("N", ret);
}

static PyObject *
dbgadv_SetThreadContext(PyDebugAdvancedObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Type;
    PVOID Context;
    ULONG ContextSize;
    PyObject *ctx;

    if (self->adv == NULL)
        return err_noadv();
    if (!PyArg_ParseTuple(args, "IO:SetThreadContext", &Type, &ctx))
        return NULL;
    if (Type == IMAGE_FILE_MACHINE_I386) {
        if (!PyObject_TypeCheck(ctx, &PyContext32Type))
            return PyErr_Format(PyExc_TypeError, "context must be Context32");
        ContextSize = sizeof(CONTEXT32);
        Context = &(((PyContext32Object *)ctx)->ctx);
    }
    else if (Type == IMAGE_FILE_MACHINE_AMD64) {
        if (!PyObject_TypeCheck(ctx, &PyContext64Type))
            return PyErr_Format(PyExc_TypeError, "context must be Context64");
        ContextSize = sizeof(CONTEXT64);
        Context = &(((PyContext64Object *)ctx)->ctx);
    }
    else {
        return PyErr_Format(PyExc_TypeError, "Invalid processor type");
    }
    if ((hr = self->adv->SetThreadContext(Context, ContextSize)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugAdvanced_methods[] = {
    {"GetThreadContext", 
     (PyCFunction)dbgadv_GetThreadContext, METH_VARARGS, 
     "GetThreadContext(type) -> ctx\n"
     "  Get current thread context.\n"
     "  Type is IMAGE_FILE_MACHINE_I386 or IMAGE_FILE_MACHINE_AMD64"
    },
    {"SetThreadContext", 
     (PyCFunction)dbgadv_SetThreadContext, METH_VARARGS, 
     "SetThreadContext(type,ctx)\n"
     "  Set current thread context.\n"
     "  Type is IMAGE_FILE_MACHINE_I386 or IMAGE_FILE_MACHINE_AMD64\n"
     "  Ctx is a Context32 or Context64 struct"
    },
// Request
// GetSourceFileInformation
// FindSourceFileInformation
// GetSymbolInformation
// GetSystemObjectInformation
    {NULL, NULL}
};

PyTypeObject PyDebugAdvancedType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugAdvanced",  /*tp_name*/
    sizeof(PyDebugAdvancedObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgadv_dealloc, /*tp_dealloc*/
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
    "DebugAdvanced objects", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugAdvancedObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugAdvanced_methods, /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgadv_init,              /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgadv_new,               /* tp_new */
    PyObject_Del,              /* tp_free */
};

