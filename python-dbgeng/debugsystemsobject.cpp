#include "pydbgeng.h"

static PyObject *
dbgsys_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugSystemsObjectsObject *self;

    self = (PyDebugSystemsObjectsObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->sys = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgsys_dealloc(PyDebugSystemsObjectsObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->sys != NULL) {
        self->sys->Release();
        self->sys = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgsys_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugSystemsObjectsObject *self = (PyDebugSystemsObjectsObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugSystemObjects4 *debugSystemObjects = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->sys) {
        self->sys->Release();
        self->sys = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugSystemsObject",
                kwlist, &PyDebugClientType, &client))
        return -1;
    if (client->client == NULL) {
        err_noclient();
        return -1;
    }
    if ((hr = client->client->QueryInterface(__uuidof(IDebugSystemObjects4),
                    (void **)&debugSystemObjects)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->sys = debugSystemObjects;
    return 0;
}

static PyObject *
dbgsys_GetEventThread(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetEventThread(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetEventProcess(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetEventProcess(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetNumberProcesses(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetNumberProcesses(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgsys_GetProcessIdByDataOffset(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetProcessIdByDataOffset", &Offset))
        return NULL;
    if ((hr = self->sys->GetProcessIdByDataOffset(Offset, &Id)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentProcessDataOffset(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessDataOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_GetCurrentProcessExecutableName(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG ExeSize;
    PyObject *ret = NULL;

    if (self->sys == NULL)
        return err_nosys();
    hr = self->sys->GetCurrentProcessExecutableName(NULL, 0, &ExeSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    Buffer = (PSTR)PyMem_Malloc(ExeSize * sizeof(*Buffer));

    if ((hr = self->sys->GetCurrentProcessExecutableName(Buffer, 
                    ExeSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsys_GetProcessIdByHandle(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Handle;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetProcessIdByHandle", &Handle))
        return NULL;
    if ((hr = self->sys->GetProcessIdByHandle(Handle, &Id)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentProcessHandle(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Handle;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessHandle(&Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}

static PyObject *
dbgsys_GetCurrentProcessId(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessId(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_SetCurrentProcessId(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:SetCurrentSystemId", &Id))
        return NULL;
    hr = self->sys->SetCurrentProcessId(Id);
    if (hr == S_OK)
        Py_RETURN_NONE;
    else if (hr == E_NOINTERFACE)
        return err_nointerface("No process with the given process ID was found.");
    else if (hr == E_FAIL)
        return err_fail("No suitable candidate for the current thread could "
                "be found in the process.");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgsys_GetProcessIdsByIndex(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG i;
    ULONG Count;
    ULONG *Ids = NULL;
    ULONG *SysIds = NULL;
    PyObject *ret = NULL;
    PyObject *idList = NULL;
    PyObject *sidList = NULL;


    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetNumberProcesses(&Count)) != S_OK)
        return err_dbgeng(hr);

    Ids    = (PULONG)PyMem_Malloc(Count * sizeof(ULONG));
    SysIds = (PULONG)PyMem_Malloc(Count * sizeof(ULONG));
    if (Ids == NULL || SysIds == NULL) {
        PyErr_NoMemory();
        goto done;
    }

    if ((hr = self->sys->GetProcessIdsByIndex(0, Count, Ids, SysIds)) != S_OK) {
        err_dbgeng(hr);
        goto done;
    }

    if ((idList = PyList_New(0)) == NULL)
        goto done;
    if ((sidList = PyList_New(0)) == NULL)
        goto done;

    for (i = 0; i < Count; i++) {
        PyObject *tmp = NULL;
        int status;

        tmp = PyInt_FromLong(Ids[i]);
        if (tmp == NULL)
            goto done;

        status = PyList_Append(idList, tmp);
        Py_DECREF(tmp);

        if (status)
            goto done;
    }

    for (i = 0; i < Count; i++) {
        PyObject *tmp = NULL;
        int status;

        tmp = PyInt_FromLong(SysIds[i]);
        if (tmp == NULL)
            goto done;

        status = PyList_Append(sidList, tmp);
        Py_DECREF(tmp);

        if (status)
            goto done;
    }
    ret = Py_BuildValue("(OO)", idList, sidList);

done:
    Py_XDECREF(idList);
    Py_XDECREF(sidList);
    if (Ids) PyMem_Free(Ids);
    if (SysIds) PyMem_Free(SysIds);
    return ret;
}

static PyObject *
dbgsys_GetProcessIdByPeb(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetProcessIdByPeb", &Offset))
        return NULL;
    if ((hr = self->sys->GetProcessIdByPeb(Offset, &Id)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentProcessPeb(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessPeb(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_GetProcessIdBySystemId(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG SysId;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:GetProcessIdBySystemId", &SysId))
        return NULL;
    if ((hr = self->sys->GetProcessIdBySystemId(SysId, &Id)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentProcessSystemId(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG SysId;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessSystemId(&SysId)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", SysId);
}

static PyObject *
dbgsys_GetNumberThreads(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetNumberThreads(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgsys_GetThreadIdByDataOffset(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetThreadIdByDataOffset", &Offset))
        return NULL;
    if ((hr = self->sys->GetThreadIdByDataOffset(Offset, &Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentThreadDataOffset(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentThreadDataOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_GetThreadIdByHandle(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Handle;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetThreadIdByHandle", &Handle))
        return NULL;
    if ((hr = self->sys->GetThreadIdByHandle(Handle, &Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentThreadHandle(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Handle;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentThreadHandle(&Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}

static PyObject *
dbgsys_GetCurrentThreadId(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentThreadId(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_SetCurrentThreadId(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:SetCurrentThreadId", &Id))
        return NULL;
    if ((hr = self->sys->SetCurrentThreadId(Id)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("No thread with the specified ID was found.");
        else
            return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyObject *
dbgsys_GetThreadIdsByIndex(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG i;
    ULONG Count;
    ULONG *Ids = NULL;
    ULONG *SysIds = NULL;
    PyObject *ret = NULL;
    PyObject *idList = NULL;
    PyObject *sidList = NULL;


    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetNumberThreads(&Count)) != S_OK)
        return err_dbgeng(hr);

    Ids    = (PULONG)PyMem_Malloc(Count * sizeof(ULONG));
    SysIds = (PULONG)PyMem_Malloc(Count * sizeof(ULONG));
    if (Ids == NULL || SysIds == NULL) {
        PyErr_NoMemory();
        goto done;
    }

    if ((hr = self->sys->GetThreadIdsByIndex(0, Count, Ids, SysIds)) != S_OK) {
        err_dbgeng(hr);
        goto done;
    }

    if ((idList = PyList_New(0)) == NULL)
        goto done;
    if ((sidList = PyList_New(0)) == NULL)
        goto done;

    for (i = 0; i < Count; i++) {
        PyObject *tmp = NULL;
        int status;

        tmp = PyInt_FromLong(Ids[i]);
        if (tmp == NULL)
            goto done;

        status = PyList_Append(idList, tmp);
        Py_DECREF(tmp);

        if (status)
            goto done;
    }

    for (i = 0; i < Count; i++) {
        PyObject *tmp = NULL;
        int status;

        tmp = PyInt_FromLong(SysIds[i]);
        if (tmp == NULL)
            goto done;

        status = PyList_Append(sidList, tmp);
        Py_DECREF(tmp);

        if (status)
            goto done;
    }
    ret = Py_BuildValue("(OO)", idList, sidList);

done:
    Py_XDECREF(idList);
    Py_XDECREF(sidList);
    if (Ids) PyMem_Free(Ids);
    if (SysIds) PyMem_Free(SysIds);
    return ret;
}

static PyObject *
dbgsys_GetThreadIdByProcessor(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Processor;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:GetThreadIdByProcessor", &Processor))
        return NULL;
    if ((hr = self->sys->GetThreadIdByProcessor(Processor, &Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetThreadIdBySystemId(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG SysId;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:GetThreadIdBySystemId", &SysId))
        return NULL;
    if ((hr = self->sys->GetThreadIdBySystemId(SysId, &Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentThreadSystemId(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG SysId;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentThreadSystemId(&SysId)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", SysId);
}

static PyObject *
dbgsys_GetThreadIdByTeb(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:GetThreadIdByTeb", &Offset))
        return NULL;
    if ((hr = self->sys->GetThreadIdByTeb(Offset, &Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentThreadTeb(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentThreadTeb(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_GetCurrentProcessUpTime(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG UpTime;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentProcessUpTime(&UpTime)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", UpTime);
}

static PyObject *
dbgsys_GetImplicitProcessDataOffset(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetImplicitProcessDataOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_SetImplicitProcessDataOffset(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:SetImplicitProcessDataOffset", &Offset))
        return NULL;
    if ((hr = self->sys->SetImplicitProcessDataOffset(Offset)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsys_GetImplicitThreadDataOffset(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetImplicitThreadDataOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsys_SetImplicitThreadDataOffset(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "K:SetImplicitThreadDataOffset", &Offset))
        return NULL;
    if ((hr = self->sys->SetImplicitThreadDataOffset(Offset)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsys_GetNumberSystems(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetNumberSystems(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgsys_GetEventSystem(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetEventSystem(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_GetCurrentSystemId(PyDebugSystemsObjectsObject *self)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if ((hr = self->sys->GetCurrentSystemId(&Id)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Id);
}

static PyObject *
dbgsys_SetCurrentSystemId(PyDebugSystemsObjectsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Id;

    if (self->sys == NULL)
        return err_nosys();
    if (!PyArg_ParseTuple(args, "I:SetCurrentSystemId", &Id))
        return NULL;
    if ((hr = self->sys->SetCurrentSystemId(Id)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugSystemsObject_methods[] = {
    {"GetEventThread", 
     (PyCFunction)dbgsys_GetEventThread, METH_NOARGS, 
     "GetEventThread() -> id\n"
     "  Returns the engine thread ID for the thread on which the last event occurred."
    },
    {"GetEventProcess", 
     (PyCFunction)dbgsys_GetEventProcess, METH_NOARGS, 
     "GetEventProcess() -> id\n"
     "  Returns the engine process ID for the process on which the last event occurred."
    },
    {"GetNumberProcesses", 
     (PyCFunction)dbgsys_GetNumberProcesses, METH_NOARGS, 
     "GetNumberProcesses() -> number\n"
     "  Returns the number of processes for the current target."
    },
    {"GetProcessIdByDataOffset", 
     (PyCFunction)dbgsys_GetProcessIdByDataOffset, METH_VARARGS, 
     "GetProcessIdByDataOffset(Offset) -> id\n"
     "  Returns the engine process ID for the specified process."
    },
    {"GetCurrentProcessDataOffset",
     (PyCFunction)dbgsys_GetCurrentProcessDataOffset, METH_VARARGS, 
     "GetCurrentProcessDataOffset() -> offset\n"
     "  Returns the location of the system data structure describing the\n"
     "  current process."
    },
    {"GetCurrentProcessExecutableName",
     (PyCFunction)dbgsys_GetCurrentProcessExecutableName, METH_NOARGS, 
     "GetCurrentProcessExecutableName() -> name\n"
     "  Returns the name of the executable file loaded in the current process."
    },
    {"GetProcessIdByHandle", 
     (PyCFunction)dbgsys_GetProcessIdByHandle, METH_VARARGS, 
     "GetProcessIdByHandle(Handle) -> id\n"
     "  Returns the engine process ID for the specified process."
    },
    {"GetCurrentProcessHandle", 
     (PyCFunction)dbgsys_GetCurrentProcessHandle, METH_NOARGS, 
     "GetCurrentProcessHandle() -> handle\n"
     "  Returns the system handle for the current process."
    },
    {"GetCurrentProcessId", 
     (PyCFunction)dbgsys_GetCurrentProcessId, METH_NOARGS, 
     "GetCurrentProcessId() -> id\n"
     "  Returns the engine process ID for the current process."
    },
    {"SetCurrentProcessId", 
     (PyCFunction)dbgsys_SetCurrentProcessId, METH_VARARGS, 
     "SetCurrentProcessId(id)\n"
     "  Makes the specified process the current process."
    },
    {"GetProcessIdsByIndex", 
     (PyCFunction)dbgsys_GetProcessIdsByIndex, METH_NOARGS, 
     "GetProcessIdsByIndex() -> ([Ids], [SysIds])\n"
     "  Returns the engine process IDs and system process IDs in the current target."
    },
    {"GetProcessIdByPeb", 
     (PyCFunction)dbgsys_GetProcessIdByPeb, METH_VARARGS, 
     "GetProcessIdByPeb(Offset) -> id\n"
     "  Returns the engine process ID for the specified process. Offset is\n"
     "  the address of the PEB."
    },
    {"GetCurrentProcessPeb",
     (PyCFunction)dbgsys_GetCurrentProcessPeb, METH_NOARGS, 
     "GetCurrentProcessPeb() -> offset\n"
     "  Returns the process environment block (PEB) of the current process."
    },
    {"GetProcessIdBySystemId", 
     (PyCFunction)dbgsys_GetProcessIdBySystemId, METH_VARARGS, 
     "GetProcessIdBySystemId(SysId) -> id\n"
     "  Returns the engine process ID for a process specified by its system process ID."
    },
    {"GetCurrentProcessSystemId", 
     (PyCFunction)dbgsys_GetCurrentProcessSystemId, METH_NOARGS, 
     "GetCurrentProcessSystemId() -> sysId\n"
     "  Returns the system process ID of the current process."
    },
    {"GetNumberThreads", 
     (PyCFunction)dbgsys_GetNumberThreads, METH_NOARGS, 
     "GetNumberThreads() -> number\n"
     "  Returns the number of threads in the current process."
    },
    // XXX - GetTotalNumberThreads
    {"GetThreadIdByDataOffset", 
     (PyCFunction)dbgsys_GetThreadIdByDataOffset, METH_VARARGS, 
     "GetThreadIdByDataOffset(Offset) -> id\n"
     "  Returns the engine thread ID for the specified thread."
    },
    {"GetCurrentThreadDataOffset",
     (PyCFunction)dbgsys_GetCurrentThreadDataOffset, METH_NOARGS, 
     "GetCurrentThreadDataOffset() -> offset\n"
     "  Returns the location of the system data structure for the current thread."
    },
    {"GetThreadIdByHandle", 
     (PyCFunction)dbgsys_GetThreadIdByHandle, METH_VARARGS, 
     "GetThreadIdByHandle(Handle) -> id\n"
     "  Returns the engine thread ID for the specified thread."
    },
    {"GetCurrentThreadHandle", 
     (PyCFunction)dbgsys_GetCurrentThreadHandle, METH_NOARGS, 
     "GetCurrentThreadHandle() -> handle\n"
     "  Returns the system handle for the current thread."
    },
    {"GetCurrentThreadId", 
     (PyCFunction)dbgsys_GetCurrentThreadId, METH_NOARGS, 
     "GetCurrentThreadId() -> id\n"
     "  Returns the engine thread ID for the current thread."
    },
    {"SetCurrentThreadId", 
     (PyCFunction)dbgsys_SetCurrentThreadId, METH_VARARGS, 
     "SetCurrentThreadId(id)\n"
     "  Makes the specified thread the current thread."
    },
    {"GetThreadIdsByIndex", 
     (PyCFunction)dbgsys_GetThreadIdsByIndex, METH_NOARGS, 
     "GetThreadIdsByIndex() -> ([ids], [sysids])\n"
     "  Returns the engine and system thread ids in the current process."
    },
    {"GetThreadIdByProcessor", 
     (PyCFunction)dbgsys_GetThreadIdByProcessor, METH_VARARGS, 
     "GetThreadIdByProcessor(Processor) -> id\n"
     "  Returns the engine thread ID for the kernel-mode virtual thread\n"
     "  corresponding to the specified processor."
    },
    {"GetThreadIdBySystemId", 
     (PyCFunction)dbgsys_GetThreadIdBySystemId, METH_VARARGS, 
     "GetThreadIdBySystemId(SysId) -> id\n"
     "  Returns the engine thread ID for the specified thread."
    },
    {"GetCurrentThreadSystemId", 
     (PyCFunction)dbgsys_GetCurrentThreadSystemId, METH_NOARGS, 
     "GetCurrentThreadSystemId() -> SysId\n"
     "  Returns the system thread ID of the current thread."
    },
    {"GetThreadIdByTeb", 
     (PyCFunction)dbgsys_GetThreadIdByTeb, METH_VARARGS, 
     "GetThreadIdByTeb(Offset) -> id\n"
     "  Returns the engine thread ID of the specified thread."
    },
    {"GetCurrentThreadTeb", 
     (PyCFunction)dbgsys_GetCurrentThreadTeb, METH_NOARGS, 
     "GetCurrentThreadTeb() -> offset\n"
     "  Returns the location of the thread environment block (TEB) for the\n"
     "  current thread."
    },
    {"GetCurrentProcessUpTime", 
     (PyCFunction)dbgsys_GetCurrentProcessUpTime, METH_NOARGS, 
     "GetCurrentProcessUpTime() -> uptime\n"
     "  Returns the length of time the current process has been running."
    },
    {"GetImplicitProcessDataOffset", 
     (PyCFunction)dbgsys_GetImplicitProcessDataOffset, METH_NOARGS, 
     "GetImplicitProcessDataOffset() -> offset\n"
     "  Returns the implicit process for the current target."
    },
    {"SetImplicitProcessDataOffset", 
     (PyCFunction)dbgsys_SetImplicitProcessDataOffset, METH_VARARGS, 
     "SetImplicitProcessDataOffset(Offset)\n"
     "  Sets the implicit process for the current target."
    },
    {"GetImplicitThreadDataOffset", 
     (PyCFunction)dbgsys_GetImplicitThreadDataOffset, METH_NOARGS, 
     "GetImplicitThreadDataOffset() -> offset\n"
     "  Returns the implicit thread for the current process."
    },
    {"SetImplicitThreadDataOffset", 
     (PyCFunction)dbgsys_SetImplicitThreadDataOffset, METH_VARARGS, 
     "SetImplicitThreadDataOffset(Offset)\n"
     "  Sets the implicit thread for the current process."
    },
    {"GetNumberSystems", 
     (PyCFunction)dbgsys_GetNumberSystems, METH_NOARGS, 
     "GetNumberSystems() -> number\n"
     "  Returns the number of targets to which the engine is currently connected."
    },
    {"GetEventSystem", 
     (PyCFunction)dbgsys_GetEventSystem, METH_NOARGS, 
     "GetEventSystem() -> id\n"
     "  Returns the engine target ID for the target in which the last event occurred."
    },
    // XXX - GetSystemIdsByIndex
    {"GetCurrentSystemId", 
     (PyCFunction)dbgsys_GetCurrentSystemId, METH_NOARGS, 
     "GetCurrentSystemId() -> id\n"
     "  Returns the engine target ID for the current process."
    },
    {"SetCurrentSystemId", 
     (PyCFunction)dbgsys_SetCurrentSystemId, METH_VARARGS, 
     "SetCurrentSystemId(Id)\n"
     "  Makes the specified target the current target."
    },
    // XXX - GetTotalNumberThreadsAndProcesses
    {NULL, NULL}
};

PyTypeObject PyDebugSystemsObjectsType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSystemsObject", /*tp_name*/
    sizeof(PyDebugSystemsObjectsObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgsys_dealloc, /*tp_dealloc*/
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
    "DebugSystemsObject objects", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugSystemsObjectsObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugSystemsObject_methods, /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsys_init,               /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgsys_new,                /* tp_new */
    PyObject_Del,              /* tp_free */
};
