import os
from os.path import realpath, dirname
import struct
import itertools
import functools
import ctypes
#from ctypes import byref, WINFUNCTYPE, HRESULT, WinError

from simple_com import COMInterface, IDebugOutputCallbacksVtable
from dbgdef import *


class IDebugClient(COMInterface):
    _functions_ = {
        "QueryInterface": ctypes.WINFUNCTYPE(HRESULT, PVOID, PVOID)(0, "QueryInterface"),
        "AddRef": ctypes.WINFUNCTYPE(HRESULT)(1, "AddRef"),
        "Release": ctypes.WINFUNCTYPE(HRESULT)(2, "Release"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff538145%28v=vs.85%29.aspx
        "AttachKernel": ctypes.WINFUNCTYPE(HRESULT, ULONG, c_char_p)(3, "AttachKernel"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff541851%28v=vs.85%29.aspx
        "DetachProcesses": WINFUNCTYPE(HRESULT)(25, "DetachProcesses"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff543004%28v=vs.85%29.aspx
        "EndSession": WINFUNCTYPE(HRESULT, c_ulong)(26, "EndSession"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff556751%28v=vs.85%29.aspx
        "SetOutputCallbacks": ctypes.WINFUNCTYPE(HRESULT, c_void_p)(34, "SetOutputCallbacks"),
    }


class IDebugDataSpaces(COMInterface):
    _functions_ = {
        "QueryInterface": ctypes.WINFUNCTYPE(HRESULT, PVOID, PVOID)(0, "QueryInterface"),
        "AddRef": ctypes.WINFUNCTYPE(HRESULT)(1, "AddRef"),
        "Release": ctypes.WINFUNCTYPE(HRESULT)(2, "Release"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff554359%28v=vs.85%29.aspx
        "ReadVirtual": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG)(3, "ReadVirtual"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561468%28v=vs.85%29.aspx
        "WriteVirtual": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG)(4, "WriteVirtual"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff554310%28v=vs.85%29.aspx
        "ReadPhysical": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG)(10, "ReadPhysical"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561432%28v=vs.85%29.aspx
        "WritePhysical": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG)(11, "WritePhysical"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff553573%28v=vs.85%29.aspx
        "ReadIo": WINFUNCTYPE(HRESULT, ULONG, ULONG, ULONG, ULONG64, PVOID, ULONG, PULONG)(14, "ReadIo"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561402%28v=vs.85%29.aspx
        "WriteIo": WINFUNCTYPE(HRESULT, ULONG, ULONG, ULONG, ULONG64, PVOID, ULONG, PULONG)(15, "WriteIo"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff554289%28v=vs.85%29.aspx
        "ReadMsr": WINFUNCTYPE(HRESULT, ULONG, PULONG64)(16, "ReadMsr"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561424%28v=vs.85%29.aspx
        "WriteMsr": WINFUNCTYPE(HRESULT, ULONG, ULONG64)(17, "WriteMsr"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff553519%28v=vs.85%29.aspx
        "ReadBusData": WINFUNCTYPE(HRESULT, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG, PULONG)(18, "ReadBusData"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561371%28v=vs.85%29.aspx
        "WriteBusData": WINFUNCTYPE(HRESULT, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG, PULONG)(19, "WriteBusData"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff554326%28v=vs.85%29.aspx
        "ReadProcessorSystemData": WINFUNCTYPE(HRESULT, ULONG, ULONG, PVOID, ULONG, PULONG)(22, "ReadProcessorSystemData"),
    }


class IDebugDataSpaces2(COMInterface):
    _functions_ = dict(IDebugDataSpaces._functions_)
    _functions_.update({
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff560335%28v=vs.85%29.aspx
        "VirtualToPhysical": WINFUNCTYPE(HRESULT, ULONG64, PULONG64)(23, "VirtualToPhysical"),
    })


# https://msdn.microsoft.com/en-us/library/windows/hardware/ff550856%28v=vs.85%29.aspx
class IDebugSymbols(COMInterface):
    _functions_ = {
        "QueryInterface": ctypes.WINFUNCTYPE(HRESULT, PVOID, PVOID)(0, "QueryInterface"),
        "AddRef": ctypes.WINFUNCTYPE(HRESULT)(1, "AddRef"),
        "Release": ctypes.WINFUNCTYPE(HRESULT)(2, "Release"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff556798%28v=vs.85%29.aspx
        "SetSymbolOption": WINFUNCTYPE(HRESULT, ctypes.c_ulong)(6, "SetSymbolOption"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff547183%28v=vs.85%29.aspx
        "GetNameByOffset": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG, PULONG64)(7, "GetNameByOffset"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff548035%28v=vs.85%29.aspx
        "GetOffsetByName": WINFUNCTYPE(HRESULT, c_char_p, POINTER(ctypes.c_uint64))(8, "GetOffsetByName"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff547927%28v=vs.85%29.aspx
        "GetNumberModules": WINFUNCTYPE(HRESULT, LPDWORD, LPDWORD)(12, "GetNumberModules"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff547080%28v=vs.85%29.aspx
        "GetModuleByIndex": WINFUNCTYPE(HRESULT, DWORD, PULONG64)(13, "GetModuleByIndex"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff547146%28v=vs.85%29.aspx
        "GetModuleNames": WINFUNCTYPE(HRESULT, DWORD, c_uint64,
                                      PVOID, DWORD, LPDWORD, PVOID, DWORD, LPDWORD, PVOID, DWORD, LPDWORD)(16, "GetModuleNames"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff549408%28v=vs.85%29.aspx
        "GetTypeName": WINFUNCTYPE(HRESULT, ULONG64, ULONG, PVOID, ULONG, PULONG)(19, "GetTypeName"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff549376%28v=vs.85%29.aspx
        "GetTypeId": WINFUNCTYPE(HRESULT, ULONG64, c_char_p, PULONG)(20, "GetTypeId"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff549457%28v=vs.85%29.aspx
        "GetTypeSize": WINFUNCTYPE(HRESULT, ULONG64, ULONG, PULONG)(21, "GetTypeSize"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff546763%28v=vs.85%29.aspx
        "GetFieldOffset": WINFUNCTYPE(HRESULT, ULONG64, ULONG, c_char_p, PULONG)(22, "GetFieldOffset"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff549173%28v=vs.85%29.aspx
        "GetSymbolTypeId": WINFUNCTYPE(HRESULT, c_char_p, PULONG, PULONG64)(23, "GetSymbolTypeId"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff558815%28v=vs.85%29.aspx
        "StartSymbolMatch": WINFUNCTYPE(HRESULT, c_char_p, PULONG64)(36, "StartSymbolMatch"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff547856%28v=vs.85%29.aspx
        "GetNextSymbolMatch": WINFUNCTYPE(HRESULT, ULONG64, PVOID, ULONG, PULONG, PULONG64)(37, "GetNextSymbolMatch"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff543008%28v=vs.85%29.aspx
        "EndSymbolMatch": WINFUNCTYPE(HRESULT, ULONG64)(38, "EndSymbolMatch"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff554379%28v=vs.85%29.aspx
        "Reload": WINFUNCTYPE(HRESULT, c_char_p)(39, "Reload"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff556802%28v=vs.85%29.aspx
        "SetSymbolPath": WINFUNCTYPE(HRESULT, c_char_p)(41, "SetSymbolPath"),
    }


class IDebugSymbols2(COMInterface):
    _functions_ = dict(IDebugSymbols._functions_)
    _functions_.update({
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff546747%28v=vs.85%29.aspx
        "GetFieldName": WINFUNCTYPE(HRESULT, ULONG64, ULONG, ULONG, PVOID, ULONG, PULONG)(55, "GetFieldName"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff546771%28v=vs.85%29.aspx
        "GetFieldTypeAndOffset": WINFUNCTYPE(HRESULT, ULONG64, ULONG, c_char_p, PULONG, PULONG)(105, "GetFieldTypeAndOffset"),
    })


class IDebugSymbols3(COMInterface):
    _functions_ = dict(IDebugSymbols2._functions_)
    _functions_.update({
    })


class IDebugControl(COMInterface):
    _functions_ = {
        "QueryInterface": ctypes.WINFUNCTYPE(HRESULT, PVOID, PVOID)(0, "QueryInterface"),
        "AddRef": ctypes.WINFUNCTYPE(HRESULT)(1, "AddRef"),
        "Release": ctypes.WINFUNCTYPE(HRESULT)(2, "Release"),
        "GetInterrupt": ctypes.WINFUNCTYPE(HRESULT)(3, "GetInterrupt"),
        "SetInterrupt": ctypes.WINFUNCTYPE(HRESULT, ULONG)(4, "SetInterrupt"),
        "GetExecutionStatus": ctypes.WINFUNCTYPE(HRESULT, PULONG)(49, "GetExecutionStatus"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff543208%28v=vs.85%29.aspx
        "Execute": ctypes.WINFUNCTYPE(HRESULT, ULONG, c_char_p, ULONG)(66, "Execute"),
        # https://msdn.microsoft.com/en-us/library/windows/hardware/ff561229%28v=vs.85%29.aspx
        "WaitForEvent": WINFUNCTYPE(HRESULT, DWORD, DWORD)(93, "WaitForEvent")
    }




class LocalKernelDebuggerBase(object):
    DEBUG_DLL_PATH = None
    DRIVER_FILENAME = None
    DRIVER_RESOURCE = None
    # Will be used if '_NT_SYMBOL_PATH' is not set
    DEFAULT_SYMBOL_PATH  = "SRV*{0}\\symbols*http://msdl.microsoft.com/download/symbols".format(realpath(dirname(__file__)))
    SYMBOL_OPT = None

    def __init__(self, quiet=True):
        self.quiet = quiet
        self._output_string = ""
        self._output_callback = None
        self._load_debug_dll()
        self.DebugClient = self._do_debug_create()
        self._do_kernel_attach()
        self._ask_other_interface()
        self._setup_symbols_options()
        self.set_output_callbacks(self._standard_output_callback)
        self._wait_local_kernel_connection()


    def _do_kernel_attach(self):
        res = self.DebugClient.AttachKernel(DEBUG_ATTACH_LOCAL_KERNEL, None)
        if res:
            raise WinError(res)

    def _load_debug_dll(self):
        self.hmoduledbghelp = winproxy.LoadLibraryA(self.DEBUG_DLL_PATH + "dbghelp.dll")
        self.hmoduledbgeng = winproxy.LoadLibraryA(self.DEBUG_DLL_PATH + "dbgeng.dll")
        self.hmodulesymsrv = winproxy.LoadLibraryA(self.DEBUG_DLL_PATH + "symsrv.dll")

    def _do_debug_create(self):
        DebugClient = IDebugClient(0)

        DebugCreateAddr = winproxy.GetProcAddress(self.hmoduledbgeng, "DebugCreate")
        DebugCreate = WINFUNCTYPE(HRESULT, PVOID, PVOID)(DebugCreateAddr)
        DebugCreate(IID_IDebugClient, byref(DebugClient))
        return DebugClient

    def _ask_other_interface(self):
        DebugClient = self.DebugClient
        self.DebugDataSpaces = IDebugDataSpaces2(0)
        self.DebugSymbols = IDebugSymbols3(0)
        self.DebugControl = IDebugControl(0)

        DebugClient.QueryInterface(IID_IDebugDataSpaces2, ctypes.byref(self.DebugDataSpaces))
        DebugClient.QueryInterface(IID_IDebugSymbols3, ctypes.byref(self.DebugSymbols))
        DebugClient.QueryInterface(IID_IDebugControl, ctypes.byref(self.DebugControl))

    def _wait_local_kernel_connection(self):
        self.DebugControl.WaitForEvent(0, 0xffffffff)
        return True

    def _setup_symbols_options(self):
        try:
            symbol_path = os.environ['_NT_SYMBOL_PATH']
        except KeyError:
            symbol_path = self.DEFAULT_SYMBOL_PATH
        self.DebugSymbols.SetSymbolPath(symbol_path)
        self.DebugSymbols.SetSymbolOption(self.SYMBOL_OPT)

    def _standard_output_callback(self, x, y, msg):
        if not self.quiet:
            print msg,
        return 0

    def _init_string_output_callback(self):
        self._output_string = ""
        self.set_output_callbacks(self._string_output_callback)

    def _string_output_callback(self, x, y, msg):
        self._output_string += msg
        return 0

    def set_output_callbacks(self, callback):
        r"""| Register a new output callback, that must respect the interface of
           | :func:`IDebugOutputCallbacks::Output` `<https://msdn.microsoft.com/en-us/library/windows/hardware/ff550815%28v=vs.85%29.aspx>`_.
           | (see :file:`example\\output_demo.py`)
        """
        self._output_callback = callback
        my_idebugoutput_vtable = IDebugOutputCallbacksVtable.create_vtable(Output=callback)
        my_debugoutput_obj = ctypes.pointer(my_idebugoutput_vtable)
        res = self.DebugClient.SetOutputCallbacks(ctypes.addressof(my_debugoutput_obj))
        # Need to keep reference to these object else our output callback will be
        # garbage collected leading to crash
        # Update self.keep_alive AFTER the call to SetOutputCallbacks because
        # SetOutputCallbacks may call methods of the old my_debugoutput_obj
        self.keep_alive = [my_idebugoutput_vtable, my_debugoutput_obj]
        return res


    def detach(self):
        """End the Debugging session and detach the COM interface"""
        self.DebugClient.EndSession(DEBUG_END_PASSIVE)
        self.DebugClient.DetachProcesses()
        self.DebugClient.Release()
        del self.DebugClient
        self.DebugSymbols.Release()
        del self.DebugSymbols
        self.DebugControl.Release()
        del self.DebugControl
        self.DebugDataSpaces.Release()
        del self.DebugDataSpaces

