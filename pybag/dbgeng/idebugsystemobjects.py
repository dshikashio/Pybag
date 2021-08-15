from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception


class DebugSystemObjects(object):
    def __init__(self, systemobj):
        self._sys = systemobj
        exception.wrap_comclass(self._sys)

    # IDebugSystemObjects

    def GetEventThread(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetEventThread()
        #exception.check_err(hr)
        #return tid

    def GetEventProcess(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetEventProcess()
        #exception.check_err(hr)
        #return pid

    def GetCurrentThreadId(self):
        tid = c_ulong()
        hr = self._sys.GetCurrentThreadId(byref(tid))
        exception.check_err(hr)
        return tid.value

    def SetCurrentThreadId(self, tid):
        hr = self._sys.SetCurrentThreadId(tid)
        exception.check_err(hr)

    def GetCurrentProcessId(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetCurrentProcessId()
        #exception.check_err(hr)
        #return pid

    def SetCurrentProcessId(self, pid):
        hr = self._sys.SetCurrentThreadId(pid)
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
        raise exception.E_NOTIMPL_Error

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

    def GetProcessIdsByIndex(self):
        raise exception.E_NOTIMPL_Error

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
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetCurrentProcessExecutableName()
        #exception.check_err(hr)
        #return name

    # IDebugSystemObjects2

    def GetCurrentProcessUpTime(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sys.GetCurrentProcessUpTime()
        #exception.check_err(hr)
        #return uptime

    def GetImplicitThreadDataOffset(self):
        raise exception.E_NOTIMPL_Error

    def SetImplicitThreadDataOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetImplicitProcessDataOffset(self):
        raise exception.E_NOTIMPL_Error

    def SetImplicitProcessDataOffset(self):
        raise exception.E_NOTIMPL_Error

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
