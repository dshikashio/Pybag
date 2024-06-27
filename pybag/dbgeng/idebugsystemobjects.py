from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception


class DebugSystemObjects(object):
    def __init__(self, systemobj):
        self._sys = systemobj
        exception.wrap_comclass(self._sys)

    def Release(self):
        cnt = self._sys.Release()
        if cnt == 0:
            self._sys = None
        return cnt

    # IDebugSystemObjects

    def GetEventThread(self):
        tid = c_ulong()
        hr = self._sys.GetEventThread(byref(tid))
        exception.check_err(hr)
        return tid.value

    def GetEventProcess(self):
        pid = c_ulong()
        hr = self._sys.GetEventProcess(byref(pid))
        exception.check_err(hr)
        return pid.value

    def GetCurrentThreadId(self):
        tid = c_ulong()
        hr = self._sys.GetCurrentThreadId(byref(tid))
        exception.check_err(hr)
        return tid.value

    def SetCurrentThreadId(self, tid):
        hr = self._sys.SetCurrentThreadId(tid)
        exception.check_err(hr)

    def GetCurrentProcessId(self):
        pid = c_ulong()
        hr = self._sys.GetCurrentProcessId(byref(pid))
        exception.check_err(hr)
        return pid.value

    def SetCurrentProcessId(self, pid):
        hr = self._sys.SetCurrentProcessId(pid)
        exception.check_err(hr)

    def GetNumberThreads(self):
        number = c_ulong()
        hr = self._sys.GetNumberThreads(byref(number))
        exception.check_err(hr)
        return number.value

    def GetTotalNumberThreads(self):
        raise exception.E_NOTIMPL_Error

    def GetThreadIdsByIndex(self, count=0):
        if count == 0:
            count = self.GetNumberThreads()
        ids = (c_ulong * count)()
        sysids = (c_ulong * count)()
        hr = self._sys.GetThreadIdsByIndex(0, count, ids, sysids)
        exception.check_err(hr)
        return (tuple(ids), tuple(sysids))

    def GetThreadIdByProcessor(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentThreadDataOffset(self):
        offset = c_ulonglong()
        hr = self._sys.GetCurrentThreadDataOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetThreadIdByDataOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentThreadTeb(self):
        offset = c_ulonglong()
        hr = self._sys.GetCurrentThreadTeb(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetThreadIdByTeb(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentThreadSystemId(self):
        sysid = c_ulong()
        hr = self._sys.GetCurrentThreadSystemId(byref(sysid))
        exception.check_err(hr)
        return sysid.value

    def GetThreadIdBySystemId(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentThreadHandle(self):
        raise exception.E_NOTIMPL_Error

    def GetThreadIdByHandle(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberProcesses(self):
        number = c_ulong()
        hr = self._sys.GetNumberProcesses(byref(number))
        exception.check_err(hr)
        return number.value

    def GetProcessIdsByIndex(self, count=0):
        if count == 0:
            count = self.GetNumberProcesses()
        ids = (c_ulong * count)()
        sysids = (c_ulong * count)()
        hr = self._sys.GetProcessIdsByIndex(0, count, ids, sysids)
        exception.check_err(hr)
        return (tuple(ids), tuple(sysids))

    def GetCurrentProcessDataOffset(self):
        offset = c_ulonglong()
        hr = self._sys.GetCurrentProcessDataOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetProcessIdByDataOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentProcessPeb(self):
        offset = c_longlong()
        hr = self._sys.GetCurrentProcessPeb(byref(offset))
        exception.check_err(hr)
        return offset.value

    def GetProcessIdByPeb(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentProcessSystemId(self):
        sysid = c_ulong()
        hr = self._sys.GetCurrentProcessSystemId(byref(sysid))
        exception.check_err(hr)
        return sysid.value

    def GetProcessIdBySystemId(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentProcessHandle(self):
        raise exception.E_NOTIMPL_Error

    def GetProcessIdByHandle(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentProcessExecutableName(self):
        buffer = create_string_buffer(512)
        size = c_ulong()
        exesize = c_ulong()
        hr = self._sys.GetCurrentProcessExecutableName(buffer, size, byref(exesize))
        exception.check_err(hr)
        buffer = buffer[:size.value]
        buffer = buffer.rstrip(b'\x00')
        return buffer

    # IDebugSystemObjects2

    def GetCurrentProcessUpTime(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetCurrentProcessUpTime()
        #exception.check_err(hr)
        #return uptime

    def GetImplicitThreadDataOffset(self):
        offset = c_ulonglong()
        hr = self._sys.GetImplicitThreadDataOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def SetImplicitThreadDataOffset(self, offset):
        hr = self._sys.SetImplicitThreadDataOffset(offset)
        exception.check_err(hr)

    def GetImplicitProcessDataOffset(self):
        offset = c_ulonglong()
        hr = self._sys.GetImplicitProcessDataOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def SetImplicitProcessDataOffset(self, offset):
        hr = self._sys.SetImplicitProcessDataOffset(offset)
        exception.check_err(hr)

    # IDebugSystemObjects3

    def GetEventSystem(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentSystemId(self):
        raise exception.E_NOTIMPL_Error

    def SetCurrentSystemId(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberSystems(self):
        raise exception.E_NOTIMPL_Error

    def GetSystemIdsByIndex(self):
        raise exception.E_NOTIMPL_Error

    def GetTotalNumberThreadsAndProcesses(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentSystemServer(self):
        raise exception.E_NOTIMPL_Error

    def GetSystemByServer(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentSystemServerName(self):
        raise exception.E_NOTIMPL_Error

    # IDebugSystemObjects4

    def GetCurrentProcessExecutableNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentSystemServerNameWide(self):
        raise exception.E_NOTIMPL_Error
