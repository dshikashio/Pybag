import functools
import inspect

import comtypes.hresult as hresult
from comtypes import COMError

class DbgEngTimeout(Exception):
    pass

class DbgEngError(WindowsError):
    pass

class S_FALSE_Error(DbgEngError):
    pass

class E_FAIL_Error(DbgEngError):
    pass

class E_INVALIDARG_Error(DbgEngError):
    pass

class E_NOINTERFACE_Error(DbgEngError):
    pass

class E_OUTOFMEMORY_Error(DbgEngError):
    pass

class E_UNEXPECTED_Error(DbgEngError):
    pass

class E_NOTIMPL_Error(DbgEngError):
    pass

class E_PENDING_Error(DbgEngError):
    pass
 
E_PENDING = 0x8000000A

def check_err(hr):
    exceptions = {
        #hresult.S_OK           : "Success",
        hresult.S_FALSE         : S_FALSE_Error("Partial Success"),
        hresult.E_FAIL          : E_FAIL_Error("The operation could not be performed"),
        hresult.E_INVALIDARG    : E_INVALIDARG_Error("One of the arguments passed was invalid"),
        hresult.E_NOINTERFACE   : E_NOINTERFACE_Error("The object searched for was not found"),
        hresult.E_OUTOFMEMORY   : E_OUTOFMEMORY_Error("A memory allocation attempt failed"),
        hresult.E_UNEXPECTED    : E_UNEXPECTED_Error("The target was not accessible"),
        hresult.E_NOTIMPL       : E_NOTIMPL_Error("Not implemented"),
        E_PENDING               : E_PENDING_Error("Pending"),
    }

    if hr == hresult.S_OK:
        return None

    try:
        err = exceptions[hr]
    except KeyError:
        err = WindowsError(hr)

    raise err

def _commerr_wrapper(func):
    @functools.wraps(func)
    def _comerr_wrapper(*args, **kwargs):
        try:
            hr = func(*args, **kwargs)
        except COMError as ce:
            #print("Exception {}".format(ce.hresult))
            hr = ce.hresult
        return hr
    return _comerr_wrapper

def wrap_comclass(instance):
    for name, func in inspect.getmembers(instance, inspect.ismethod):
        # XXX - This might need to be a stronger filter
        if name.startswith('_'):
            continue
        setattr(instance, name, _commerr_wrapper(func))

