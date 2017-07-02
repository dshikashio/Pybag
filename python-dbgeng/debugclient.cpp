#include "pydbgeng.h"

static PyObject *
dbgclient_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugClientObject *self;

    self = (PyDebugClientObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->client = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgclient_dealloc(PyDebugClientObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->client != NULL) {
        self->client->Release();
        self->client = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgclient_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugClientObject *self = (PyDebugClientObject *)oself;
    if (self->client) {
        self->client->Release();
        self->client = NULL;
    }
    return 0;
}

static PyObject *
dbgclient_SetEventCallback(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG EventType;
    PyObject *pycb;
    PyDebugEventCallbacks *ev;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "IO:SetEventCallback", 
                &EventType, &pycb))
        return NULL;
    if (!PyCallable_Check(pycb)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }
    if ((hr = self->client->GetEventCallbacks((IDebugEventCallbacks **)&ev)) != S_OK)
        return err_dbgeng(hr);
    
    switch (EventType) {
    case DEBUG_EVENT_BREAKPOINT:
        ev->RemoveBreakpointCB();
        ev->AddBreakpointCB(pycb);
        break;
    case DEBUG_EVENT_EXCEPTION:
        ev->RemoveExceptionCB();
        ev->AddExceptionCB(pycb);
        break;
    case DEBUG_EVENT_LOAD_MODULE:
        ev->RemoveLoadModuleCB();
        ev->AddLoadModuleCB(pycb);
        break;
    case DEBUG_EVENT_UNLOAD_MODULE:
        ev->RemoveUnloadModuleCB();
        ev->AddUnloadModuleCB(pycb);
        break;
    case DEBUG_EVENT_CREATE_PROCESS:
        ev->RemoveCreateProcessCB();
        ev->AddCreateProcessCB(pycb);
        break;
    case DEBUG_EVENT_EXIT_PROCESS:
        ev->RemoveExitProcessCB();
        ev->AddExitProcessCB(pycb);
        break;
    case DEBUG_EVENT_CREATE_THREAD:
        ev->RemoveCreateThreadCB();
        ev->AddCreateThreadCB(pycb);
        break;
    case DEBUG_EVENT_EXIT_THREAD:
        ev->RemoveExitThreadCB();
        ev->AddExitThreadCB(pycb);
        break;
    case DEBUG_EVENT_SYSTEM_ERROR:
        ev->RemoveSystemErrorCB();
        ev->AddSystemErrorCB(pycb);
        break;
    case DEBUG_EVENT_SESSION_STATUS:
        ev->RemoveSessionStatusCB();
        ev->AddSessionStatusCB(pycb);
        break;
    case DEBUG_EVENT_CHANGE_DEBUGGEE_STATE:
        ev->RemoveChangeDebuggeeStateCB();
        ev->AddChangeDebuggeeStateCB(pycb);
        break;
    case DEBUG_EVENT_CHANGE_ENGINE_STATE:
        ev->RemoveChangeEngineStateCB();
        ev->AddChangeEngineStateCB(pycb);
        break;
    case DEBUG_EVENT_CHANGE_SYMBOL_STATE:
        ev->RemoveChangeSymbolStateCB();
        ev->AddChangeSymbolStateCB(pycb);
        break;
    default:
        ev->Release();
        PyErr_SetString(PyExc_TypeError, "parameter must be DEBUG_EVENT type");
        return NULL;
    }

    if ((hr = self->client->SetEventCallbacks((IDebugEventCallbacks *)ev)) != S_OK) {
        ev->Release();
        return err_dbgeng(hr);
    }

    ev->Release();
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_RemoveEventCallback(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG EventType;
    PyDebugEventCallbacks *ev;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:RemoveEventCallback", &EventType))
        return NULL;
    if ((hr = self->client->GetEventCallbacks((IDebugEventCallbacks **)&ev)) != S_OK)
        return err_dbgeng(hr);
    
    switch (EventType) {
    case DEBUG_EVENT_BREAKPOINT:
        ev->RemoveBreakpointCB();
        break;
    case DEBUG_EVENT_EXCEPTION:
        ev->RemoveExceptionCB();
        break;
    case DEBUG_EVENT_LOAD_MODULE:
        ev->RemoveLoadModuleCB();
        break;
    case DEBUG_EVENT_UNLOAD_MODULE:
        ev->RemoveUnloadModuleCB();
        break;
    case DEBUG_EVENT_CREATE_PROCESS:
        ev->RemoveCreateProcessCB();
        break;
    case DEBUG_EVENT_EXIT_PROCESS:
        ev->RemoveExitProcessCB();
        break;
    case DEBUG_EVENT_CREATE_THREAD:
        ev->RemoveCreateThreadCB();
        break;
    case DEBUG_EVENT_EXIT_THREAD:
        ev->RemoveExitThreadCB();
        break;
    case DEBUG_EVENT_SYSTEM_ERROR:
        ev->RemoveSystemErrorCB();
        break;
    case DEBUG_EVENT_SESSION_STATUS:
        ev->RemoveSessionStatusCB();
        break;
    case DEBUG_EVENT_CHANGE_DEBUGGEE_STATE:
        ev->RemoveChangeDebuggeeStateCB();
        break;
    case DEBUG_EVENT_CHANGE_ENGINE_STATE:
        ev->RemoveChangeEngineStateCB();
        break;
    case DEBUG_EVENT_CHANGE_SYMBOL_STATE:
        ev->RemoveChangeSymbolStateCB();
    default:
        ev->Release();
        PyErr_SetString(PyExc_TypeError, "parameter must be DEBUG_EVENT type");
        return NULL;
    }

    if ((hr = self->client->SetEventCallbacks((IDebugEventCallbacks *)ev)) != S_OK) {
        ev->Release();
        return err_dbgeng(hr);
    }

    ev->Release();
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_DispatchCallbacks(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Timeout = INFINITE;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "|I:DispatchCallbacks", &Timeout))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->DispatchCallbacks(Timeout);
    Py_END_ALLOW_THREADS
    if (hr == S_OK)
        Py_RETURN_TRUE;
    else if (hr == S_FALSE)
        return exc_timeout("timeout elapsed");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgclient_ExitDispatch(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PyDebugClientObject *cli = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "O!:ExitDispatch", &PyDebugClientType, &cli))
        return NULL;
    if (cli->client == NULL)
        return err_noclient();

    if ((hr = self->client->ExitDispatch((IDebugClient *)cli->client)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_FlushCallbacks(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->FlushCallbacks();
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetNumberEventCallbacks(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG EventFlags;
    ULONG Count;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:GetNumberEventCallbacks", &EventFlags))
        return NULL;
    if ((hr = self->client->GetNumberEventCallbacks(EventFlags, &Count)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Count);
}

static PyObject *
dbgclient_SetOutputCallbacks(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;

    PyObject *pycb = NULL;
    PyDebugOutputCallbacks *ocb = NULL;
    
    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "|O:SetOutputCallbacks", &pycb))
        return NULL;

    if (pycb != NULL) {
        if (!PyCallable_Check(pycb)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }

        ocb = new PyDebugOutputCallbacks();
        if (ocb == NULL) {
            PyErr_NoMemory();
            return NULL;
        }
        ocb->SetCallback(pycb);
    }

    if ((hr = self->client->SetOutputCallbacks((PDEBUG_OUTPUT_CALLBACKS)ocb)) != S_OK) {
        ocb->Release();
        return err_dbgeng(hr);
    }

    // XXX - Documentation is Bullshit. Can't call release here even though the 
    // manual says it addrefs it doesn't.
    //if (ocb)
    //    ocb->Release();
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetNumberOutputCallbacks(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG Count;

    if (self->client == NULL)
        return err_noclient();
    if ((hr = self->client->GetNumberOutputCallbacks(&Count)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Count);
}

static PyObject *
dbgclient_CreateClient(PyDebugClientObject *self)
{
    HRESULT hr;
    IDebugClient7 *Client;
    PyDebugClientObject *cli = NULL;
    PyDebugEventCallbacks *ev = NULL;
    PyObject *ret = NULL;

    if (self->client == NULL)
        return err_noclient();

    if ((hr = self->client->CreateClient((IDebugClient **)&Client)) != S_OK)
        return err_dbgeng(hr);

    cli = (PyDebugClientObject *)PyObject_CallObject(
            (PyObject *)&PyDebugClientType, NULL);
    if (cli == NULL)
        Py_RETURN_NONE;

    cli->client = Client;

    ev = new PyDebugEventCallbacks();
    if (ev == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    if ((hr = cli->client->SetEventCallbacks((IDebugEventCallbacks *)ev)) != S_OK) {
        err_dbgeng(hr);
        goto error;
    }

    ret = Py_BuildValue("N", cli);

error:
    if (ret == NULL) {
        if (ev) ev->Release();
        if (cli) { 
            if (cli->client) {
                cli->client->Release();
                cli->client = NULL;
            }
            Py_DECREF(cli);
        }
    }
    return ret;
}

static PyObject *
dbgclient_AddDumpInformationFile(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR InfoFile;
    ULONG Type = DEBUG_DUMP_FILE_PAGE_FILE_DUMP;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s|I:AddDumpInformationFile", &InfoFile, &Type))
        return NULL;
    if ((hr = self->client->AddDumpInformationFile(InfoFile, Type)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetDumpFile(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Buffer = NULL;
    ULONG NameSize;
    ULONG Type;
    PyObject *ret = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:GetDumpFile", &Index))
        return NULL;
    hr = self->client->GetDumpFile(Index, NULL, 0, &NameSize, NULL, &Type);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(NameSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->client->GetDumpFile(Index, Buffer, NameSize, 
                    NULL, NULL, &Type)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("(sI)", Buffer, Type);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgclient_GetNumberDumpFiles(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG Number;

    if (self->client == NULL)
        return err_noclient();
    if ((hr = self->client->GetNumberDumpFiles(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgclient_OpenDumpFile(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR DumpFile;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s:OpenDumpFile", &DumpFile))
        return NULL;
    if ((hr = self->client->OpenDumpFile(DumpFile)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_WriteDumpFile(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR DumpFile;
    ULONG Qualifier;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "sI:WriteDumpFile", &DumpFile, &Qualifier))
        return NULL;
    if ((hr = self->client->WriteDumpFile(DumpFile, Qualifier)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_WriteDumpFile2(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR DumpFile;
    ULONG Qualifier;
    ULONG FormatFlags;
    PSTR Comment = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "sII|s:WriteDumpFile2", &DumpFile, &Qualifier,
                &FormatFlags, &Comment))
        return NULL;
    if ((hr = self->client->WriteDumpFile2(DumpFile, Qualifier, 
                    FormatFlags, Comment)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetExitCode(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG Code;

    if (self->client == NULL)
        return err_noclient();
    hr = self->client->GetExitCode(&Code);
    if (hr == S_OK)
        return Py_BuildValue("I", Code);
    else if (hr == S_FALSE)
        return err_false("the process is still running");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgclient_GetIdentity(PyDebugClientObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG IdentitySize;
    PyObject *ret = NULL;

    if (self->client == NULL)
        return err_noclient();
    hr = self->client->GetIdentity(NULL, 0, &IdentitySize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(IdentitySize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->client->GetIdentity(Buffer, IdentitySize, NULL)) != S_OK) {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s", Buffer);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgclient_OutputIdentity(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;

    if (self->client == NULL)
        return err_noclient();
    if ((hr = self->client->OutputIdentity(OutputControl, 0, "%s\n")) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_AttachKernel(PyDebugClientObject *self, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    ULONG Flags;
    PCSTR ConnectOptions = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I|s:AttachKernel", &Flags, &ConnectOptions))
        return NULL;
    if ((hr = self->client->AttachKernel(Flags, ConnectOptions)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetKernelConnectionOptions(PyDebugClientObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG OptionsSize = 0;
    PyObject *ret = NULL;

    if (self->client == NULL)
        return err_noclient();
    hr = self->client->GetKernelConnectionOptions(NULL, 0, &OptionsSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(OptionsSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        goto error;
    }
    if ((hr = self->client->GetKernelConnectionOptions(Buffer, 
                    OptionsSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto error;
    }
    ret = Py_BuildValue("s", Buffer);
error:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgclient_SetKernelConnectionOptions(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Options = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s:SetKernelConnectionOptions", &Options))
        return NULL;
    if ((hr = self->client->SetKernelConnectionOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_IsKernelDebuggerEnabled(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    hr = self->client->IsKernelDebuggerEnabled();
    if (hr == S_OK)
        Py_RETURN_TRUE;
    else if (hr == S_FALSE)
        Py_RETURN_FALSE;
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgclient_GetOutputMask(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG Mask;

    if (self->client == NULL)
        return err_noclient();
    if ((hr = self->client->GetOutputMask(&Mask)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Mask);
}

static PyObject *
dbgclient_SetOutputMask(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Mask;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:SetOutputMask", &Mask))
        return NULL;
    if ((hr = self->client->SetOutputMask(Mask)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_AttachProcess(PyDebugClientObject *self, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    static char *kwlist[] = {"server", "pid", "flags", NULL};
    ULONG64 Server = 0;
    ULONG ProcessId = -1;
    ULONG Flags = DEBUG_ATTACH_DEFAULT;

    if (self->client == NULL)
        return err_noclient();

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "KI|I:AttachProcess",
                kwlist, &Server, &ProcessId, &Flags))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    hr = self->client->AttachProcess(Server, ProcessId, Flags);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_CreateProcess(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Server = 0;
    PSTR CommandLine = NULL;
    ULONG CreateFlags = 0;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "KsI:CreateProcess", &Server, 
                &CommandLine, &CreateFlags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->CreateProcess(Server, CommandLine, CreateFlags);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_CreateProcessAndAttach(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Server = 0;
    PSTR CommandLine = NULL;
    ULONG CreateFlags = 0;
    ULONG ProcessId = 0;
    ULONG AttachFlags = 0;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "KsIII:CreateProcessAndAttach", 
                &Server, &CommandLine, &CreateFlags, &ProcessId, &AttachFlags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->CreateProcessAndAttach(Server, CommandLine, CreateFlags, 
                        ProcessId, AttachFlags);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
    {
        return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

// XXX - CreateProcessAndAttach2
/*
static PyObject *
dbgclient_CreateProcessAndAttach2(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR CommandLine = NULL;
    ULONG CreateFlags = 0;
    ULONG AttachFlags = 0;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "sII", &CommandLine, &CreateFlags, &AttachFlags))
        return NULL;
    if ((hr = self->client->CreateProcessAndAttach(0, CommandLine, CreateFlags, 
                    0, AttachFlags)) != S_OK)
    {
        return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}
*/

static PyObject *
dbgclient_DetachProcesses(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();

    Py_BEGIN_ALLOW_THREADS
    hr = self->client->DetachProcesses();
    Py_END_ALLOW_THREADS
    if (hr != S_OK) {
        return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_TerminateProcesses(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();

    Py_BEGIN_ALLOW_THREADS
    hr = self->client->TerminateProcesses();
    Py_END_ALLOW_THREADS
    if (hr != S_OK) {
        return err_dbgeng(hr);
    }
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_AbandonCurrentProcess(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->AbandonCurrentProcess();
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_DetachCurrentProcess(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->DetachCurrentProcess();
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_TerminateCurrentProcess(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->TerminateCurrentProcess();
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_AddProcessOptions(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:AddProcessOptions", &Options))
        return NULL;
    if ((hr = self->client->AddProcessOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetProcessOptions(PyDebugClientObject *self)
{
    HRESULT hr;
    ULONG Options;

    if (self->client == NULL)
        return err_noclient();
    if ((hr = self->client->GetProcessOptions(&Options)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Options);
}

static PyObject *
dbgclient_RemoveProcessOptions(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:RemoveProcessOptions", &Options))
        return NULL;
    if ((hr = self->client->RemoveProcessOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_SetProcessOptions(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:SetProcessOptions", &Options))
        return NULL;
    if ((hr = self->client->SetProcessOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_GetRunningProcessDescription(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG pid = -1;
    ULONG flags = 0;
    PSTR ExeName = NULL;
    ULONG ExeNameSize = 0;
    ULONG ActualExeNameSize = 0;
    PSTR Description = NULL;
    ULONG DescriptionSize = 0;
    ULONG ActualDescriptionSize = 0;

    PyObject *ret = NULL;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I|I:GetRunningProcessDescription", &pid, &flags))
        return NULL;

    hr = self->client->GetRunningProcessDescription(0, pid, flags, 
                ExeName, ExeNameSize, &ActualExeNameSize,
                Description, DescriptionSize, &ActualDescriptionSize);
    if (hr != S_OK && hr != S_FALSE)
    {
        return err_dbgeng(hr);
    }

    ExeNameSize = ActualExeNameSize;
    ExeName = (PSTR)PyMem_Malloc(ExeNameSize);
    if (ExeName == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    DescriptionSize = ActualDescriptionSize;
    Description = (PSTR)PyMem_Malloc(DescriptionSize);
    if (Description == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    if ((hr = self->client->GetRunningProcessDescription(0, pid, flags, 
                ExeName, ExeNameSize, &ActualExeNameSize,
                Description, DescriptionSize, &ActualDescriptionSize)) != S_OK)
    {
        err_dbgeng(hr);
        goto error; 
    }

    ret = Py_BuildValue("ss", ExeName, Description);

error:
    if (ExeName) PyMem_Free(ExeName);
    if (Description) PyMem_Free(Description);
    return ret;
}

static PyObject *
dbgclient_GetRunningProcessSystemIds(PyDebugClientObject *self)
{
    HRESULT hr;
    PyObject *pid_list = NULL;
    ULONG *ids = NULL;
    ULONG count = 0;
    ULONG needed = 0;

    if (self->client == NULL)
        return err_noclient();

    if ((hr = self->client->GetRunningProcessSystemIds(0, 
                    NULL, count, &needed)) != S_OK) 
    {
        return err_dbgeng(hr);
    }

    count = needed;
    ids = (PULONG)PyMem_Malloc(count * sizeof(ULONG));
    if (ids == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    if ((hr = self->client->GetRunningProcessSystemIds(0, 
                    ids, count, &needed)) != S_OK)
    {
        err_dbgeng(hr);
        goto error;
    }

    if ((pid_list = PyList_New(0)) == NULL)
        goto error;

    for (count = 0; count < needed; count++) {
        PyObject *tmp = NULL;
        int status;

        tmp = PyInt_FromLong(ids[count]);
        if (tmp == NULL)
            goto error;

        status = PyList_Append(pid_list, tmp);
        Py_DECREF(tmp);

        if (status)
            goto error;
    }

    PyMem_Free(ids);
    PyList_Sort(pid_list);
    return pid_list;

error:
    Py_XDECREF(pid_list);
    if (ids != NULL) PyMem_Free(ids);
    return NULL;
}

static PyObject *
dbgclient_ConnectProcessServer(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR RemoteOptions;
    ULONG64 Server;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s:ConnectProcessServer", &RemoteOptions))
        return NULL;

    if ((hr = self->client->ConnectProcessServer(RemoteOptions, &Server)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("K", Server);
}

static PyObject *
dbgclient_DisconnectProcessServer(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Server;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "K:DisconnectProcessServer", &Server))
        return NULL;

    if ((hr = self->client->DisconnectProcessServer(Server)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}

static PyObject *
dbgclient_EndProcessServer(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Server;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "K:EndProcessServer", &Server))
        return NULL;

    if ((hr = self->client->EndProcessServer(Server)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}

static PyObject *
dbgclient_StartProcessServer(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Options;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s:StartProcessServer", &Options))
        return NULL;

    if ((hr = self->client->StartProcessServer(DEBUG_CLASS_USER_WINDOWS,
                    Options, NULL)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}

static PyObject *
dbgclient_WaitForProcessServerEnd(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Timeout;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:WaitForProcessServerEnd", &Timeout))
        return NULL;

    if ((hr = self->client->WaitForProcessServerEnd(Timeout)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}

static PyObject *
dbgclient_OutputServers(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Machine;
    ULONG Flags;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "sI:OutputServers", &Machine, &Flags))
        return NULL;

    if ((hr = self->client->OutputServers(DEBUG_OUTCTL_ALL_CLIENTS,
                    Machine, Flags)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}

static PyObject *
dbgclient_StartServer(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Options;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "s:StartServer", &Options))
        return NULL;
    if ((hr = self->client->StartServer(Options)) != S_OK)
        return err_dbgeng(hr);

    Py_RETURN_NONE;
}


static PyObject *
dbgclient_ConnectSession(PyDebugClientObject *self)
{
    HRESULT hr;

    if (self->client == NULL)
        return err_noclient();
    Py_BEGIN_ALLOW_THREADS
    hr = self->client->ConnectSession(0,0);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgclient_EndSession(PyDebugClientObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Flags;

    if (self->client == NULL)
        return err_noclient();
    if (!PyArg_ParseTuple(args, "I:EndSession", &Flags))
        return NULL;
    if ((hr = self->client->EndSession(Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugClient_methods[] = {
    // Custom event hacks
    {"SetEventCallback", 
     (PyCFunction)dbgclient_SetEventCallback, METH_VARARGS, 
     "SetEventCallback(type,callbackfn)\n"
     "  Sets an event callback for a specific event type\n"
     "  Types:\n"
     "    DEBUG_EVENT_BREAKPOINT\n"
     "    DEBUG_EVENT_EXCEPTION\n"
     "    DEBUG_EVENT_LOAD_MODULE\n"
     "    DEBUG_EVENT_UNLOAD_MODULE\n"
     "    DEBUG_EVENT_CREATE_PROCESS\n"
     "    DEBUG_EVENT_EXIT_PROCESS\n"
     "    DEBUG_EVENT_CREATE_THREAD\n"
     "    DEBUG_EVENT_EXIT_THREAD\n"
     "    DEBUG_EVENT_SYSTEM_ERROR\n"
     "    DEBUG_EVENT_SESSION_STATUS\n"
     "    DEBUG_EVENT_CHANGE_DEBUGGEE_STATE\n"
     "    DEBUG_EVENT_CHANGE_ENGINE_STATE\n"
     "    DEBUG_EVENT_CHANGE_SYMBOL_STATE"
    },
    {"RemoveEventCallback", 
     (PyCFunction)dbgclient_RemoveEventCallback, METH_VARARGS, 
     "RemoveEventCallback(type)\n"
     "  Remove an event callback for a specific event type"
    },


    {"DispatchCallbacks", 
     (PyCFunction)dbgclient_DispatchCallbacks, METH_VARARGS, 
     "DispatchCallbacks(timeout=INFINITE)\n"
     "  Lets the debugger engine use the current thread for callbacks"
    },
    {"ExitDispatch", 
     (PyCFunction)dbgclient_ExitDispatch, METH_VARARGS, 
     "ExitDispatch(dbgclient)\n"
     "  causes the DispatchCallbacks method to return"
    },
    {"FlushCallbacks", 
     (PyCFunction)dbgclient_FlushCallbacks, METH_NOARGS, 
     "FlushCallbacks()\n"
     "  forces any remaining buffered output to be delivered to the output\n"
     "  callback object registered with the client"
    },
// GetEventCallbacks
// SetEventCallbacks
    {"GetNumberEventCallbacks", 
     (PyCFunction)dbgclient_GetNumberEventCallbacks, METH_VARARGS, 
     "GetNumberEventCallbacks(event_flags) -> count\n"
     "  See DEBUG_EVENT_XXX for event_flags"
    },
// GetInputCallbacks
// SetInputCallbacks
// GetNumberInputCallbacks
// GetOutputCallbacks
    {"SetOutputCallbacks", 
     (PyCFunction)dbgclient_SetOutputCallbacks, METH_VARARGS, 
     "SetOutputCallbacks(writefn)\n"
     "  Registers an output callback fn with this client"
    },
    {"GetNumberOutputCallbacks", 
     (PyCFunction)dbgclient_GetNumberOutputCallbacks, METH_NOARGS, 
     "GetNumberOutputCallbacks() -> count\n"
     "  Returns the number of output callbacks registered over all clients"
    },
    {"CreateClient", 
     (PyCFunction)dbgclient_CreateClient, METH_NOARGS, 
     "CreateClient() -> IDebugClient\n"
     "  Creates a new client object for the current thread"
    },
    {"AddDumpInformationFile", 
     (PyCFunction)dbgclient_AddDumpInformationFile, METH_VARARGS, 
     "AddDumpInformationFile(InfoFile, type=DEBUG_DUMP_FILE_PAGE_FILE_DUMP)\n"
     "  Registers addtional files containing supporting information that will be\n"
     "  used when opening a dump file."
    },
    {"GetDumpFile", 
     (PyCFunction)dbgclient_GetDumpFile, METH_VARARGS, 
     "GetDumpFile(index) -> (filename, type)\n"
     "  Describes the files containing supporting information that were used when\n"
     "  opening the current dump target."
    },
    {"GetNumberDumpFiles", 
     (PyCFunction)dbgclient_GetNumberDumpFiles, METH_NOARGS, 
     "GetNumberDumpFiles() -> number\n"
     "  Returns the number of files containing supporting information that were used\n"
     "  when opening the current dump target."
    },
    {"OpenDumpFile", 
     (PyCFunction)dbgclient_OpenDumpFile, METH_VARARGS, 
     "OpenDumpFile(dumpfile)\n"
     "  Opens a dump file as a debugger target."
    },
    {"WriteDumpFile", 
     (PyCFunction)dbgclient_WriteDumpFile, METH_VARARGS, 
     "WriteDumpFile(dumpfile, qualifier)\n"
     "  Creates a user-mode or kernel-mode crash dump file.\n"
     "  Qualifiers:\n"
     "    DEBUG_DUMP_SMALL\n"
     "    DEBUG_DUMP_DEFAULT\n"
     "    DEBUG_DUMP_FULL\n"
     "    DEBUG_KERNEL_SMALL_DUMP\n"
     "    DEBUG_KERNEL_DUMP\n"
     "    DEBUG_KERNEL_FULL_DUMP\n"
     "    DEBUG_USER_WINDOWS_SMALL_DUMP\n"
     "    DEBUG_USER_WINDOWS_DUMP"
    },
    {"WriteDumpFile2", 
     (PyCFunction)dbgclient_WriteDumpFile2, METH_VARARGS, 
     "WriteDumpFile2(dumpfile, qualifier, FormatFlags, Comment='')\n"
     "  Creates a user-mode or kernel-mode crash dump file.\n"
     "  Qualifiers:\n"
     "    DEBUG_DUMP_SMALL\n"
     "    DEBUG_DUMP_DEFAULT\n"
     "    DEBUG_DUMP_FULL\n"
     "    DEBUG_KERNEL_SMALL_DUMP\n"
     "    DEBUG_KERNEL_DUMP\n"
     "    DEBUG_KERNEL_FULL_DUMP\n"
     "    DEBUG_USER_WINDOWS_SMALL_DUMP\n"
     "    DEBUG_USER_WINDOWS_DUMP\n"
     "  FormatFlags:\n"
     "    DEBUG_FORMAT_WRITE_CAB\n"
     "    DEBUG_FORMAT_CAB_SECONDARY_FILES\n"
     "    DEBUG_FORMAT_NO_OVERWRITE\n"
     "    DEBUG_FORMAT_USER_SMALL_FULL_MEMORY\n"
     "    DEBUG_FORMAT_USER_SMALL_HANDLE_DATA\n"
     "    DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES\n"
     "    DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY\n"
     "    DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS\n"
     "    DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY\n"
     "    DEBUG_FORMAT_USER_SMALL_FILTER_PATHS\n"
     "    DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA\n"
     "    DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY\n"
     "    DEBUG_FORMAT_USER_SMALL_NO_OPTIONAL_DATA\n"
     "    DEBUG_FORMAT_USER_SMALL_FULL_MEMORY_INFO\n"
     "    DEBUG_FORMAT_USER_SMALL_THREAD_INFO\n"
     "    DEBUG_FORMAT_USER_SMALL_CODE_SEGMENTS"
    },
    {"GetExitCode", 
     (PyCFunction)dbgclient_GetExitCode, METH_NOARGS, 
     "GetExitCode() -> code\n"
     "  Returns the exit code of the current process."
    },
    {"GetIdentity", 
     (PyCFunction)dbgclient_GetIdentity, METH_NOARGS, 
     "GetIdentity() -> identity\n"
     "Returns a string describing the computer and user this client represents."
    },
    {"OutputIdentity", 
     (PyCFunction)dbgclient_OutputIdentity, METH_NOARGS, 
     "OutputIdentity()\n"
     "Outputs a string describing the computer and user this client represents."
    },
    {"AttachKernel", 
     (PyCFunction)dbgclient_AttachKernel, METH_VARARGS, 
     "AttachKernel(flags, ConnectOptions=NULL)\n"
     "  Connects the debugger engine to a kernel target.\n"
     "  Flags:\n"
     "    DEBUG_ATTACH_KERNEL_CONNECTION\n"
     "    DEBUG_ATTACH_LOCAL_KERNEL\n"
     "    DEBUG_ATTACH_EXDI_DRIVER\n"
     "  ConnectOptions:\n"
     "    Set for DEBUG_ATTACH_KERNEL_CONNECTION.  Interpreted as what follows -k.\n"
     "    ex. com:port=COM1,baud=115200"
    },
    {"GetKernelConnectionOptions", 
     (PyCFunction)dbgclient_GetKernelConnectionOptions, METH_NOARGS, 
     "GetKernelConnectionOptions() -> options\n"
     "  Return the connection options for the current kernel target."
    },
    {"SetKernelConnectionOptions", 
     (PyCFunction)dbgclient_SetKernelConnectionOptions, METH_VARARGS, 
     "SetKernelConnectionOptions(options)\n"
     "  Update some of the connection options for a live kernel target."
    },
    {"IsKernelDebuggerEnabled", 
     (PyCFunction)dbgclient_IsKernelDebuggerEnabled, METH_NOARGS, 
     "IsKernelDebuggerEnabled() -> true/false\n"
     "  Checks whether kernel debugging is enabled for the local kernel."
    },
    {"GetOutputMask", 
     (PyCFunction)dbgclient_GetOutputMask, METH_NOARGS, 
     "GetOutputMask() -> mask\n"
     "  Returns the output mask current set for the client."
    },
    {"SetOutputMask", 
     (PyCFunction)dbgclient_SetOutputMask, METH_VARARGS, 
     "SetOutputMask(mask)\n"
     "  Sets the output mask for the client.\n"
     "  Mask:\n"
     "    DEBUG_OUTPUT_NORMAL\n"
     "    DEBUG_OUTPUT_ERROR\n"
     "    DEBUG_OUTPUT_WARNING\n"
     "    DEBUG_OUTPUT_VERBOSE\n"
     "    DEBUG_OUTPUT_PROMPT\n"
     "    DEBUG_OUTPUT_PROMPT_REGISTERS\n"
     "    DEBUG_OUTPUT_EXTENSION_WARNING\n"
     "    DEBUG_OUTPUT_DEBUGGEE\n"
     "    DEBUG_OUTPUT_DEBUGGEE_PROMPT\n"
     "    DEBUG_OUTPUT_SYMBOLS"
    },
// GetOtherOutputMask
// SetOtherOutputMask
    {"AttachProcess", 
     (PyCFunction)dbgclient_AttachProcess, METH_VARARGS | METH_KEYWORDS, 
     "AttachProcess(server, pid, flags=DEBUG_ATTACH_DEFAULT)\n"
     "  Connects the debugger engine to a user-mode process.\n"
     "  Server: Set to 0 for localhost\n"
     "  Flags:\n"
     "    DEBUG_ATTACH_NONINVASIVE\n"
     "    DEBUG_ATTACH_EXISTING\n"
     "    DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND\n"
     "    DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK\n"
     "    DEBUG_ATTACH_INVASIVE_RESUME_PROCESS"
    },
    {"CreateProcess", 
     (PyCFunction)dbgclient_CreateProcess, METH_VARARGS, 
     "CreateProcess(server, cmdline, create_flags)\n"
     "  Create a process from the specified command line.\n"
     "  Server: Set to 0 for localhost\n"
     "  Create_flags:\n"
     "    DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP\n"
     "    DEBUG_CREATE_PROCESS_THROUGH_RTL\n"
     "    DEBUG_PROCESS\n"
     "    DEBUG_ONLY_THIS_PROCESS"
    },
// CreateProcess2
    {"CreateProcessAndAttach", 
     (PyCFunction)dbgclient_CreateProcessAndAttach, METH_VARARGS, 
     "CreateProcessAndAttach(server, cmdline, create_flags, pid, attach_flags)\n"
     "  Create a process from a specified command line, then attach to another\n"
     "  user-mode process.  The created process is suspended and only allowed to\n"
     "  execute when the attach has completed.  This allows rough synchronization\n"
     "  when debugging both, client and server processes.\n"
     "  Server: Set to 0 for localhost\n"
     "  Create_flags:\n"
     "    DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP\n"
     "    DEBUG_CREATE_PROCESS_THROUGH_RTL\n"
     "    DEBUG_PROCESS\n"
     "    DEBUG_ONLY_THIS_PROCESS\n"
     "  Pid: Set to 0 to attach process created by command line\n"
     "  attach_flags:\n"
     "    DEBUG_ATTACH_NONINVASIVE\n"
     "    DEBUG_ATTACH_EXISTING\n"
     "    DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND\n"
     "    DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK\n"
     "    DEBUG_ATTACH_INVASIVE_RESUME_PROCESS"
    },
// CreateProcessAndAttach2
    {"DetachProcesses", 
     (PyCFunction)dbgclient_DetachProcesses, METH_NOARGS, 
     "DetachProcesses()\n"
     "  Detaches the debugger engine from all processes in all targets, resuming\n"
     "  all their threads."
    },
    {"TerminateProcesses", 
     (PyCFunction)dbgclient_TerminateProcesses, METH_NOARGS, 
     "TerminateProcesses()\n"
     "  Attempts to terminate all processes in all targets."
    },
    {"AbandonCurrentProcess", 
     (PyCFunction)dbgclient_AbandonCurrentProcess, METH_NOARGS, 
     "AbandonCurrentProcess()\n"
     "  Removes the current process from the debugger engine's process list without\n"
     "  detaching or terminating the process."
    },
    {"DetachCurrentProcess", 
     (PyCFunction)dbgclient_DetachCurrentProcess, METH_NOARGS, 
     "DetachCurrentProcess()\n"
     "  Detaches the debugger engine from the current process resuming all its threads."
    },
    {"TerminateCurrentProcess", 
     (PyCFunction)dbgclient_TerminateCurrentProcess, METH_NOARGS, 
     "TerminateCurrentProcess()\n"
     "  Attempts to terminate the current process."
    },
    {"AddProcessOptions", 
     (PyCFunction)dbgclient_AddProcessOptions, METH_VARARGS, 
     "AddProcessOptions(options)\n"
     "  Adds to the process options of the current process."
     "  options:\n"
     "    DEBUG_PROCESS_DETACH_ON_EXIT\n"
     "    DEBUG_PROCESS_ONLY_THIS_PROCESS"
    },
    {"GetProcessOptions", 
     (PyCFunction)dbgclient_GetProcessOptions, METH_NOARGS, 
     "GetProcessOptions() -> options\n"
     "  Retrieves the process options affecting the current process."
    },
    {"RemoveProcessOptions", 
     (PyCFunction)dbgclient_RemoveProcessOptions, METH_VARARGS, 
     "RemoveProcessOptions(options)\n"
     "  Removes process options for the current process."
    },
    {"SetProcessOptions", 
     (PyCFunction)dbgclient_SetProcessOptions, METH_VARARGS, 
     "SetProcessOptions(options)\n"
     "  Sets the process options for the current process."
    },
    {"GetRunningProcessDescription", 
     (PyCFunction)dbgclient_GetRunningProcessDescription, METH_VARARGS, 
     "GetRunningProcessDescription(pid, flags=0) -> (name, description)\n"
     "  Returns a description of the process that inclues the executable image\n"
     "  name, the service names, the MTS package names, and the command line.\n"
     "  flags:\n"
     "    DEBUG_PROC_DESC_NO_PATHS\n"
     "    DEBUG_PROC_DESC_NO_SERVICES\n"
     "    DEBUG_PROC_DESC_NO_MTS_PACKAGES\n"
     "    DEBUG_PROC_DESC_NO_COMMAND_LINE"
    },
    {"GetRunningProcessSystemIds", 
     (PyCFunction)dbgclient_GetRunningProcessSystemIds, METH_NOARGS, 
     "GetRunningProcessSystemIds() -> [pid list]\n"
     "  Returns the process IDs for each running process."
    },
// GetRunningProcessSystemIdByExecutableName
    {"ConnectProcessServer", 
     (PyCFunction)dbgclient_ConnectProcessServer, METH_VARARGS, 
     "ConnectProcessServer(RemoteOptions) -> server\n"
     "  Connect to a process server.\n"
     "  RemoteOptions:\n"
     "    The same as passed to the -premote option on the command line.\n"
     "    ex. tcp:server=192.168.1.1,port=5555"
    },
    {"DisconnectProcessServer", 
     (PyCFunction)dbgclient_DisconnectProcessServer, METH_VARARGS, 
     "DisconnectProcessServer(Server)\n"
     "  Disconnects the debugger engine from a process server."
    },
    {"EndProcessServer", 
     (PyCFunction)dbgclient_EndProcessServer, METH_VARARGS, 
     "EndProcessServer(Server)\n"
     "  Requests that a process server be shut down."
    },
    {"StartProcessServer", 
     (PyCFunction)dbgclient_StartProcessServer, METH_VARARGS, 
     "StartProcessServer(Options)\n"
     "  Starts a process server.\n"
     "  Options:\n"
     "    Same as passed to the -t option on the command line.\n"
     "    ex. tcp:port=5555"
    },
    {"WaitForProcessServerEnd", 
     (PyCFunction)dbgclient_WaitForProcessServerEnd, METH_VARARGS, 
     "WaitForProcessServerEnd(Timeout)\n"
     "  Waits for a local process server to exit."
    },
    {"OutputServers", 
     (PyCFunction)dbgclient_OutputServers, METH_VARARGS, 
     "OutputServers(Machine, Flags)\n"
     "  Lists the servers running on a given computer.\n"
     "  Flags:\n"
     "    DEBUG_SERVERS_DEBUGGER\n"
     "    DEBUG_SERVERS_PROCESS"
    },
    {"StartServer", 
     (PyCFunction)dbgclient_StartServer, METH_VARARGS, 
     "StartServer(Options)\n"
     "  Start a debugger server.\n"
     "  Options:\n"
     "    Same as passed to the -server option on the command line.\n"
     "    ex. tcp:port=5555"
    },
    {"ConnectSession", 
     (PyCFunction)dbgclient_ConnectSession, METH_NOARGS, 
     "ConnectSession()\n"
     "  Joins the client to an existing debugger session."
    },
    {"EndSession", 
     (PyCFunction)dbgclient_EndSession, METH_VARARGS, 
     "EndSession(flags)\n"
     "  ends the current debugger session\n"
     "  flags is:\n"
     "    DEBUG_END_PASSIVE\n"
     "    DEBUG_END_ACTIVE_TERMINATE\n"
     "    DEBUG_END_ACTIVE_DETACH\n"
     "    DEBUG_END_REENTRANT\n"
     "    DEBUG_END_DISCONNECT"
    },
    {NULL, NULL}
};


PyTypeObject PyDebugClientType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugClient",    /*tp_name*/
    sizeof(PyDebugClientObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgclient_dealloc, /*tp_dealloc*/
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
    "DebugClient objects",     /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugClientObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugClient_methods,     /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgclient_init,            /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgclient_new,             /* tp_new */
    PyObject_Del,              /* tp_free */
};

