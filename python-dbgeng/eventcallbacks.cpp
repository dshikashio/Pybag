#include "eventcallbacks.h"
#include "pydbgeng.h"

PyDebugEventCallbacks::PyDebugEventCallbacks()
{
    count = 0;
    InterestMask = 0;

    PyBreakpointCB = NULL;
    PyExceptionCB = NULL;
    PyLoadModuleCB = NULL;
    PyUnloadModuleCB = NULL;
    PyCreateProcessCB = NULL;
    PyExitProcessCB = NULL;
    PyCreateThreadCB = NULL;
    PyExitThreadCB = NULL;
    PySystemErrorCB = NULL;
    PySessionStatusCB = NULL;
    PyChangeDebuggeeStateCB = NULL;
    PyChangeEngineStateCB = NULL;
    PyChangeSymbolStateCB = NULL;
}

PyDebugEventCallbacks::~PyDebugEventCallbacks()
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    Py_XDECREF(PyBreakpointCB);
    Py_XDECREF(PyExceptionCB);
    Py_XDECREF(PyLoadModuleCB);
    Py_XDECREF(PyUnloadModuleCB);
    Py_XDECREF(PyCreateProcessCB);
    Py_XDECREF(PyExitProcessCB);
    Py_XDECREF(PyCreateThreadCB);
    Py_XDECREF(PyExitThreadCB);
    Py_XDECREF(PySystemErrorCB);
    Py_XDECREF(PySessionStatusCB);
    Py_XDECREF(PyChangeDebuggeeStateCB);
    Py_XDECREF(PyChangeEngineStateCB);
    Py_XDECREF(PyChangeSymbolStateCB);
    PyGILState_Release(gstate);
}


HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::Breakpoint(PDEBUG_BREAKPOINT Bp)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;
    PyDebugBreakpointObject *obp;
    
    if (PyBreakpointCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    obp = (PyDebugBreakpointObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakpointType, NULL);
    if (obp == NULL)
        goto done;
    obp->bp = Bp;

    args = Py_BuildValue("(N)", obp);
    result = PyObject_CallObject(PyBreakpointCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);

    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }

    Py_DECREF(args);
    Py_XDECREF(result);
done:
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::Exception(PEXCEPTION_RECORD64 Exception, 
        ULONG FirstChance)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyExceptionRecordObject *exr;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyExceptionCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    exr = (PyExceptionRecordObject *)PyObject_CallObject(
            (PyObject *)&PyExceptionRecordType, NULL);
    if (exr == NULL)
        goto done;
    exr->exr = *Exception;

    args = Py_BuildValue("NI", exr, FirstChance);
    result = PyObject_CallObject(PyExceptionCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }
    
    Py_DECREF(args);
    Py_XDECREF(result);
done:
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::LoadModule(
    ULONG64 ImageFileHandle,
    ULONG64 BaseOffset,
    ULONG ModuleSize,
    PCSTR ModuleName,
    PCSTR ImageName,
    ULONG CheckSum,
    ULONG TimeDateStamp
    )
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;
    if (PyLoadModuleCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("KKIssII", ImageFileHandle, BaseOffset,
            ModuleSize, ModuleName, ImageName, CheckSum, TimeDateStamp);
    result = PyObject_CallObject(PyLoadModuleCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }
    
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::UnloadModule( PCSTR ImageBaseName, ULONG64 BaseOffset)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;
    if (PyUnloadModuleCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("sK", ImageBaseName, BaseOffset);
    result = PyObject_CallObject(PyUnloadModuleCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }
    
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::CreateProcess(
    ULONG64 ImageFileHandle,
    ULONG64 Handle,
    ULONG64 BaseOffset,
    ULONG ModuleSize,
    PCSTR ModuleName,
    PCSTR ImageName,
    ULONG CheckSum,
    ULONG TimeDateStamp,
    ULONG64 InitialThreadHandle,
    ULONG64 ThreadDataOffset,
    ULONG64 StartOffset
    )
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;
    if (PyCreateProcessCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("KKKIssIIKKK", ImageFileHandle, Handle, BaseOffset,
            ModuleSize, ModuleName, ImageName, CheckSum, TimeDateStamp,
            InitialThreadHandle, ThreadDataOffset, StartOffset);
    result = PyObject_CallObject(PyCreateProcessCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }
    
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::ExitProcess(ULONG ExitCode)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyExitProcessCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("(I)", ExitCode);
    result = PyObject_CallObject(PyExitProcessCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::CreateThread(ULONG64 Handle, ULONG64 DataOffset,
    ULONG64 StartOffset)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;

    gstate = PyGILState_Ensure();
    if (PyCreateThreadCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    args = Py_BuildValue("KKK", Handle, DataOffset, StartOffset);
    result = PyObject_CallObject(PyCreateThreadCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }

    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr; 
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::ExitThread(ULONG ExitCode)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyExitThreadCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("(I)", ExitCode);
    result = PyObject_CallObject(PyExitThreadCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }

    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::SystemError(ULONG Error, ULONG Level)
{
    PyGILState_STATE gstate;
    HRESULT hr = DEBUG_STATUS_NO_CHANGE;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PySystemErrorCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("II", Error, Level);
    result = PyObject_CallObject(PySystemErrorCB, args);

    if (result && result != Py_None)
        hr = PyInt_AsLong(result);
    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
        hr = DEBUG_STATUS_NO_CHANGE;
    }

    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return hr;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::SessionStatus(ULONG Status)
{
    PyGILState_STATE gstate;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PySessionStatusCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("(I)", Status);
    result = PyObject_CallObject(PySessionStatusCB, args);

    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
    }

    // Return value is ignored
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return DEBUG_STATUS_NO_CHANGE;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::ChangeDebuggeeState(ULONG Flags, ULONG64 Argument)
{
    PyGILState_STATE gstate;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyChangeDebuggeeStateCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("IK", Flags, Argument);
    result = PyObject_CallObject(PyChangeDebuggeeStateCB, args);

    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
    }

    // From help file:
    // The return value is ignored by the engine unless it indicates a 
    // remote procedure call error; in this case the client, with which
    // this IDebugEventCallbacks object is registered, is disabled.
    //
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return DEBUG_STATUS_NO_CHANGE;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::ChangeEngineState(ULONG Flags, ULONG64 Argument)
{
    PyGILState_STATE gstate;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyChangeEngineStateCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("IK", Flags, Argument);
    result = PyObject_CallObject(PyChangeEngineStateCB, args);

    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
    }

    // The return value is ignored by the engine unless it indicates a 
    // remote procedure call error; in this case the client, with which 
    // this IDebugEventCallbacks object is registered, is disabled.
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return DEBUG_STATUS_NO_CHANGE;
}

HRESULT STDMETHODCALLTYPE 
PyDebugEventCallbacks::ChangeSymbolState(ULONG Flags, ULONG64 Argument)
{
    PyGILState_STATE gstate;
    PyObject *args = NULL;
    PyObject *result = NULL;

    if (PyChangeSymbolStateCB == NULL)
        return DEBUG_STATUS_NO_CHANGE;

    gstate = PyGILState_Ensure();
    args = Py_BuildValue("IK", Flags, Argument);
    result = PyObject_CallObject(PyChangeSymbolStateCB, args);

    if (PyErr_Occurred() || result == NULL) {
        PyErr_Print();
    }

    // Return value is ignored
    Py_DECREF(args);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return DEBUG_STATUS_NO_CHANGE;
}

#define Py_Wrapper_CB(AddName, RemoveName, StorageObj, Mask) \
HRESULT PyDebugEventCallbacks::AddName(PyObject *cb)         \
{                                               \
    if (StorageObj != NULL)                     \
        return E_FAIL;                          \
    Py_XINCREF(cb);                             \
    InterestMask |= Mask;                       \
    StorageObj = cb;                            \
    return S_OK;                                \
}                                               \
                                                \
HRESULT PyDebugEventCallbacks::RemoveName(void) \
{                                               \
    Py_XDECREF(StorageObj);                     \
    StorageObj = NULL;                          \
    InterestMask &= ~Mask;                      \
    return S_OK;                                \
}

Py_Wrapper_CB(AddBreakpointCB, RemoveBreakpointCB,
        PyBreakpointCB, DEBUG_EVENT_BREAKPOINT)

Py_Wrapper_CB(AddExceptionCB, RemoveExceptionCB,
        PyExceptionCB, DEBUG_EVENT_EXCEPTION)

Py_Wrapper_CB(AddLoadModuleCB, RemoveLoadModuleCB,
        PyLoadModuleCB, DEBUG_EVENT_LOAD_MODULE)

Py_Wrapper_CB(AddUnloadModuleCB, RemoveUnloadModuleCB,
        PyUnloadModuleCB, DEBUG_EVENT_UNLOAD_MODULE)

Py_Wrapper_CB(AddCreateProcessCB, RemoveCreateProcessCB,
        PyCreateProcessCB, DEBUG_EVENT_CREATE_PROCESS)

Py_Wrapper_CB(AddExitProcessCB, RemoveExitProcessCB,
        PyExitProcessCB, DEBUG_EVENT_EXIT_PROCESS)

Py_Wrapper_CB(AddCreateThreadCB, RemoveCreateThreadCB,
        PyCreateThreadCB, DEBUG_EVENT_CREATE_THREAD)

Py_Wrapper_CB(AddExitThreadCB, RemoveExitThreadCB,
        PyExitThreadCB, DEBUG_EVENT_EXIT_THREAD)

Py_Wrapper_CB(AddSystemErrorCB, RemoveSystemErrorCB,
        PySystemErrorCB, DEBUG_EVENT_SYSTEM_ERROR)

Py_Wrapper_CB(AddSessionStatusCB, RemoveSessionStatusCB,
        PySessionStatusCB, DEBUG_EVENT_SESSION_STATUS)

Py_Wrapper_CB(AddChangeDebuggeeStateCB, RemoveChangeDebuggeeStateCB,
        PyChangeDebuggeeStateCB, DEBUG_EVENT_CHANGE_DEBUGGEE_STATE)

Py_Wrapper_CB(AddChangeEngineStateCB, RemoveChangeEngineStateCB,
        PyChangeEngineStateCB, DEBUG_EVENT_CHANGE_ENGINE_STATE)

Py_Wrapper_CB(AddChangeSymbolStateCB, RemoveChangeSymbolStateCB,
        PyChangeSymbolStateCB, DEBUG_EVENT_CHANGE_SYMBOL_STATE)

