#ifndef _OUTPUTCALLBACKS_HPP
#define _OUTPUTCALLBACKS_HPP

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>

class PyDebugOutputCallbacks : IDebugOutputCallbacks
{
    LONG count;
    PyObject *OutputCallback;

public:
    PyDebugOutputCallbacks();
    ~PyDebugOutputCallbacks();

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID &InterfaceId, PVOID *Interface)
    {
        *Interface = NULL;
        if (InterfaceId == __uuidof(IDebugOutputCallbacks))
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

    // IDebugOutputCallbacks.

    // This method is only called if the supplied mask
    // is allowed by the clients output control.
    // The return value is ignored.
    HRESULT STDMETHODCALLTYPE Output(
        ULONG Mask,
        PCSTR Text);

    void SetCallback(PyObject *cb);
};

#endif
