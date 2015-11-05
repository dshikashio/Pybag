"""

Future


- basic blocks
- control flow graph?

Code
**- Disassemble
- Assemble
**- Assemble At (built in windbg thing)
- Function return address (GetReturnOffset???)
- Stack frame
- Local vars
- Function args
**- Call stack
**Call a function

- Inject DLL in target

Random
- Need an "args" class
-- Then can do arg[0], arg[1], etc
- For bp handlers and such for semantic sugar

Heap
- List process heaps
- Traverse heaps?

**PEB
Peb->ProcessHeap

Threads
**- TEBs
**- TIDs
- Stacks

Symbols
***- Lookup Name -> Addr
***- Lookup pattern (like windbg x)
***- Lookup Addr -> nearest name
- Synthetic modules

Filters
- Anything to add here?


Others
- Socket stalk, if ws2_32 gets loaded, record all
    create / close / read / write

- Reg watch
- Network watch
- File watch


NEED
- Polish
-- Easily load into windbg
-- Easily run windbg extensions from standalone
"""

import collections
import itertools
import re
import string
import struct
import subprocess
import sys
import time
import threading
import Queue

import distorm3
import pefile
import pydbgeng

__all__ = ['Kerneldbg',
           'Userdbg',
           'Windbg',
           'hexdump',
           'pydbgeng']

class WorkItem(object):
    def __init__(self, task, timeout):
        self.task = task
        self.timeout = timeout

def EventThread(Dbg, Ev, WorkQ):
        Dbg._client         = pydbgeng.DebugCreate()
        Dbg._advanced       = pydbgeng.DebugAdvanced(Dbg._client)
        Dbg._control        = pydbgeng.DebugControl(Dbg._client)
        Dbg._registers      = pydbgeng.DebugRegisters(Dbg._client)
        Dbg._dataspaces     = pydbgeng.DebugDataSpaces(Dbg._client)
        Dbg._symbols        = pydbgeng.DebugSymbols(Dbg._client)
        Dbg._systems        = pydbgeng.DebugSystems(Dbg._client)

        Dbg.reg    = Registers(Dbg._registers)
        Dbg.mod    = Modules(Dbg._dataspaces, Dbg._symbols)
        Dbg.events = Events(Dbg._client)

        Dbg.breakpoints = Breakpoints(Dbg._control)
        Dbg.events.breakpoint(Dbg.breakpoints)
            
        Ev.set()

        while True:
            item = WorkQ.get(True)
            if item.task == 'WaitForEvent':
                try:
                    Dbg._control.WaitForEvent(item.timeout)
                except:
                    pass
            elif item.task == 'DispatchCallbacks':
                try:
                    Dbg._client.DispatchCallbacks(item.timeout)
                except:
                    pass

            WorkQ.task_done()
            Ev.set()

def hexdump(data, vaddr=0, width=16):
    def printable(c):
        if c in string.whitespace and c != ' ':
            c = '.'
        elif c not in string.printable:
            c = '.'
        return c
    i = 0
    while i < len(data):
        line = data[i:i+width]
        h = ["%02x" % ord(x) for x in line]
        a = ["%c" % printable(x) for x in line]
        pad = "   "*((width+1) - len(line))
        print "%07x: %s%s%s" % (i+vaddr, ' '.join(h), pad, ''.join(a))
        i += width
    print ""

def str_memory_state(val):
    s = []
    if val & 0x1000:
        s.append('MEM_COMMIT')
    if val & 0x2000:
        s.append('MEM_RESERVE')
    if val & 0x10000:
        s.append('MEM_FREE')
    return ' | '.join(s)

def str_memory_protect(val):
    prot_tab = { 0x10: 'PAGE_EXECUTE',
                 0x20: 'PAGE_EXECUTE_READ',
                 0x40: 'PAGE_EXECUTE_READWRITE',
                 0x80: 'PAGE_EXECUTE_WRITECOPY',
                 0x01: 'PAGE_NOACCESS',
                 0x02: 'PAGE_READONLY',
                 0x04: 'PAGE_READWRITE',
                 0x08: 'PAGE_WRITECOPY',
                 0x100: 'PAGE_GUARD',
                 0x200: 'PAGE_NOCACHE',
                 0x400: 'PAGE_WRITECOMBINE',
               }
    return ' | '.join(desc
                for mask, desc in prot_tab.items()
                if val & mask)

def str_memory_type(val):
    s = []
    if val & 0x20000:
        s.append('MEM_PRIVATE')
    if val & 0x40000:
        s.append('MEM_MAPPED')
    if val & 0x1000000:
        s.append('MEM_IMAGE')
    return ' | '.join(s)

def str_memory_info(info):
    fmt  = "%015x - %015x : %08x"
    fmt2 = "\t%s %s %s"
    return '\n'.join([fmt  % (info[0], info[0] + info[3], info[3]),
                      fmt2 % (str_memory_state(info[4]), 
                              str_memory_protect(info[5]), 
                              str_memory_type(info[6]))])

def bp_wrap(cls, fn):
    def bp_handler(bp):
        return fn(bp, cls)
    return bp_handler

class Events(object):
    def __init__(self, cli):
        self._client = cli

    def _ignore_event(self, event):
        self._client.RemoveEventCallback(event)

    def _catch_event(self, event, handler):
        if handler is None:
            handler = self._ev_break
        self._client.SetEventCallback(event, handler)

    @staticmethod
    def _ev_break(*args):
        return pydbgeng.DEBUG_STATUS_BREAK

    @staticmethod
    def _ev_breakpoint(*args):
        print "DEBUG_EVENT_BREAKPOINT"
        bp = args[0]
        try:
            tid = bp.GetMatchThreadId()
        except pydbgeng.E_NOINTERFACE_Error:
            tid = "ANY"
        print "        ID: ", bp.GetId()
        print "     Flags: ", bp.GetFlags()
        print "    Offset: ", bp.GetOffset()
        print "    OffExp: ", bp.GetOffsetExpression()
        print "  PassOrig: ", bp.GetPassCount()
        print "  Pass Cnt: ", bp.GetCurrentPassCount()
        print "    Thread: ", tid
        print "   BP Type: ", bp.GetType()[0]
        #return pydbgeng.DEBUG_STATUS_NO_CHANGE
        return pydbgeng.DEBUG_STATUS_GO

    def breakpoint(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_breakpoint
        self._catch_event(pydbgeng.DEBUG_EVENT_BREAKPOINT, handler)

    def nobreakpoint(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_BREAKPOINT)

    @staticmethod
    def _ev_exception(*args):
        print "DEBUG_EVENT_EXCEPTION"
        if args[1]:
            print "  First Chance"
        else:
            print "  Second Chance"
        print "  Address: %x" % args[0].ExceptionAddress
        print "     Code: %x" % args[0].ExceptionCode
        print "    Flags: %x" % args[0].ExceptionFlags
        print "   Record: %x" % args[0].ExceptionRecord
        print "   Params: %x" % args[0].NumberParameters

    def exception(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exception
        self._catch_event(pydbgeng.DEBUG_EVENT_EXCEPTION, handler)

    def noexception(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_EXCEPTION)

    @staticmethod
    def _ev_loadmodule(*args):
        print "DEBUG_EVENT_LOAD_MODULE"
        print "  ImageFileHandle: ", args[0]
        print "  BaseOffset: ", args[1]
        print "  ModuleSize: ", args[2]
        print "  ModuleName: ", args[3]
        print "   ImageName: ", args[4]
        print "    CheckSum: ", args[5]
        print "    TimeDate: ", args[6]
        return pydbgeng.DEBUG_STATUS_BREAK

    def module_load(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_loadmodule
        self._catch_event(pydbgeng.DEBUG_EVENT_LOAD_MODULE, handler)

    def nomodule_load(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_LOAD_MODULE)

    @staticmethod
    def _ev_unloadmodule(*args):
        print "DEBUG_EVENT_UNLOAD_MODULE"
        print "  ImageBaseName: ", args[0]
        print "     BaseOffset: ", args[1]
        return pydbgeng.DEBUG_STATUS_BREAK

    def unload_module(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_unloadmodule
        self._catch_event(pydbgeng.DEBUG_EVENT_UNLOAD_MODULE, handler)

    def nounload_module(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_UNLOAD_MODULE)

    @staticmethod
    def _ev_createthread(*args):
        print "DEBUG_EVENT_CREATE_THREAD"
        print "       Handle: ", args[0]
        print "   DataOffset: %x" % args[1]
        print "  StartOffset: %x" % args[2]
        return pydbgeng.DEBUG_STATUS_BREAK

    def create_thread(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_createthread
        self._catch_event(pydbgeng.DEBUG_EVENT_CREATE_THREAD, handler)

    def nocreate_thread(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_CREATE_THREAD)

    @staticmethod
    def _ev_createprocess(*args):
        print "DEBUG_EVENT_CREATE_PROCESS"
        print "  ImageFileHandle: ", args[0]
        print "  Handle: ", args[1]
        print "  BaseOffset: ", args[2]
        print "  ModuleSize: ", args[3]
        print "  ModuleName: ", args[4]
        print "   ImageName: ", args[5]
        print "    CheckSum: ", args[6]
        print "    TimeDate: ", args[7]
        print "  InitialThreadHandle: ", args[8]
        print "  ThreadDataOffset: ", args[9]
        print "  StartOffset: ", args[10]
        return pydbgeng.DEBUG_STATUS_BREAK

    def create_process(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_createprocess
        self._catch_event(pydbgeng.DEBUG_EVENT_CREATE_PROCESS, handler)

    def nocreate_process(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_CREATE_PROCESS)

    @staticmethod
    def _ev_exitprocess(*args):
        print "DEBUG_EVENT_EXIT_PROCESS"
        print "  ExitCode: ", args[0]
        return pydbgeng.DEBUG_STATUS_BREAK

    def exit_process(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exitprocess
        self._catch_event(pydbgeng.DEBUG_EVENT_EXIT_PROCESS, handler)

    def noexit_process(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_EXIT_PROCESS)

    @staticmethod
    def _ev_exitthread(*args):
        print "DEBUG_EVENT_EXIT_THREAD"
        print "  ExitCode: ", args[0]
        return pydbgeng.DEBUG_STATUS_BREAK

    def exit_thread(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exitthread
        self._catch_event(pydbgeng.DEBUG_EVENT_EXIT_THREAD, handler)

    def noexit_thread(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_EXIT_THREAD)

    @staticmethod
    def _ev_systemerror(*args):
        print "DEBUG_EVENT_SYSTEM_ERROR"
        print "Error: ", args[0]
        print "Level: ", args[1]

    def system_error(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_systemerror
        self._catch_event(pydbgeng.DEBUG_EVENT_SYSTEM_ERROR, handler)

    def nosystem_error(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_SYSTEM_ERROR)

    @staticmethod
    def _ev_sessionstatus(*args):
        print "DEBUG_EVENT_SESSION_STATUS"
        sdict = {pydbgeng.DEBUG_SESSION_ACTIVE: 
                    "DEBUG_SESSION_ACTIVE",
                pydbgeng.DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE:
                    "DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE",
                pydbgeng.DEBUG_SESSION_END_SESSION_ACTIVE_DETACH:
                    "DEBUG_SESSION_END_SESSION_ACTIVE_DETACH",
                pydbgeng.DEBUG_SESSION_END_SESSION_PASSIVE:
                    "DEBUG_SESSION_END_SESSION_PASSIVE",
                pydbgeng.DEBUG_SESSION_END: 
                    "DEBUG_SESSION_END",
                pydbgeng.DEBUG_SESSION_REBOOT: 
                    "DEBUG_SESSION_REBOOT",
                pydbgeng.DEBUG_SESSION_HIBERNATE: 
                    "DEBUG_SESSION_HIBERNATE",
                pydbgeng.DEBUG_SESSION_FAILURE: 
                    "DEBUG_SESSION_FAILURE"}
        print "Status: ", sdict[args[0]]

    def session_status(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_sessionstatus
        self._catch_event(pydbgeng.DEBUG_EVENT_SESSION_STATUS, handler)

    def nosession_status(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_SESSION_STATUS)

    @staticmethod
    def _ev_debuggeestate(*args):
        print "DEBUG_EVENT_CHANGE_DEBUGGEE_STATE"
        fdict = {pydbgeng.DEBUG_CDS_ALL:       'DEBUG_CDS_ALL',
                pydbgeng.DEBUG_CDS_REGISTERS: 'DEBUG_CDS_REGISTERS',
                pydbgeng.DEBUG_CDS_DATA:      'DEBUG_CDS_DATA'}
        print "Flags: ", fdict[args[0]]
        if args[0] == pydbgeng.DEBUG_CDS_DATA:
            adict = {pydbgeng.DEBUG_DATA_SPACE_VIRTUAL: 
                        'DEBUG_DATA_SPACE_VIRTUAL',
                    pydbgeng.DEBUG_DATA_SPACE_PHYSICAL: 
                        'DEBUG_DATA_SPACE_PHYSICAL',
                    pydbgeng.DEBUG_DATA_SPACE_CONTROL: 
                        'DEBUG_DATA_SPACE_CONTROL',
                    pydbgeng.DEBUG_DATA_SPACE_IO: 
                        'DEBUG_DATA_SPACE_IO',
                    pydbgeng.DEBUG_DATA_SPACE_MSR: 
                        'DEBUG_DATA_SPACE_MSR',
                    pydbgeng.DEBUG_DATA_SPACE_BUS_DATA: 
                        'DEBUG_DATA_SPACE_BUS_DATA'}
            print "Argument: ", adict[args[1]]
        else:
            print "Argument: ", args[1]

    def debuggee_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_debuggeestate
        self._catch_event(pydbgeng.DEBUG_EVENT_CHANGE_DEBUGGEE_STATE, handler)

    def nodebuggee_state(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_CHANGE_DEBUGGEE_STATE)

    @staticmethod
    def _ev_enginestate(*args):
        print "DEBUG_EVENT_CHANGE_ENGINE_STATE"
        fdict = {pydbgeng.DEBUG_CES_CURRENT_THREAD: 
                    "DEBUG_CES_CURRENT_THREAD",
                pydbgeng.DEBUG_CES_EFFECTIVE_PROCESSOR: 
                    "DEBUG_CES_EFFECTIVE_PROCESSOR",
                pydbgeng.DEBUG_CES_BREAKPOINTS: 
                    "DEBUG_CES_BREAKPOINTS",
                pydbgeng.DEBUG_CES_CODE_LEVEL: 
                    "DEBUG_CES_CODE_LEVEL",
                pydbgeng.DEBUG_CES_EXECUTION_STATUS: 
                    "DEBUG_CES_EXECUTION_STATUS",
                pydbgeng.DEBUG_CES_ENGINE_OPTIONS: 
                    "DEBUG_CES_ENGINE_OPTIONS",
                pydbgeng.DEBUG_CES_LOG_FILE: 
                    "DEBUG_CES_LOG_FILE",
                pydbgeng.DEBUG_CES_RADIX: 
                    "DEBUG_CES_RADIX",
                pydbgeng.DEBUG_CES_EVENT_FILTERS: 
                    "DEBUG_CES_EVENT_FILTERS",
                pydbgeng.DEBUG_CES_PROCESS_OPTIONS: 
                    "DEBUG_CES_PROCESS_OPTIONS",
                pydbgeng.DEBUG_CES_EXTENSIONS: 
                    "DEBUG_CES_EXTENSIONS",
                pydbgeng.DEBUG_CES_SYSTEMS: 
                    "DEBUG_CES_SYSTEMS",
                pydbgeng.DEBUG_CES_ASSEMBLY_OPTIONS: 
                    "DEBUG_CES_ASSEMBLY_OPTIONS",
                pydbgeng.DEBUG_CES_EXPRESSION_SYNTAX: 
                    "DEBUG_CES_EXPRESSION_SYNTAX",
                pydbgeng.DEBUG_CES_TEXT_REPLACEMENTS: 
                    "DEBUG_CES_TEXT_REPLACEMENTS"}
        try:
            print "Flags: ", fdict[args[0]]
        except:
            pass
        if args[0] == pydbgeng.DEBUG_CES_EXECUTION_STATUS:
            adict = {pydbgeng.DEBUG_STATUS_NO_DEBUGGEE: 
                        "DEBUG_STATUS_NO_DEBUGGEE",
                    pydbgeng.DEBUG_STATUS_BREAK: 
                        "DEBUG_STATUS_BREAK",
                    pydbgeng.DEBUG_STATUS_STEP_INTO: 
                        "DEBUG_STATUS_STEP_INTO",
                    pydbgeng.DEBUG_STATUS_STEP_BRANCH: 
                        "DEBUG_STATUS_STEP_BRANCH",
                    pydbgeng.DEBUG_STATUS_STEP_OVER: 
                        "DEBUG_STATUS_STEP_OVER",
                    pydbgeng.DEBUG_STATUS_GO_NOT_HANDLED: 
                        "DEBUG_STATUS_GO_NOT_HANDLED",
                    pydbgeng.DEBUG_STATUS_GO_HANDLED: 
                        "DEBUG_STATUS_GO_HANDLED",
                    pydbgeng.DEBUG_STATUS_GO: 
                        "DEBUG_STATUS_GO",
                    pydbgeng.DEBUG_STATUS_IGNORE_EVENT: 
                        "DEBUG_STATUS_IGNORE_EVENT",
                    pydbgeng.DEBUG_STATUS_RESTART_REQUESTED: 
                        "DEBUG_STATUS_RESTART_REQUESTED", 
                    pydbgeng.DEBUG_STATUS_NO_CHANGE:
                        "DEBUG_STATUS_NO_CHANGE"}
            a = args[1] & 0xffffffff
            b = args[1] & pydbgeng.DEBUG_STATUS_INSIDE_WAIT
            if b:
                b = "DEBUG_STATUS_INSIDE_WAIT"
            c = args[1] & pydbgeng.DEBUG_STATUS_WAIT_TIMEOUT
            if c:
                c = "DEBUG_STATUS_WAIT_TIMEOUT"
            print "Argument: ", adict[a], b, c
        else:
            print "Argument: ", args[1]

    def engine_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_enginestate
        self._catch_event(pydbgeng.DEBUG_EVENT_CHANGE_ENGINE_STATE, handler)

    def noengine_state(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_CHANGE_ENGINE_STATE)

    @staticmethod
    def _ev_symbolstate(*args, **kw):
        print "DEBUG_EVENT_CHANGE_SYMBOL_STATE"
        fdict = {pydbgeng.DEBUG_CSS_LOADS:          "DEBUG_CSS_LOADS",
                pydbgeng.DEBUG_CSS_UNLOADS:        "DEBUG_CSS_UNLOADS",
                pydbgeng.DEBUG_CSS_SCOPE:          "DEBUG_CSS_SCOPE",
                pydbgeng.DEBUG_CSS_PATHS:          "DEBUG_CSS_PATHS",
                pydbgeng.DEBUG_CSS_SYMBOL_OPTIONS: "DEBUG_CSS_SYMBOL_OPTIONS",
                pydbgeng.DEBUG_CSS_TYPE_OPTIONS:   "DEBUG_CSS_TYPE_OPTIONS"}
        print "Flags: ", fdict[args[0]]
        print "Argument: ", args[1]

    def symbol_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_symbolstate
        self._catch_event(pydbgeng.DEBUG_EVENT_CHANGE_SYMBOL_STATE, handler)

    def nosymbol_state(self):
        self._ignore_event(pydbgeng.DEBUG_EVENT_CHANGE_SYMBOL_STATE)

class Registers(collections.Sequence, collections.Mapping):
    def __init__(self, DebugRegistersObj=None):
        self.__dict__['_reg'] = None
        if DebugRegistersObj:
            self._set_DebugRegister(DebugRegistersObj)

    def _set_DebugRegister(self, DebugRegistersObj):
        self._reg = DebugRegistersObj

    def _get_register(self, name):
        try:
            return self._get_register_index(self._reg.GetIndexByName(name))
        except pydbgeng.DbgEngError:
            raise AttributeError(name)

    def _set_register(self, name, value):
        try:
            self._set_register_index(self._reg.GetIndexByName(name), value)
        except pydbgeng.DbgEngError:
            raise AttributeError(name)

    def _get_register_index(self, index):
        try:
            return self._reg.GetValue(index)
        except pydbgeng.DbgEngError:
            raise IndexError(index)
        except TypeError:
            return 0

    def _set_register_index(self, index, value):
        if not isinstance(value, int):
            raise ValueError('value must be of type int')
        try:
            self._reg.SetValue(index, value)
        except pydbgeng.DbgEngError:
            raise IndexError(index)

    def __len__(self):
        return self._reg.GetNumberRegisters()

    def __iter__(self):
        return iter(self.register_list())

    def __contains__(self, item):
        try:
            self.__getitem__(item)
            return True
        except:
            return False

    def __getitem__(self, key):
        if isinstance(key, str):
            try:
                return self._get_register(key)
            except AttributeError:
                raise KeyError(key)
        else:
            return self._get_register_index(key)

    def __setitem__(self, key, value):
        if isinstance(key, str):
            try:
                self._set_register(key)
            except AttributeError:
                raise KeyError(key)
        else:
            self._set_register_index(key, value)

    def __getattr__(self, name):
        if name in self.__dict__:
            return self.__dict__[name]
        else:
            return self._get_register(name) 

    def __setattr__(self, name, value):
        if name in self.__dict__:
            self.__dict__[name] = value
        else:
            self._set_register(name, value) 

    def output(self, source=None):
        if not source:
            source = pydbgeng.DEBUG_REGSRC_DEBUGGEE
        self._reg.OutputRegisters2(source)

    def register_list(self):
        itemlist = []
        for i in range(0, len(self)):
            try:
                name = self._reg.GetDescription(i)[0]
                val  = self._get_register_index(i)
                itemlist.append((name, val))
            except AttributeError:
                pass
        return itemlist

    def get_sp(self):
        return self._reg.GetStackOffset()

    def get_pc(self):
        return self._reg.GetInstructionOffset()

    def get_bp(self):
        return self._reg.GetFrameOffset()


class Module(object):
    def __init__(self, addr, size, fnread, name=""):
        self.addr = addr
        self.size = size
        self.fnread = fnread
        self.name = name
        self._pe = pefile.PE(addr, size, fnread)

    def entry_point(self):
        self._pe.OPTIONAL_HEADER.AddressOfEntryPoint

    def export_list(self):
        return self._pe.DIRECTORY_ENTRY_EXPORT.symbols

    def exports(self):
        for e in self.export_list():
            if e.forwarder:
                name = "%s -> %s" % (e.name, e.forwarder)
                addr = 0
            else:
                if e.address == self.addr:
                    continue
                if e.name:
                    name = "%s" % e.name
                elif e.ordinal:
                    name = "ORD(%d)" % e.ordinal
                else:
                    name = "[UNKNOWN]"
                addr = e.address
            print "%015x  %s" % (addr, name)

    def import_list(self):
        # XXX - Getting the right values here?
        return self._pe.DIRECTORY_ENTRY_IMPORT

    def imports(self):
        for i in self.import_list():
            for fn in i.imports:
                if fn.import_by_ordinal:
                    name = "%s!ORD(%d)" % (i.dll, fn.ordinal)
                else:
                    name = "%s!%s" % (i.dll, fn.name)
                print "%015x  %s" % (fn.address, name)

    def section_list(self):
        return self._pe.sections

    def sections(self):
        for s in self.section_list():
            print "SECTION HEADER"
            print "%16s name" % s.Name.strip('\x00')
            print "%16x virtual size" % s.Misc_VirtualSize
            print "%16x virtual address (%016x to %016x" % (s.VirtualAddress,
                    (s.VirtualAddress + self.addr),
                    (s.VirtualAddress + self.addr + s.Misc_VirtualSize))
            print "%16x size of raw data" % s.SizeOfRawData
            print "%16x flags" % s.Characteristics
            if s.IMAGE_SCN_CNT_CODE:
                print "%16s Code" % ' '
            if s.IMAGE_SCN_CNT_INITIALIZED_DATA:
                print "%16s Initialized Data" % ' '
            if s.IMAGE_SCN_CNT_UNINITIALIZED_DATA:
                print "%16s Uninitialized Data" % ' '
            prot = []
            if s.IMAGE_SCN_MEM_EXECUTE:
                prot.append("Execute")
            if s.IMAGE_SCN_MEM_READ:
                prot.append("Read")
            if s.IMAGE_SCN_MEM_WRITE:
                prot.append("Write")
            if s.IMAGE_SCN_MEM_SHARED:
                prot.append("Shared")
            print "%16s %s" % (' ', ' '.join(prot))

    def addr_to_section(self, addr):
        for s in self.section_list():
            start = s.VirtualAddress + self.addr
            end = start + s.SizeOfRawData
            if addr >= start and addr <= end:
                return s
        return None

    def file_header(self):
        # XXX - See dumpbin
        pass

    def optional_header(self):
        # XXX - See dumpbin
        pass

    def function_list(self):
        """function_list() -> list of all functions"""
        # grab exports that point into an executable section
        flist = []
        for e in self.export_list():
            if e.forwarder:
                continue
            if e.address == self.addr:
                continue

            if e.name:
                name = "%s" % e.name
            elif e.ordinal:
                name = "ORD(%d)" % e.ordinal
            else:
                name = "[UNKNOWN]"

            s = self.addr_to_section(e.address)
            if not s:
                continue
            if s.IMAGE_SCN_MEM_EXECUTE:
                flist.append((e.address, name))
        return flist

    def functions(self):
        """functions() -> print all module functions"""
        for f in self.function_list():
            print "%015x  %s" % f

    def seh_status(self):
        """seh_status() -> module seh status"""
        if self._pe.OPTIONAL_HEADER.DllCharacteristics & 0x400:
            return (False, "NO_SEH Flag Enabled")
        # IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG = 10
        ldconf = self._pe.OPTIONAL_HEADER.DATA_DIRECTORY[10]
        #ldconf.Size
        #ldconf.VirtualAddress
        return (False, "LOAD_CONFIG Size 0x%x" % ldconf.Size)
        

class Modules(collections.Sequence, collections.Mapping):
    def __init__(self, DebugDataSpacesObj=None, DebugSymbolsObj=None):
        self.__dict__['_data'] = None
        self.__dict__['_sym'] = None
        if DebugDataSpacesObj:
            self._set_DebugDataSpaces(DebugDataSpacesObj)
        if DebugSymbolsObj:
            self._set_DebugSymbols(DebugSymbolsObj)

    def _set_DebugDataSpaces(self, DebugDataSpacesObj):
        self._data = DebugDataSpacesObj

    def _set_DebugSymbols(self, DebugSymbolsObj):
        self._sym = DebugSymbolsObj

    def _get_module(self, name):
        try:
            addr = self._sym.GetModuleByModuleName(name, 0)[1]
            return self.get_module(addr)
        except pydbgeng.DbgEngError:
            raise AttributeError(name)

    def _get_module_index(self, index):
        try:
            addr = self._sym.GetModuleByIndex(i)
            return self.get_module(addr)
        except pydbgeng.DbgEngError:
            raise IndexError(index)
        except TypeError:
            return 0

    def __len__(self):
        return self._sym.GetNumberModules()[0]

    def __iter__(self):
        return iter(self.modules())

    def __contains__(self, item):
        try:
            self.__getitem__(item)
            return True
        except:
            return False

    def __getitem__(self, key):
        if isinstance(key, str):
            try:
                return self._get_module(key)
            except AttributeError:
                raise KeyError(key)
        else:
            return self._get_module_index(key)

    def __getattr__(self, name):
        if name in self.__dict__:
            return self.__dict__[name]
        else:
            return self._get_module(name)

    def addr_to_name(self, addr):
        for name,params in self.modules():
            if (params.Base <= addr and 
                    addr <= (params.Base + params.Size)):
                return name
        return ""

    def get_module(self, addr):
        size = self._sym.GetModuleParameters(addr).Size
        return Module(addr, size, self._data.ReadVirtual, self.addr_to_name(addr))

    def module_names(self):
        """module_names() -> list of all module names"""
        return [self._sym.GetModuleNames(i)[1] for i in range(len(self))]

    def modules(self):
        """modules() -> list of (name, modparams) tuples"""
        return [
            (self._sym.GetModuleNames(i),
             self._sym.GetModuleParameters(self._sym.GetModuleByIndex(i)))
            for i in range(len(self))]

    def seh_status(self):
        """seh_status() -> list of all modules and SEH status"""
        for name in self.module_names():
            (status, reason) = self[name].seh_status()
            print "%s %s" % (name, reason)


class Breakpoints(collections.Mapping, collections.Callable):
    def __init__(self, DebugControlObj):
        self._control = DebugControlObj
        self._bp = {}

    def __len__(self):
        return len(self._bp)

    def __iter__(self):
        return iter(self._bp.keys())

    def __contains__(self, id):
        return id in self._bp

    def __getitem__(self, id):
        return self._bp[id]

    def __call__(self, *args):
        rawbp = args[0]
        try:
            bpfn = self.__getitem__(rawbp.GetId())
        except KeyError:
            bpfn = None
        if bpfn:
            ret = bpfn(rawbp)
        else:
            ret = pydbgeng.DEBUG_STATUS_NO_CHANGE

        if rawbp.GetFlags() & pydbgeng.DEBUG_BREAKPOINT_ONE_SHOT:
            self._bp.pop(rawbp.GetId(), None)
            #self._control.RemoveBreakpoint(rawbp)

        return ret

    def set(self, expr, handler=None, type=None, windbgcmd=None, oneshot=False,
                  passcount=None, threadid=None, size=None, access=None):
        # XXX - would be nice to check if bp with expr already exists
        # if so, then enable existing, else add
        if type is None:
            type = pydbgeng.DEBUG_BREAKPOINT_CODE
        if isinstance(expr, int) or isinstance(expr, long):
            expr = "0x%x" % expr
        bp = self._control.AddBreakpoint(type)
        id = bp.GetId()
        bp.SetOffsetExpression(str(expr))
        if windbgcmd:
            pass
        if passcount is not None:
            bp.SetPassCount(passcount)
        if threadid:
            bp.SetMatchThreadId(threadid)
        if type == pydbgeng.DEBUG_BREAKPOINT_DATA:
            bp.SetDataParameters(size, access)
        if oneshot:
            bp.AddFlags(pydbgeng.DEBUG_BREAKPOINT_ONE_SHOT)
        self._bp[id] = handler
        if not bp.GetFlags() & pydbgeng.DEBUG_BREAKPOINT_DEFERRED:
            self.enable(id)
        return id

    def enable(self, id):
        bp = self._control.GetBreakpointById(id)
        bp.AddFlags(pydbgeng.DEBUG_BREAKPOINT_ENABLED)

    def disable(self, id):
        bp = self._control.GetBreakpointById(id)
        bp.RemoveFlags(pydbgeng.DEBUG_BREAKPOINT_ENABLED)

    def _remove(self, bp, id):
        self._control.RemoveBreakpoint(bp)
        self._bp.pop(id, None)

    def _remove_stale(self, id):
        self._bp.pop(id, None)

    def remove(self, id):
        bp = self._control.GetBreakpointById(id)
        self._remove(bp, id)

    def enable_all(self):
        for bpid in self:
            self.enable(bpid)

    def disable_all(self):
        for bpid in self:
            self.disable(bpid)

    def remove_all(self):
        for bpid in self:
            self.remove(bpid)


class Windbg(object):
    def __init__(self, client=None, standalone=False):
        if not client and standalone:
            self._ev = threading.Event()
            self._q = Queue.Queue()
            self._thread = threading.Thread(target=EventThread, 
                                            args=(self, self._ev, self._q))
            self._thread.daemon = True
            self._thread.start()
            self._ev.wait(30)
            if not self._ev.is_set():
                raise RuntimeError

        else:
            if client:
                self._client = client
            else:
                self._client = pydbgeng.DebugCreate()

            self._advanced      = pydbgeng.DebugAdvanced(self._client)
            self._control       = pydbgeng.DebugControl(self._client)
            self._registers     = pydbgeng.DebugRegisters(self._client)
            self._dataspaces    = pydbgeng.DebugDataSpaces(self._client)
            self._symbols       = pydbgeng.DebugSymbols(self._client)
            self._systems       = pydbgeng.DebugSystems(self._client)
            
            self.reg    = Registers(self._registers)
            self.mod    = Modules(self._dataspaces, self._symbols)

            self.events = Events(self._client)
            self.breakpoints = Breakpoints(self._control)
            self.events.breakpoint(self.breakpoints)

    def is64bit(self):
        """is64bit() -> True/False whether target is 64bit"""
        ## XXX - This isn't reliable for wow64 processes
        return self._control.IsPointer64Bit()

    def cmd(self, cmd):
        """cmd(windbg input) -> execute a windbg console command"""
        self._control.Execute(cmd)

    def quiet(self):
        """quiet() -> shut dbgeng up"""
        self._client.SetOutputMask(0)

    def verbose(self, flags=pydbgeng.DEBUG_OUTPUT_NORMAL):
        """verbose(flags=DEBUG_OUTPUT_NORMAL) -> allow dbgeng to output"""
        self._client.SetOutputMask(flags)

    def read(self, addr, len=1):
        """read(addr,len) -> read len bytes from addr"""
        return self._dataspaces.ReadVirtual(addr, len)

    def readptr(self, addr, count=1):
        """readptr(addr,count=1) -> read and return size_t dwords from addr"""
        if self.is64bit():
            width = 8
            fmt = "Q"
        else:
            width = 4
            fmt = "I"
        data = self.read(addr, count * width)
        return struct.unpack("<" + fmt*count, data)

    def write(self, addr, data):
        """write(addr,data) -> write data to addr"""
        return self._dataspaces.WriteVirtual(addr, data)

    def search(self, pattern):
        """search(pattern) -> regex search memory"""
        matches = []
        prog = re.compile(pattern, re.M | re.S)
        for m in self.memory_list():
            # if memory commited
            # and not page guarded
            if m[4] & 0x1000 and not (m[5] & 0x100):
                index = 0
                data = self.read(m[0], m[3])
                while True:
                    s = prog.search(data, index)
                    if not s:
                        break
                    matches.append(m[0] + s.start())
                    print "%016x" % (m[0] + s.start())
                    index = s.end()
        return matches

    def memory_list(self):
        """memory_list() -> Return a list of all process memory regions"""
        try:
            memlist = []
            addr = 0
            while True:
                info = self._dataspaces.QueryVirtual(addr)
                memlist.append(info)
                addr = info[0] + info[3] 
        except pydbgeng.E_NOINTERFACE_Error:
            pass
        return memlist

    def memory(self):
        """memory() -> Print virtual address space"""
        for info in self.memory_list():
            print str_memory_info(info)

    def address(self, addr):
        """address(addr) -> Return information about an addr"""
        print str_memory_info(self._dataspaces.QueryVirtual(addr))

    def register_list(self):
        """register_list() -> Returns a list of all registers"""
        return self.reg.register_list()

    def pc(self):
        """pc() -> print program counter"""
        addr = self.reg.get_pc()
        instr = self.instruction_at(addr)
        name = self.get_name_by_offset(addr)
        bytes = "%02x"*instr[1]
        bytes = bytes % struct.unpack("B"*instr[1], self.read(addr, instr[1]))
        print "  %s" % name
        print "%015x %-15s  %s\n" % (addr, bytes, instr[0])

    def r(self):
        """r() -> registers() alias"""
        self.registers()

    def registers(self):
        """registers() -> print current register state"""
        self._registers.OutputRegisters()
        print ""
        self.pc()

    def handle_list(self, max_handle=0x10000):
        """handle_list() -> Return a list of all handles"""
        hlist = []
        for i in range(4, max_handle, 4):
            try:
                hlist.append((i,
                        self._dataspaces.ReadHandleData(i,
                            pydbgeng.DEBUG_HANDLE_DATA_TYPE_TYPE_NAME),
                        self._dataspaces.ReadHandleData(i,
                            pydbgeng.DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME)))
            except WindowsError:
                pass
        return hlist

    def handles(self, max_handle=0x10000):
        """handles() -> print current handles"""
        for h in self.handle_list(max_handle):
            print "%08x : %s : %s" % h

    def assemble(self, addr, instr):
        """assemble(addr, instr) -> assemble instr at addr"""
        self._control.Assemble(addr, instr)

    def instruction_at(self, addr):
        """instruction_at(addr) -> Return instruction at addr"""
        if self.is64bit():
            arch = distorm3.Decode64Bits
        else:
            arch = distorm3.Decode32Bits

        if not addr:
            addr = self.reg.get_pc()

        data = self.read(addr, 15)

        try:
            (t1,width,disasm,t2) = distorm3.Decode(addr, data, arch)[0]
        except ValueError, IndexError:
            return None

        if width > 0:
            return (disasm, width)
        else:
            return None

    def _disasm_str(self, addr):
        instr = self.instruction_at(addr)  
        if instr is None:
            width = 1
            bytes = "%02x" % ord(self.read(addr, 1))
            pinstr = "??"
            valid = False
        else:
            width = instr[1]
            bytes = "%02x"*width
            bytes = bytes % struct.unpack("B"*width,
                                            self.read(addr, width))
            pinstr = instr[0]
            valid = True
        fmt = "%015x %-15s %s" % (addr, bytes, pinstr)
        return (fmt, width, valid)

    def _disasm(self, addr=None, count=10):
        """_disasm(addr=pc, count=10) -> disassemble as code to str"""
        if not addr:
            addr = self.reg.get_pc()
        for i in range(count):
            (fmt, width, valid) = self._disasm_str(addr)
            print fmt
            addr += width

    def _disasm_back(self, addr, nbytes):
        seq = []
        for i in range(0,nbytes+1):
            start_addr = addr - i
            curseq = []
            status = True
            while start_addr < addr:
                (fmt, width, valid) = self._disasm_str(start_addr)
                if valid is False:
                    status = False
                curseq.append(fmt)
                start_addr += width

            if start_addr != addr:
                status = False
            else:
                (fmt, width, valid) = self._disasm_str(start_addr)
                curseq.append(fmt)

            if status == True:
                seq.append('\n'.join(curseq))
        return seq

    def disasm(self, addr=None, count=10):
        """disasm(addr=pc, count=10) -> disassemble as code"""
        if not addr:
            addr = self.reg.get_pc()
        for i in range(count):
            instr,addr = self._control.Disassemble(addr)
            print instr.strip()

    def exec_status(self):
        status_map = {pydbgeng.DEBUG_STATUS_BREAK : "BREAK",
                      pydbgeng.DEBUG_STATUS_GO : "GO",
                      pydbgeng.DEBUG_STATUS_STEP_BRANCH : "STEP_BRANCH",
                      pydbgeng.DEBUG_STATUS_STEP_INTO : "STEP_INTO", 
                      pydbgeng.DEBUG_STATUS_STEP_OVER : "STEP_OVER",
                      pydbgeng.DEBUG_STATUS_NO_DEBUGGEE : "NO_DEBUGGEE"}
        st = self._control.GetExecutionStatus()
        return status_map[st]

    def stepi(self, count=1):
        """stepi(count=1) -> step into count instructions"""
        self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_STEP_INTO)
        for tmp in range(count):
            self.wait()

    def stepo(self, count=1):
        """stepo(count=1) -> step over count instructions"""
        self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_STEP_OVER)
        for tmp in range(count):
            self.wait()

    def stepbr(self, count=1):
        """stepbr(count=1) -> step branch count instructions"""
        self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_STEP_BRANCH)
        for tmp in range(count):
            self.wait()

    def stepto(self, addr, max=None):
        """stepto(addr, [max]) -> step into until addr or max instructions"""
        for i in itertools.count():
            if max and i >= max:
                return False
            if self.reg.get_pc() == addr:
                return True
            self.stepi()

    def stepout(self):
        """stepout() -> step over until return addr"""
        addr = self._control.GetReturnOffset()
        while self.reg.get_pc() != addr:
            self.stepo()

    def trace(self, registers=True, count=1):
        """trace() -> step pc and print context"""
        for i in range(count):
            self.stepi()
            if registers:
                self.registers()
            else:
                self.pc()
        self.dispatch_events()

    def traceto(self, addr, registers=True, max=None):
        """traceto(addr, [max]) -> trace until addr or max instructions"""
        for i in itertools.count():
            if max and i >= max:
                return False
            if self.reg.get_pc() == addr:
                return True
            self.trace(registers)


    def go(self, timeout=pydbgeng.INFINITE):
        """go(timeout) -> Continue execution"""
        try:
            self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_GO)
        except pydbgeng.E_UNEXPECTED_Error:
            pass
        return self.wait(timeout)

    def go_handled(self, timeout=pydbgeng.INFINITE):
        """go_handled(timeout) -> Continue with exception handled"""
        self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_GO_HANDLED)
        return self.wait(timeout)

    def go_nothandled(self, timeout=pydbgeng.INFINITE):
        self._control.SetExecutionStatus(pydbgeng.DEBUG_STATUS_GO_NOT_HANDLED)
        return self.wait(timeout)

    def _reset_ctx(self):
        try:
            self._systems.GetCurrentThreadId()
        except pydbgeng.E_UNEXPECTED_Error:
            self._systems.SetCurrentThreadId(0)

    def _threaded_wait(self, timeout=pydbgeng.INFINITE):
        def wait_event(ctrl, timeout):
            ctrl.WaitForEvent(timeout)
        if timeout in (-1, pydbgeng.INFINITE):
            # python having issues on 32-bit
            #timeout = 0xfffffff
            pass
        else:
            timeout *= 1000
        t = threading.Thread(target=wait_event, args=(self._control, timeout))
        t.daemon = True
        t.start()
        try:
            for i in itertools.repeat(1, timeout / 1000):
                t.join(1)
                if not t.isAlive():
                    break
        except KeyboardInterrupt:
            pass

        if t.isAlive():
            self._control.SetInterrupt()
            raise pydbgeng.DbgEngTimeout()
        else:
            return True

    def dispatch_events(self):
        if not self._worker_wait('DispatchCallbacks', 1):
            self._client.ExitDispatch(self._client)
            return False
        else:
            return True

    def _worker_wait(self, msg, timeout=pydbgeng.INFINITE):
        if timeout in (-1, pydbgeng.INFINITE):
            # python having issues on 32-bit
            #timeout = 0xfffffff
            pass
        else:
            timeout *= 1000

        item = WorkItem(msg, timeout)
        self._ev.clear()
        self._q.put(item)
        try:
            for i in itertools.repeat(1, timeout / 1000):
                if self._ev.is_set():
                    break
                self._ev.wait(1)
        except KeyboardInterrupt:
            pass

        if self._ev.is_set():
            return True
        else:
            return False

    def wait(self, timeout=pydbgeng.INFINITE):
        """wait(timeout=INFINITE) -> Wait timeout seconds for an event"""
        if not self._worker_wait('WaitForEvent', timeout):
            self._control.SetInterrupt()
            return False
        else:
            return False

    def module_list(self):
        """module_list() -> returns the module list of tuples (name, modparams)"""
        return self.mod.modules()

    def lm(self):
        for m in self.module_list():
            print "%016x %016x  %s" % (m[1].Base, m[1].Base + m[1].Size, m[0][0])

    def exports(self, name):
        """exports(name) -> returns the export list for name"""
        self.mod[name].exports()

    def imports(self, name):
        """imports(name) -> returns the import list for name"""
        self.mod[name].imports()

    def get_thread(self):
        """get_thread() -> Get current thread index"""
        return self._systems.GetCurrentThreadId()

    def set_thread(self, id):
        """set_thread(id) -> Set current thread by index"""
        self._systems.SetCurrentThreadId(id)

    def apply_threads(self, fn, tid=None):
        """apply_threads(fn, id=None) -> Run a function in all thread ctx"""
        if tid is None:
            ids,trash = self._systems.GetThreadIdsByIndex()
        else:
            ids = [tid]

        curid = self._systems.GetCurrentThreadId()
        results = []
        for id in ids:
            self._systems.SetCurrentThreadId(id)
            results.append(fn(self, id))
        self._systems.SetCurrentThreadId(curid)
        return results

    def thread_list(self):
        """thread_list() -> list of all threads"""
        # XXX - use apply_threads
        ids,sysids = self._systems.GetThreadIdsByIndex()
        curid = self._systems.GetCurrentThreadId()
        
        tebs = []
        syms = []
        for id in ids:
            self._systems.SetCurrentThreadId(id)
            tebs.append(self._systems.GetCurrentThreadTeb())
            addr = self.reg.get_pc()
            syms.append(self.get_name_by_offset(addr))
        self._systems.SetCurrentThreadId(curid)
        return zip(sysids, tebs, syms)

    def threads(self):
        """threads() -> print threads"""
        for i,t in enumerate(self.thread_list()):
            print "%d: %d - %08x %s" % (i, t[0], t[1], t[2])

    def exec_fn(self, fn, args):
        """exec_fn(fn, arglist) -> call WinApi fn with arglist"""
        if self.is64bit():
            type = pydbgeng.IMAGE_FILE_MACHINE_AMD64
            fmt = "<Q"
            pc = 'Rip'
            sp = 'Rsp'
            acc = 'Rax'
            width = 8
            print "Not supported"
            return None
        else:
            type = pydbgeng.IMAGE_FILE_MACHINE_I386
            fmt = "<I"
            pc = 'Eip'
            sp = 'Esp'
            acc = 'Eax'
            width = 4
        savedctx = self._advanced.GetThreadContext(type)
        ctx = self._advanced.GetThreadContext(type)
        
        setattr(ctx, pc, fn)
        setattr(ctx, sp, getattr(ctx, sp) - 0x100)
        for arg in args[::-1]:
            # XXX - 64bit calling convention this is not
            self.write(getattr(ctx, sp), struct.pack(fmt, arg))
            setattr(ctx, sp, getattr(ctx, sp) - width)
        setattr(ctx, sp, getattr(ctx, sp) - width)

        self._advanced.SetThreadContext(type, ctx)
        self.stepout()
        ctx = self._advanced.GetThreadContext(type)

        self._advanced.SetThreadContext(type, savedctx)
        return getattr(ctx, acc)

    def teb_addr(self):
        """teb_addr() -> return teb address for current thread"""
        return self._systems.GetCurrentThreadDataOffset()

    def teb(self, addr=None):
        """teb() -> display teb"""
        if addr is None:
            addr = self.teb_addr()
        try:
            id = self._symbols.GetTypeId(0, "ntdll!_TEB")
            self._symbols.OutputTypedDataVirtual(addr, 
                                                 self.mod.ntdll.addr,
                                                 id,
                                                 0)
            return
        except pydbgeng.E_NOINTERFACE_Error:
            pass
        # XXX No symbols..
        # Need to create a struct.unpack fmt string

    def peb_addr(self):
        """peb_addr() -> return peb address"""
        return self._systems.GetCurrentProcessDataOffset()

    def peb(self, addr=None):
        """peb() -> display peb"""
        if addr is None:
            addr = self.peb_addr()
        try:
            id = self._symbols.GetTypeId(0, "ntdll!_PEB")
            self._symbols.OutputTypedDataVirtual(addr, 
                                                 self.mod.ntdll.addr,
                                                 id,
                                                 0)
            return
        except pydbgeng.E_NOINTERFACE_Error:
            pass
        # XXX No symbols..
        # Need to create a struct.unpack fmt string

    def dd(self, addr, count=5, width=16):
        """dd(addr, count=5, width=16) -> dump data"""
        data = self._dataspaces.ReadVirtual(addr, (width * count))
        hexdump(data, vaddr=addr, width=width)

    def dp(self, addr, count=10):
        """dp(addr, count=10) -> dump data pointers (stack like)"""
        if self.is64bit():
            mult = 8
            strfmt = '<Q'
            prnfmt = "%016x: %016x  %s"
        else:
            mult = 4
            strfmt = '<I'
            prnfmt = "%08x: %08x  %s"
        data = self._dataspaces.ReadVirtual(addr, count * mult)
        while data:
            val = struct.unpack(strfmt, data[:mult])[0]
            print prnfmt % (addr, val, self.get_name_by_offset(val))
            addr += mult
            data = data[mult:]

    def ds(self, addr, wchar=False):
        """ds(addr, wchar=False) -> display string"""
        MAX_LEN=0x10000
        if wchar:
            width = 2
            enc = 'utf-16-le'
        else:
            width = 1
            enc = 'ascii'
        data = []
        for i in range(0, MAX_LEN, width):
            x = self.read(addr + i, width)    
            if x == "\x00"*width:
                break
            else:
                data.append(x)
        print ''.join(data).decode(enc)

    def symbol(self, name):
        """symbol(name) -> resolve a symbol"""
        return self._symbols.GetOffsetByName(name)

    def find_symbol(self, pattern):
        """symbol(name) -> resolve a symbol"""
        syms = []
        hnd = self._symbols.StartSymbolMatch(pattern)
        try:
            while True:
                syms.append(self._symbols.GetNextSymbolMatch(hnd))
        except pydbgeng.E_NOINTERFACE_Error:
            pass
        finally:
            self._symbols.EndSymbolMatch(hnd)
        return syms

    def set_symbol(self, addr, name):
        # XXX - Only works in an existing module
        # Other wise needs a synthetic modules
        # Synthetic modules cannot overlap other modules
        dmid = self._symbols.AddSyntheticSymbol(addr, 1, name, 0)
        return (dmid.Id, dmid.ModuleBase)

    def get_name_by_offset(self, addr):
        """get_name_by_offset(addr) -> return name near addr"""
        try:
            name = self._symbols.GetNameByOffset(addr)
            name = "%s+0x%x" % name
        except pydbgeng.E_FAIL_Error:
            name = "0x%x" % addr
        return name

    def get_module_by_offset(self, addr):
        """get_module_by_offset(addr) -> return module containing addr"""
        return self.mod.addr_to_name(addr)

    def whereami(self, addr=None):
        """whereami([addr]) -> heuristic to find where addr located"""
        if addr is None:
            addr = self.reg.get_pc()
        sym = self.get_name_by_offset(addr)
        if sym:
            print sym
        else:
            print "[unknown]"
            # XXX - do some more checks

    def bl(self):
        """bl() -> List breakpoints"""
        for bpid in self.breakpoints:
            try:
                bp = self._control.GetBreakpointById(bpid)
            except pydbgeng.E_NOINTERFACE_Error:
                self.breakpoints._remove_stale(bpid)
                continue
            if bp.GetFlags() & pydbgeng.DEBUG_BREAKPOINT_ENABLED:
                status = 'e'
            else:
                status = 'd'
            if bp.GetFlags() & pydbgeng.DEBUG_BREAKPOINT_DEFERRED:
                offset = "[Deferred]"
                expr = bp.GetOffsetExpression()
            else:
                offset = "%016x" % bp.GetOffset()
                expr = self.get_name_by_offset(bp.GetOffset())
            try:
                tid = bp.GetMatchThreadId()
                tid = "%04x" % tid
            except pydbgeng.E_NOINTERFACE_Error:
                tid = "****"

            if bp.GetType()[0] == pydbgeng.DEBUG_BREAKPOINT_DATA:
                width,prot = bp.GetDataParameters()
                width = str(width)
                prot = {4 : 'e', 2 : 'w', 1 : 'r'}[prot] 
            else:
                width = ' '
                prot  = ' '
            print "%d %s %16s %s %s %04d %04d  0:%s %s" % (bp.GetId(), status,
                    offset, prot, width, bp.GetCurrentPassCount(), 
                    bp.GetPassCount(), tid, expr)
            # id status addr acces size curpass passcount pid:tid expr
            #fmt = "%d %c %016x %c %d %04d %04d  %s %s"

    def bc(self, id):
        """bc(id) -> Clear (delete) breakpoint"""
        self.breakpoints.remove(id)

    def bd(self, id):
        """bd(id) -> Disable breakpoint"""
        self.breakpoints.disable(id)

    def be(self, id):
        """be(id) -> Enable breakpoint"""
        self.breakpoint.enable(id)

    def bp(self, expr=None, handler=None, windbgcmd=None, oneshot=False, 
            passcount=None, threadid=None):
        """bp(expr,handler,windbgcmd) -> Breakpoint on expression"""
        if expr is None:
            expr = self.reg.get_pc()
        #if threadid is None:
        #    threadid = self._systems.GetCurrentThreadId()
        if handler:
            handler = bp_wrap(self, handler)
        return self.breakpoints.set(expr, 
                                    handler, 
                                    pydbgeng.DEBUG_BREAKPOINT_CODE,
                                    windbgcmd,
                                    oneshot,
                                    passcount,
                                    threadid)

    def ba(self, expr=None, handler=None, windbgcmd=None, oneshot=False,
            passcount=None, threadid=None, size=None, access=None):
        """ba(expr,handler,windbgcmd) -> Hardware bp on expression"""
        if expr is None:
            expr = self.reg.get_pc()
        if threadid is None:
            threadid = self._systems.GetCurrentThreadId()
        if handler:
            handler = bp_wrap(self, handler)
        if size is None:
            size = 1
        if access is None:
            access = pydbgeng.DEBUG_BREAK_EXECUTE
        return self.breakpoints.set(expr, 
                                    handler, 
                                    pydbgeng.DEBUG_BREAKPOINT_DATA,
                                    windbgcmd,
                                    oneshot,
                                    passcount,
                                    threadid,
                                    size,
                                    access)

    def watch(self, expr):
        def watcher(bp):
            def leave(bp):
                print "--- Leave %s" % expr
                return pydbgeng.DEBUG_STATUS_GO
            print "+++ Enter %s" % expr
            bt = self.backtrace_list()[0]
            self.bp(bt.ReturnOffset, handler=leave, oneshot=True)
            return pydbgeng.DEBUG_STATUS_GO
        return self.bp(expr, watcher, threadid=0)

    def k(self):
        """k(self) -> alias for backtrace"""
        self.backtrace()

    def backtrace(self):
        """backtrace(self) -> current backtrace"""
        print "   %-16s %-16s %s" % ("Stack", "Return", "Instruction")
        fmt = "%02d %016x %016x %s"
        for frame in self.backtrace_list():
            print fmt % (frame.FrameNumber, 
                        frame.StackOffset,
                        frame.ReturnOffset,
                        self.get_name_by_offset(frame.InstructionOffset))

    def backtrace_list(self):
        return self._control.GetStackTrace()

#
#
#    def stack_list(self):
#        """stack_list -> list of all thread stacks"""
#        pass
#
#    def heap_list(self):
#        """heap_list -> list of all heaps"""
#        pass



class Userdbg(Windbg):
    def __init__(self):
        super(Userdbg, self).__init__(client=None, standalone=True)
        self._client.SetOutputCallbacks(sys.stdout.write)

    def proclist(self):
        """proclist() -> Print a list of running processes"""
        for id in self._client.GetRunningProcessSystemIds():
            try:
                name,desc = self._client.GetRunningProcessDescription(id)
            except WindowsError:
                name,desc = '<UKNOWN>',''
            print " %6d %4x\n      %s\n      %s" % (id, id, name, desc)

    def pids_by_name(self, proc_name):
        """pids_by_name(name) -> return a list of pids matching name"""
        pids = []
        for id in self._client.GetRunningProcessSystemIds():
            try:
                name,desc = self._client.GetRunningProcessDescription(id)
                if name.lower().endswith(proc_name.lower()):
                    pids.append(id)
            except WindowsError:
                pass
        return pids

    def create(self, name, initial_break=True):
        """create(name, initial_break=True) -> Create a new process to debug"""
        self._client.CreateProcess(0, name, pydbgeng.DEBUG_PROCESS)
        if initial_break:
            self._control.AddEngineOptions(pydbgeng.DEBUG_ENGOPT_INITIAL_BREAK)
            self.wait()
        else:
            self.go()

    def attach(self, pid, flags=pydbgeng.DEBUG_ATTACH_DEFAULT):
        """attach(pid, flags=DEBUG_ATTACH_DEFAULT) -> attach to pid"""
        self._client.AttachProcess(0, pid, flags)
        self.wait()

    def reattach(self, pid):
        """reattach(pid) -> reattach to pid"""
        #self._control.RemoveEngineOptions(pydbgeng.DEBUG_ENGOPT_INITIAL_BREAK)
        self.attach(pid, pydbgeng.DEBUG_ATTACH_EXISTING)

    def detach(self):
        """detach() -> detach from current process"""
        self._client.DetachCurrentProcess()

    def abandon(self):
        """abandon() -> abandon the current process"""
        self._client.AbandonCurrentProcess()

    def terminate(self):
        """terminate() -> terminate the current process"""
        self._client.TerminateCurrentProcess()

    @property
    def pid(self):
        return self._systems.GetCurrentProcessSystemId()

    def handoff(self):
        # XXX - Fixme
        # -y symbolpath
        # -srcpath sourcepath
        prog = r'c:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64\windbg.exe'
        pid = self.pid
        print "PID ", pid
        self.abandon()
        subprocess.Popen([prog, '-c', '~*m;g', '-pe', '-p', '%d' % pid])
        time.sleep(2)
        sys.exit(0)


class Kerneldbg(Windbg):
    def __init__(self):
        super(Kerneldbg, self).__init__(None, True)
        self._client.SetOutputCallbacks(sys.stdout.write)

    def attach(self, pipename, serial=True):
        self.events.engine_state(verbose=True)
        self.events.debuggee_state(verbose=True)
        self.events.session_status(verbose=True)
        self.events.system_error(verbose=True)
        self._control.SetEngineOptions(pydbgeng.DEBUG_ENGOPT_INITIAL_BREAK)
        if serial:
            self._client.AttachKernel(
                    pydbgeng.DEBUG_ATTACH_KERNEL_CONNECTION,
                    r"com:pipe,port=\\.\pipe\%s,reconnect,resets=0,baud=115200" % pipename)
        else:
            self._client.AttachKernel(
                    pydbgeng.DEBUG_ATTACH_KERNEL_CONNECTION,
                    pipename)
        self.wait()

    def detach(self):
        pass


class Crashdump(Windbg):
    def __init__(self):
        super(Crashdump, self).__init__(client=None, standalone=True)
        self._client.SetOutputCallbacks(sys.stdout.write)

    def load_dump(self, name):
        self._client.OpenDumpFile(name)
        self.go()
