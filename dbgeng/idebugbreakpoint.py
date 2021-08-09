from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception

class DebugBreakpoint(object):
    def __init__(self, bp):
        self._bp = bp
        exception.wrap_comclass(self._bp)

    # IDebugBreakpoint

    def GetId(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetSymbolOptions()
        #exception.check_err(hr)
        #return id

    def GetType(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetType()
        #exception.check_err(hr)
        #return (breaktype, proctype)

    def GetAdder(self):
        raise exception.E_NOTIMPL_Error

    def GetFlags(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetFlags()
        #exception.check_err(hr)
        #return flags

    def AddFlags(self, flags):
        raise exception.E_NOTIMPL_Error
        hr = self._bp.AddFlags(flags)
        exception.check_err(hr)

    def RemoveFlags(self, flags):
        raise exception.E_NOTIMPL_Error
        hr = self._bp.RemoveFlags(flags)
        exception.check_err(hr)

    def SetFlags(self, flags):
        raise exception.E_NOTIMPL_Error
        hr = self._bp.SetFlags(flags)
        exception.check_err(hr)

    def GetOffset(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetOffset()
        #exception.check_err(hr)
        #return offset

    def SetOffset(self, offset):
        raise exception.E_NOTIMPL_Error
        hr = self._bp.SetOffset(offset)
        exception.check_err(hr)

    def GetDataParameters(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetDataParameters()
        #exception.check_err(hr)
        #return (size, access)

    def SetDataParameters(self, size, access):
        raise exception.E_NOTIMPL_Error
        hr = self._bp.SetOffset(offset)
        exception.check_err(hr)

    def GetPassCount(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetPassCount()
        #exception.check_err(hr)
        #return count

    def SetPassCount(self, count):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.SetPassCount(count)
        #exception.check_err(hr)

    def GetCurrentPassCount(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetPassCount()
        #exception.check_err(hr)
        #return count

    def GetMatchThreadId(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetMatchThreadId()
        #exception.check_err(hr)
        #return tid

    def SetMatchThreadId(self, tid):
        hr = self._bp.SetMatchThreadId(tid)
        exception.check_err(hr)

    def GetCommand(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetCommand()
        #exception.check_err(hr)
        #return cmd

    def SetCommand(self, cmd):
        hr = self._bp.SetCommand(cmd)
        exception.check_err(hr)

    def GetOffsetExpression(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetOffsetExpression()
        #exception.check_err(hr)
        #return expression

    def SetOffsetExpression(self, expression):
        hr = self._bp.SetOffsetExpression(expression)
        exception.check_err(hr)

    def GetParameters(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._bp.GetParameters()
        #exception.check_err(hr)
        #return parameters

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
