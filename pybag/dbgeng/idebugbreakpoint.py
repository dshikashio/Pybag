from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception

def fakeRelease(*args):
    # we don't want to reference the object
    return

class DebugBreakpoint(object):
    def __init__(self, bp):
        self._bp = bp
        exception.wrap_comclass(self._bp)
        self._bp.Release = fakeRelease

    # IDebugBreakpoint

    def GetId(self):
        id = c_ulong()
        hr = self._bp.GetId(byref(id))
        exception.check_err(hr)
        return id.value

    def GetType(self):
        breaktype = c_ulong()
        proctype  = c_ulong()
        hr = self._bp.GetType(byref(breaktype), byref(proctype))
        exception.check_err(hr)
        return (breaktype.value, proctype.value)

    def GetAdder(self):
        raise exception.E_NOTIMPL_Error

    def GetFlags(self):
        flags = c_ulong()
        hr = self._bp.GetFlags(byref(flags))
        exception.check_err(hr)
        return flags.value

    def AddFlags(self, flags):
        hr = self._bp.AddFlags(flags)
        exception.check_err(hr)

    def RemoveFlags(self, flags):
        hr = self._bp.RemoveFlags(flags)
        exception.check_err(hr)

    def SetFlags(self, flags):
        hr = self._bp.SetFlags(flags)
        exception.check_err(hr)

    def GetOffset(self):
        offset = c_ulonglong()
        hr = self._bp.GetOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def SetOffset(self, offset):
        hr = self._bp.SetOffset(offset)
        exception.check_err(hr)

    def GetDataParameters(self):
        size   = c_ulong()
        access = c_ulong()
        hr = self._bp.GetDataParameters(byref(size), byref(access))
        exception.check_err(hr)
        return (size.value, access.value)

    def SetDataParameters(self, size, access):
        hr = self._bp.SetDataParameters(size, access)
        exception.check_err(hr)

    def GetPassCount(self):
        count = c_ulong()
        hr = self._bp.GetPassCount(byref(count))
        exception.check_err(hr)
        return count.value

    def SetPassCount(self, count):
        hr = self._bp.SetPassCount(count)
        exception.check_err(hr)

    def GetCurrentPassCount(self):
        count = c_ulong()
        hr = self._bp.GetCurrentPassCount(byref(count))
        exception.check_err(hr)
        return count.value

    def GetMatchThreadId(self):
        tid = c_ulong()
        hr = self._bp.GetMatchThreadId(byref(tid))
        exception.check_err(hr)
        return tid.value

    def SetMatchThreadId(self, tid):
        hr = self._bp.SetMatchThreadId(tid)
        exception.check_err(hr)

    def GetCommand(self):
        size = c_ulong(1024)
        buffer = create_string_buffer(size.value)
        cmdsize = c_ulong()
        hr = self._bp.GetCommand(buffer, size, byref(cmdsize))
        exception.check_err(hr)
        buffer = buffer[:cmdsize.value]
        return buffer.rstrip(b'\x00').decode()

    def SetCommand(self, cmd):
        hr = self._bp.SetCommand(cmd)
        exception.check_err(hr)

    def GetOffsetExpression(self):
        size = c_ulong(256)
        expression = create_string_buffer(size.value)
        nread = c_ulong()
        hr = self._bp.GetOffsetExpression(expression, size, byref(nread))
        exception.check_err(hr)
        expression = expression[:nread.value]
        return expression.rstrip(b'\x00').decode()

    def SetOffsetExpression(self, expression):
        hr = self._bp.SetOffsetExpression(expression)
        exception.check_err(hr)

    def GetParameters(self):
        params = DbgEng._DEBUG_BREAKPOINT_PARAMETERS()
        hr = self._bp.GetParameters(byref(params))
        exception.check_err(hr)
        return params

    # IDebugBreakpoint2

    def GetCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def SetCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def GetOffsetExpressionWide(self):
        raise exception.E_NOTIMPL_Error

    def SetOffsetExpressionWide(self):
        raise exception.E_NOTIMPL_Error

    # IDebugBreakpoint3

    def GetGuid(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetGuid()
        #exception.check_err(hr)
        #return guid
