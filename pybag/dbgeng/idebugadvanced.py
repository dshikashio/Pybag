from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception
from . import win32

class DebugAdvanced(object):
    def __init__(self, advanced):
        self._adv = advanced
        exception.wrap_comclass(self._adv)

    def Release(self):
        cnt = self._adv.Release()
        if cnt == 0:
            self._adv = None
        return cnt

    # IDebugAdvanced

    def GetThreadContext(self):
        # XXX - Check target bitness first
        ctx = win32.CONTEXT()
        hr = self._adv.GetThreadContext(byref(ctx), sizeof(ctx))
        exception.check_err(hr)
        return ctx

    def SetThreadContext(self, context):
        ctx = bytes(context)
        hr = self._adv.SetThreadContext(ctx, len(ctx))
        exception.check_err(hr)

    # IDebugAdvanced2

    def Request(self, requestid):
        if requestid == DbgEng.DEBUG_REQUEST_SOURCE_PATH_HAS_SOURCE_SERVER:
            hr = self._adv.Request(requestid, None, 0, None, 0, None)
            if hr == S_OK:
                return True
            if hr == S_FALSE:
                return False
            exception.check_err(hr)

        elif requestid == DbgEng.DEBUG_REQUEST_TARGET_EXCEPTION_CONTEXT:
            # XXX - Check target bitness first
            ctx = win32.CONTEXT()
            hr = self._adv.Request(requestid, None, 0, byref(ctx), sizeof(ctx), None)
            exception.check_err(hr)
            return ctx

        elif requestid == DbgEng.DEBUG_REQUEST_TARGET_EXCEPTION_THREAD:
            thread = c_ulong()
            hr = self._adv.Request(requestid, None, 0, byref(thread), 4, None)
            exception.check_err(hr)
            return thread.value

        elif requestid == DbgEng.DEBUG_REQUEST_TARGET_EXCEPTION_RECORD:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_GET_ADDITIONAL_CREATE_OPTIONS:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_SET_ADDITIONAL_CREATE_OPTIONS:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_GET_WIN32_MAJOR_MINOR_VERSIONS:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_READ_USER_MINIDUMP_STREAM:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_TARGET_CAN_DETACH:
            hr = self._adv.Request(requestid, None, 0, None, 0, None)
            if hr == S_OK:
                return True
            if hr == S_FALSE:
                return False
            exception.check_err(hr)

        elif requestid == DbgEng.DEBUG_REQUEST_SET_LOCAL_IMPLICIT_COMMAND_LINE:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_GET_CAPTURED_EVENT_CODE_OFFSET:
            pc = c_ulonglong()
            hr = self._adv.Request(requestid, None, 0, byref(pc), 8, None)
            exception.check_err(hr)
            return pc.value

        elif requestid == DbgEng.DEBUG_REQUEST_READ_CAPTURED_EVENT_CODE_STREAM:
            pass
        elif requestid == DbgEng.DEBUG_REQUEST_EXT_TYPED_DATA_ANSI:
            pass
        else:
            raise exception.E_INVALIDARG_Error

        raise exception.E_NOTIMPL_Error

    def GetSourceFileInformation(self):
        raise exception.E_NOTIMPL_Error

    def FindSourceFileAndToken(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolInformation(self):
        raise exception.E_NOTIMPL_Error

    def GetSystemObjectInformation(self):
        raise exception.E_NOTIMPL_Error

    # IDebugAdvanced3

    def GetSourceFileInformationWide(self):
        raise exception.E_NOTIMPL_Error

    def FindSourceFileAndTokenWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolInformationWide(self):
        raise exception.E_NOTIMPL_Error

    # IDebugAdvanced4

    def GetSymbolInformationWideEx(self):
        raise exception.E_NOTIMPL_Error
