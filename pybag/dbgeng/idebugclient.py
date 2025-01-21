from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

import comtypes.gen.DbgEng as DbgEng
from . import DebugCreate
from . import exception
from . import string_to_wstr

from .idebugadvanced        import DebugAdvanced
from .idebugcontrol         import DebugControl
from .idebugdataspaces      import DebugDataSpaces
from .idebugregisters       import DebugRegisters
from .idebugsymbols         import DebugSymbols
from .idebugsystemobjects   import DebugSystemObjects

DEBUG_ONLY_THIS_PROCESS = 0x2
DEBUG_PROCESS = 0x1
DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP = 0x400


class DebugClient(object):
    def __init__(self, client=None):
        if client:
            self._cli = client
        else:
            self._cli = DebugCreate()
        exception.wrap_comclass(self._cli)
        self._proc_server_hndl = 0

    def Release(self):
        return
        cnt = self._cli.Release()
        if cnt == 0:
            self._cli = None
        return cnt

    # Convenience Methods

    def IDebugAdvanced(self):
        return DebugAdvanced(
            self._cli.QueryInterface(interface = DbgEng.IDebugAdvanced4))

    def IDebugControl(self):
        return DebugControl(
            self._cli.QueryInterface(interface = DbgEng.IDebugControl7))

    def IDebugDataSpaces(self):
        return DebugDataSpaces(
            self._cli.QueryInterface(interface = DbgEng.IDebugDataSpaces4))

    def IDebugRegisters(self):
        return DebugRegisters(
            self._cli.QueryInterface(interface = DbgEng.IDebugRegisters2))

    def IDebugSymbols(self):
        return DebugSymbols(
            self._cli.QueryInterface(interface = DbgEng.IDebugSymbols5))

    def IDebugSystemObjects(self):
        return DebugSystemObjects(
            self._cli.QueryInterface(interface = DbgEng.IDebugSystemObjects4))

    # IDebugClient

    def AttachKernel(self, options, flags=DbgEng.DEBUG_ATTACH_KERNEL_CONNECTION):
        if isinstance(options, str):
            options = options.encode()
        hr = self._cli.AttachKernel(flags, options)
        exception.check_err(hr)

    def GetKernelConnectionOptions(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetKernelConnectionOptions()
        #exception.check_err(hr)
        #return options

    def SetKernelConnectionOptions(self, options):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.SetKernelConnectionOptions()
        #exception.check_err(hr)

    def StartProcessServer(self, options):
        raise exception.E_NOTIMPL_Error
        #flags = DbgEng.DEBUG_CLASS_USER_WINDOWS
        #hr = self._cli.StartProcessServer(flags, options, NULL)
        #exception.check_err(hr)

    def ConnectProcessServer(self, options):
        server = c_ulonglong()
        hr = self._cli.ConnectProcessServer(options, byref(server))
        exception.check_err(hr)
        self._proc_server_hndl = server

    def DisconnectProcessServer(self):
        if self._proc_server_hndl:
            hr = self._cli.DisconnectProcessServer(self._proc_server_hndl)
            exception.check_err(hr)
        self._proc_server_hndl = 0

    def GetRunningProcessSystemIds(self):
        server = self._proc_server_hndl
        count = c_ulong()
        actual = c_ulong()
        hr = self._cli.GetRunningProcessSystemIds(server, None, 0, byref(count))
        exception.check_err(hr)
        count.value += 100
        pids = (count.value * c_ulong)()
        hr = self._cli.GetRunningProcessSystemIds(server, pids, count, byref(actual))
        exception.check_err(hr)
        return pids[:actual.value]

    def GetRunningProcessSystemIdByExecutableName(self, name):
        raise exception.E_NOTIMPL_Error
        #self._proc_server_hndl
        #hr = self._cli.GetRunningProcessSystemIdByExecutableName()
        #exception.check_err(hr)
        #return pid

    def GetRunningProcessDescription(self, pid):
        server = self._proc_server_hndl
        exename = create_string_buffer(1024)
        exesize = c_ulong()
        desc = create_string_buffer(1024)
        descsize = c_ulong()
        hr = self._cli.GetRunningProcessDescription(server, pid, 0,
                            exename, 1024, byref(exesize),
                            desc, 1024, byref(descsize))
        exception.check_err(hr)
        name = exename[:exesize.value].rstrip(b'\x00').decode()
        desc = desc[:descsize.value].rstrip(b'\x00').decode()
        return (name, desc)

    def AttachProcess(self, pid, flags=0):
        server = self._proc_server_hndl
        hr = self._cli.AttachProcess(server, pid, flags)
        exception.check_err(hr)

    def CreateProcess(self, cmdline, flags=DEBUG_ONLY_THIS_PROCESS):
        if isinstance(cmdline, str):
            cmdline = cmdline.encode()

        hr = self._cli.CreateProcess(self._proc_server_hndl, cmdline, flags)
        exception.check_err(hr)

    def CreateProcessAndAttach(self, cmdline, cflags=DEBUG_ONLY_THIS_PROCESS, aflags=0):
        ## XXX can create a process from cmdline and debug a different one with pid
        raise exception.E_NOTIMPL_Error
        #self._proc_server_hndl
        #hr = self._cli.CreateProcessAndAttach()
        #exception.check_err(hr)

    def GetProcessOptions(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetProcessOptions()
        #exception.check_err(hr)
        #return options

    def AddProcessOptions(self, options):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.AddProcessOptions()
        #exception.check_err(hr)

    def RemoveProcessOptions(self, options):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.RemoveProcessOptions()
        #exception.check_err(hr)

    def SetProcessOptions(self, options):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.SetProcessOptions()
        #exception.check_err(hr)

    def OpenDumpFile(self, dump_file):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.OpenDumpFile()
        #exception.check_err(hr)

    def WriteDumpFile(self, dump_file, options=DbgEng.DEBUG_DUMP_SMALL):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.WriteDumpFile()
        #exception.check_err(hr)

    def ConnectSession(self, flags=0, history=4096*10):
        hr = self._cli.ConnectSession(flags, history)
        exception.check_err(hr)

    def StartServer(self, options):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.StartServer()
        #exception.check_err(hr)

    def OutputServers(self, machine, flags):
        raise exception.E_NOTIMPL_Error
        #outctl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        #flags = DbgEng.DEBUG_SERVERS_DEBUGGER | DbgEng.DEBUG_SERVERS_PROCESS
        #hr = self._cli.OutputServers(outctl, machine, flags)
        #exception.check_err(hr)

    def TerminateProcesses(self):
        hr = self._cli.TerminateProcesses()
        exception.check_err(hr)

    def DetachProcesses(self):
        hr = self._cli.DetachProcesses()
        exception.check_err(hr)

    def EndSession(self, flags=DbgEng.DEBUG_END_ACTIVE_TERMINATE):
        hr = self._cli.EndSession(flags)
        exception.check_err(hr)

    def GetExitCode(self):
        exit_code = c_ulong()
        hr = self._cli.GetExitCode(byref(exit_code))
        exception.check_err(hr)
        return exit_code.value

    def DispatchCallbacks(self, timeout=DbgEng.WAIT_INFINITE):
        hr = self._cli.DispatchCallbacks(timeout)
        if hr == S_FALSE:
            raise exception.DbgEngTimeout("DispatchCallbacks timeout: {}".format(timeout))
        exception.check_err(hr)

    def ExitDispatch(self, client=None):
        if not client:
            client = self._cli
        hr = self._cli.ExitDispatch(client)
        exception.check_err(hr)

    def CreateClient(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.CreateClient()
        #exception.check_err(hr)
        #return client

    def GetInputCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetInputCallbacks()
        #exception.check_err(hr)
        #return callbacks

    def SetInputCallbacks(self, callbacks):
        hr = self._cli.SetInputCallbacks(callbacks)
        exception.check_err(hr)

    def GetOutputCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetOutputCallbacks()
        #exception.check_err(hr)
        #return callbacks

    def SetOutputCallbacks(self, callbacks):
        hr = self._cli.SetOutputCallbacks(callbacks)
        exception.check_err(hr)

    def GetOutputMask(self):
        mask = c_ulong()
        hr = self._cli.GetOutputMask(byref(mask))
        exception.check_err(hr)
        return mask.value

    def SetOutputMask(self, mask):
        hr = self._cli.SetOutputMask(mask)
        exception.check_err(hr)

    def GetOtherOutputMask(self, client):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetOtherOutputMask()
        #exception.check_err(hr)
        #return mask

    def SetOtherOutputMask(self, client, mask):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.SetOtherOutputMask()
        #exception.check_err(hr)

    def GetOutputWidth(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetOutputWidth()
        #exception.check_err(hr)
        #return width

    def SetOutputWidth(self, width):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.SetOutputWidth()
        #exception.check_err(hr)

    def GetOutputLinePrefix(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetOutputLinePrefix()
        #exception.check_err(hr)
        #return prefix

    def SetOutputLinePrefix(self, prefix):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.SetOutputLinePrefix()
        #exception.check_err(hr)

    def GetIdentity(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetIdentity()
        #exception.check_err(hr)
        #return identity

    def OutputIdentity(self, format="Identity: %s"):
        raise exception.E_NOTIMPL_Error
        #outctl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        #flags = 0
        #hr = self._cli.OutputIdentity(outctl, flags, format)
        #exception.check_err(hr)

    def GetEventCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetEventCallbacks()
        #exception.check_err(hr)
        #return callbacks

    def SetEventCallbacks(self, callbacks):
        hr = self._cli.SetEventCallbacks(callbacks)
        exception.check_err(hr)

    def FlushCallbacks(self):
        hr = self._cli.FlushCallbacks()
        exception.check_err(hr)

    # IDebugClient2

    def WriteDumpFile2(self, dump_file, qualifier, format_flags, comment=None):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.WriteDumpFile2()
        #exception.check_err(hr)

    def AddDumpInformationFile(self, info_file, type):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.AddDumpInformationFile()
        #exception.check_err(hr)

    def EndProcessServer(self):
        hr = self._cli.EndProcessServer(self._proc_server_hndl)
        exception.check_err(hr)
        self._proc_server_hndl = 0

    def WaitForProcessServerEnd(self, timeout=DbgEng.WAIT_INFINITE):
        hr = self._cli.WaitForProcessServerEnd(timeout)
        if hr == S_FALSE:
            raise exception.DbgEngTimeout("WaitForProcessServerEnd timeout: {}".format(timeout))
        exception.check_err(hr)

    def IsKernelDebuggerEnabled(self):
        hr = self._cli.IsKernelDebuggerEnabled()
        if hr == S_OK:
            return True
        else:
            return False

    def TerminateCurrentProcess(self):
        hr = self._cli.TerminateCurrentProcess()
        exception.check_err(hr)

    def DetachCurrentProcess(self):
        hr = self._cli.DetachCurrentProcess()
        exception.check_err(hr)

    def AbandonCurrentProcess(self):
        hr = self._cli.AbandonCurrentProcess()
        exception.check_err(hr)

    # IDebugClient3

    def GetRunningProcessSystemIdByExecutableNameWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetRunningProcessDescriptionWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def CreateProcessWide(self, cmdline, flags=DEBUG_ONLY_THIS_PROCESS):
        cmdline = string_to_wstr(cmdline)
        hr = self._cli.CreateProcessWide(self._proc_server_hndl, cmdline, flags)
        exception.check_err(hr)

    def CreateProcessAndAttachWide(self, *args):
        raise exception.E_NOTIMPL_Error

    # IDebugClient4

    def OpenDumpFileWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def WriteDumpFileWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def AddDumpInformationFileWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetNumberDumpFiles(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetNumberDumpFiles()
        #exception.check_err(hr)
        #return number

    def GetDumpFile(self, idx=0):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetDumpFile()
        #exception.check_err(hr)
        #return (name, handle, type)

    def GetDumpFileWide(self, *args):
        raise exception.E_NOTIMPL_Error

    # IDebugClient5

    def AttachKernelWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetKernelConnectionOptionsWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetKernelConnectionOptionsWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def StartProcessServerWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def ConnectProcessServerWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def StartServerWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def OutputServersWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetOutputCallbacksWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetOutputCallbacksWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetOutputLinePrefixWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetOutputLinePrefixWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetIdentityWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def OutputIdentityWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetEventCallbacksWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetEventCallbacksWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def CreateProcess2(self, cmdline, options, initial_dir, env):
        if isinstance(cmdline, str):
            cmdline = cmdline.encode()
        if isinstance(initial_dir, str):
            initial_dir = initial_dir.encode()
        if isinstance(env, str):
            env = env.encode()
        # env is \0 delineated key=value pairs
        # options is instance of DbgEng._DEBUG_CREATE_PROCESS_OPTIONS()
        hr = self._cli.CreateProcess2(self._proc_server_hndl, cmdline, byref(options), sizeof(options), initial_dir, env)
        exception.check_err(hr)

    def CreateProcess2Wide(self, *args):
        raise exception.E_NOTIMPL_Error

    def CreateProcessAndAttach2(self, cmdline, options, initial_dir, env, pid, aflags):
        raise exception.E_NOTIMPL_Error
        #self._proc_server_hndl
        #hr = self._cli.CreateProcessAndAttach2()
        #exception.check_err(hr)

    def CreateProcessAndAttach2Wide(self, *args):
        raise exception.E_NOTIMPL_Error

    def PushOutputLinePrefix(self, *args):
        raise exception.E_NOTIMPL_Error

    def PushOutputLinePrefixWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def PopOutputLinePrefix(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetNumberInputCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetNumberInputCallbacks()
        #exception.check_err(hr)
        #return count

    def GetNumberOutputCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetNumberOutputCallbacks()
        #exception.check_err(hr)
        #return count

    def GetNumberEventCallbacks(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._cli.GetNumberEventCallbacks()
        #exception.check_err(hr)
        #return count

    def GetQuitLockString(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetQuitLockString(self, *args):
        raise exception.E_NOTIMPL_Error

    def GetQuitLockStringWide(self, *args):
        raise exception.E_NOTIMPL_Error

    def SetQuitLockStringWide(self, *args):
        raise exception.E_NOTIMPL_Error

    # IDebugClient6

    def SetEventContextCallbacks(self, callbacks):
        hr = self._cli.SetEventContextCallbacks(callbacks)
        exception.check_err(hr)

    # IDebugClient7

    def SetClientContext(self, *args):
        raise exception.E_NOTIMPL_Error
