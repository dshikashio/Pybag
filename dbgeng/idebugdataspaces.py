from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE
from comtypes           import COMError

from . import core as DbgEng
from . import exception

class DebugDataSpaces(object):
    def __init__(self, data):
        self._data = data
        exception.wrap_comclass(self._data)

    # IDebugDataSpaces

    def ReadVirtual(self, offset, size):
        nread = c_ulong()
        data = create_string_buffer(size)
        hr = self._data.ReadVirtual(offset, data, size, byref(nread))
        exception.check_err(hr)
        return data[:nread.value]

    def WriteVirtual(self, offset, data):
        if isinstance(data,  bytes):
            raise TypeError("data is not bytes")
        nwritten = c_ulong()
        hr = self._data.WriteVirtual(offset, data, len(data), byref(nwritten))
        exception.check_err(hr)
        return nwritten.value

    def SearchVirtual(self, offset, length, pattern, granularity):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.SearchVirtual()
        #exception.check_err(hr)
        #return match

    def ReadVirtualUncached(self, offset, size):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.ReadVirtualUncached()
        #exception.check_err(hr)
        #return data

    def WriteVirtualUncached(self, offset, data):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.WriteVirtualUncached()
        #exception.check_err(hr)

    def ReadPointersVirtual(self):
        raise exception.E_NOTIMPL_Error

    def WritePointersVirtual(self):
        raise exception.E_NOTIMPL_Error

    def ReadPhysical(self, offset, size):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.ReadPhysical()
        #exception.check_err(hr)
        #return data

    def WritePhysical(self, offset, data):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.WritePhysical()
        #exception.check_err(hr)

    def ReadControl(self):
        raise exception.E_NOTIMPL_Error

    def WriteControl(self):
        raise exception.E_NOTIMPL_Error

    def ReadIo(self):
        raise exception.E_NOTIMPL_Error

    def WriteIo(self):
        raise exception.E_NOTIMPL_Error

    def ReadMsr(self, msr):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.ReadMsr()
        #exception.check_err(hr)
        #return value

    def WriteMsr(self, msr, value):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.WriteMsr()
        #exception.check_err(hr)

    def ReadBusData(self):
        raise exception.E_NOTIMPL_Error

    def WriteBusData(self):
        raise exception.E_NOTIMPL_Error

    def CheckLowMemory(self):
        raise exception.E_NOTIMPL_Error

    def ReadDebuggerData(self):
        raise exception.E_NOTIMPL_Error

    def ReadProcessorSystemData(self):
        raise exception.E_NOTIMPL_Error

    # IDebugDataSpaces2

    def VirtualToPhysical(self, virtual):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.VirtualToPhysical()
        #exception.check_err(hr)
        #return physical

    def GetVirtualTranslationPhysicalOffsets(self):
        raise exception.E_NOTIMPL_Error

    def ReadHandleData(self):
        raise exception.E_NOTIMPL_Error

    def FillVirtual(self, offset, size, pattern):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.FillVirtual()
        #exception.check_err(hr)
        #return filled

    def FillPhysical(self, offset, size, pattern):
        raise exception.E_NOTIMPL_Error
        #hr = self._data.FillPhysical()
        #exception.check_err(hr)
        #return filled

    def QueryVirtual(self, offset):
        Info = DbgEng._MEMORY_BASIC_INFORMATION64()
        hr = self._data.QueryVirtual(offset, byref(Info))
        exception.check_err(hr)
        return Info

    # IDebugDataSpaces3

    def ReadImageNtHeaders(self):
        raise exception.E_NOTIMPL_Error

    def ReadTagged(self):
        raise exception.E_NOTIMPL_Error

    def StartEnumTagged(self):
        raise exception.E_NOTIMPL_Error

    def GetNextTagged(self):
        raise exception.E_NOTIMPL_Error

    def EndEnumTagged(self):
        raise exception.E_NOTIMPL_Error

    # IDebugDataSpaces4

    def GetOffsetInformation(self):
        raise exception.E_NOTIMPL_Error

    def GetNextDifferentlyValidOffsetVirtual(self):
        raise exception.E_NOTIMPL_Error

    def GetValidRegionVirtual(self):
        raise exception.E_NOTIMPL_Error

    def SearchVirtual2(self):
        raise exception.E_NOTIMPL_Error

    def ReadMultiByteStringVirtual(self):
        raise exception.E_NOTIMPL_Error

    def ReadMultiByteStringVirtualWide(self):
        raise exception.E_NOTIMPL_Error

    def ReadUnicodeStringVirtual(self):
        raise exception.E_NOTIMPL_Error

    def ReadUnicodeStringVirtualWide(self):
        raise exception.E_NOTIMPL_Error

    def ReadPhysical2(self):
        raise exception.E_NOTIMPL_Error

    def WritePhysical2(self):
        raise exception.E_NOTIMPL_Error
