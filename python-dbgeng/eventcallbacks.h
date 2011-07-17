#ifndef _EVENTCALLBACKS_HPP
#define _EVENTCALLBACKS_HPP

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>


class PyDebugEventCallbacks : IDebugEventCallbacks
{
    LONG count;
    ULONG InterestMask;
    PyObject *PyBreakpointCB;
    PyObject *PyExceptionCB;
    PyObject *PyLoadModuleCB;
    PyObject *PyUnloadModuleCB;
    PyObject *PyCreateProcessCB;
    PyObject *PyExitProcessCB;
    PyObject *PyCreateThreadCB;
    PyObject *PyExitThreadCB;
    PyObject *PySystemErrorCB;
    PyObject *PySessionStatusCB;
    PyObject *PyChangeDebuggeeStateCB;
    PyObject *PyChangeEngineStateCB;
    PyObject *PyChangeSymbolStateCB;

public:
    PyDebugEventCallbacks();
    ~PyDebugEventCallbacks();

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID &InterfaceId, PVOID *Interface)
    {
        *Interface = NULL;
        if (InterfaceId == __uuidof(IDebugEventCallbacks))
            *Interface = this;
        else if (InterfaceId == __uuidof(IUnknown))
            *Interface = static_cast<IUnknown *>(this);
        else
            return E_NOINTERFACE;
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return InterlockedIncrement(&count);
    }

    ULONG STDMETHODCALLTYPE Release(void)
    {
        ULONG c = InterlockedDecrement(&count);
        if (c == 0)
            delete this;
        return c;
    }

    HRESULT STDMETHODCALLTYPE GetInterestMask(PULONG Mask)
    {
        if (Mask != NULL)
            *Mask = InterestMask;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Breakpoint(PDEBUG_BREAKPOINT Bp);
    HRESULT STDMETHODCALLTYPE Exception(PEXCEPTION_RECORD64 Exception, 
            ULONG FirstChance);
    HRESULT STDMETHODCALLTYPE LoadModule(
        ULONG64 ImageFileHandle,
        ULONG64 BaseOffset,
        ULONG ModuleSize,
        PCSTR ModuleName,
        PCSTR ImageName,
        ULONG CheckSum,
        ULONG TimeDateStamp
        );
    HRESULT STDMETHODCALLTYPE UnloadModule( PCSTR ImageBaseName, ULONG64 BaseOffset);
    HRESULT STDMETHODCALLTYPE CreateProcess(
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
        );
    HRESULT STDMETHODCALLTYPE ExitProcess(ULONG ExitCode);
    HRESULT STDMETHODCALLTYPE CreateThread(ULONG64 Handle, ULONG64 DataOffset,
        ULONG64 StartOffset);
    HRESULT STDMETHODCALLTYPE ExitThread(ULONG ExitCode);
    HRESULT STDMETHODCALLTYPE SystemError(ULONG Error, ULONG Level);
    HRESULT STDMETHODCALLTYPE SessionStatus(ULONG Status);
    HRESULT STDMETHODCALLTYPE ChangeDebuggeeState(ULONG Flags, ULONG64 Argument);
    HRESULT STDMETHODCALLTYPE ChangeEngineState(ULONG Flags, ULONG64 Argument);
    HRESULT STDMETHODCALLTYPE ChangeSymbolState(ULONG Flags, ULONG64 Argument);

#define Py_Wrapper_CBHeader(AddName, RemoveName)  \
    HRESULT AddName(PyObject *cb);          \
    HRESULT RemoveName(void);

    Py_Wrapper_CBHeader(AddBreakpointCB, RemoveBreakpointCB)
    Py_Wrapper_CBHeader(AddExceptionCB, RemoveExceptionCB)
    Py_Wrapper_CBHeader(AddLoadModuleCB, RemoveLoadModuleCB)
    Py_Wrapper_CBHeader(AddUnloadModuleCB, RemoveUnloadModuleCB)
    Py_Wrapper_CBHeader(AddCreateProcessCB, RemoveCreateProcessCB)
    Py_Wrapper_CBHeader(AddExitProcessCB, RemoveExitProcessCB)
    Py_Wrapper_CBHeader(AddCreateThreadCB, RemoveCreateThreadCB)
    Py_Wrapper_CBHeader(AddExitThreadCB, RemoveExitThreadCB)
    Py_Wrapper_CBHeader(AddSystemErrorCB, RemoveSystemErrorCB)
    Py_Wrapper_CBHeader(AddSessionStatusCB, RemoveSessionStatusCB)
    Py_Wrapper_CBHeader(AddChangeDebuggeeStateCB, RemoveChangeDebuggeeStateCB)
    Py_Wrapper_CBHeader(AddChangeEngineStateCB, RemoveChangeEngineStateCB)
    Py_Wrapper_CBHeader(AddChangeSymbolStateCB, RemoveChangeSymbolStateCB)
};

#endif


