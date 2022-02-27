from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception
from .dbgengstructs import DebugValue

class DebugRegisters(object):
    def __init__(self, reg):
        self._reg = reg
        exception.wrap_comclass(self._reg)

    # IDebugRegisters

    def GetNumberRegisters(self):
        number = c_ulong()
        hr = self._reg.GetNumberRegisters(byref(number))
        exception.check_err(hr)
        return number.value

    def GetDescription(self, index):
        nread  = c_ulong()
        name   = create_string_buffer(32)
        desc = DbgEng._DEBUG_REGISTER_DESCRIPTION()
        hr = self._reg.GetDescription(index, name, 32, byref(nread), byref(desc))
        exception.check_err(hr)
        name = name[:nread.value]
        name = name.rstrip(b'\x00')
        return (name,desc)

    def GetIndexByName(self, name):
        if isinstance(name, str):
            name = name.encode()
        index = c_ulong()
        hr = self._reg.GetIndexByName(name, byref(index))
        exception.check_err(hr)
        return index.value

    def GetValue(self, index):
        value = DbgEng._DEBUG_VALUE()
        hr = self._reg.GetValue(index, byref(value))
        exception.check_err(hr)
        return DebugValue(value).value

    def SetValue(self, index, value):
        (name,desc) = self.GetDescription(index)
        dv = DebugValue()
        dv.type = desc.Type
        dv.value = value
        hr = self._reg.SetValue(index, byref(dv.dv))
        exception.check_err(hr)

    def GetValues(self):
        raise exception.E_NOTIMPL_Error

    def SetValues(self):
        raise exception.E_NOTIMPL_Error

    def OutputRegisters(self):
        outctl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        flags = DbgEng.DEBUG_REGISTERS_DEFAULT
        hr = self._reg.OutputRegisters(outctl, flags)
        exception.check_err(hr)

    def GetInstructionOffset(self):
        offset = c_ulonglong()
        hr = self._reg.GetInstructionOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetStackOffset(self):
        offset = c_ulonglong()
        hr = self._reg.GetStackOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetFrameOffset(self):
        offset = c_ulonglong()
        hr = self._reg.GetFrameOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    # IDebugRegisters2

    def GetDescriptionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetIndexByNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberPseudoRegisters(self):
        raise exception.E_NOTIMPL_Error

    def GetPseudoDescription(self):
        raise exception.E_NOTIMPL_Error

    def GetPseudoDescriptionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetPseudoIndexByName(self):
        raise exception.E_NOTIMPL_Error

    def GetPseudoIndexByNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetPseudoValues(self):
        raise exception.E_NOTIMPL_Error

    def SetPseudoValues(self):
        raise exception.E_NOTIMPL_Error

    def GetValues2(self):
        raise exception.E_NOTIMPL_Error

    def SetValues2(self):
        raise exception.E_NOTIMPL_Error

    def OutputRegisters2(self, source, flags):
        outctl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        hr = self._reg.OutputRegisters2(outctl, source, flags)
        exception.check_err(hr)

    def GetInstructionOffset2(self):
        raise exception.E_NOTIMPL_Error

    def GetStackOffset2(self):
        raise exception.E_NOTIMPL_Error

    def GetFrameOffset2(self):
        raise exception.E_NOTIMPL_Error
