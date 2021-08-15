from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception

class DebugAdvanced(object):
    def __init__(self, advanced):
        self._adv = advanced
        exception.wrap_comclass(self._adv)

    # IDebugAdvanced

    def GetThreadContext(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._adv.GetThreadContext()
        #exception.check_err(hr)
        #return context

    def SetThreadContext(self, context):
        raise exception.E_NOTIMPL_Error
        #hr = self._adv.SetThreadContext()
        #exception.check_err(hr)
        #return context

    # IDebugAdvanced2

    def Request(self):
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
