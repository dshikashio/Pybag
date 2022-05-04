from comtypes.hresult   import S_OK
from comtypes           import CoClass, GUID
import comtypes

from . import core as DbgEng
from . import exception
from .idebugbreakpoint import DebugBreakpoint


class EventHandler(object):
    def __init__(self, dbg):
        self._handlers = {}
        self._mask = 0
        self._dbg = dbg

    def _ignore_event(self, event):
        self._handlers[event] = None
        self._mask &= ~event
        self._dbg._reset_callbacks()

    def _catch_event(self, event, handler):
        if handler is None:
            handler = self._ev_break
        self._mask |= event
        self._handlers[event] = handler
        self._dbg._reset_callbacks()

    def callback(self, event, *args):
        return self._handlers[event](*args)

    @staticmethod
    def _ev_break(*args):
        """Default handler when none provided"""
        return DbgEng.DEBUG_STATUS_BREAK

    @staticmethod
    def _ev_breakpoint(*args):
        bp = DebugBreakpoint(args[0])
        try:
            tid = bp.GetMatchThreadId()
        except exception.E_NOINTERFACE_Error:
            tid = "ANY"
        print("        ID: ", bp.GetId())
        print("     Flags: ", bp.GetFlags())
        print("    Offset: ", bp.GetOffset())
        print("    OffExp: ", bp.GetOffsetExpression())
        print("  PassOrig: ", bp.GetPassCount())
        print("  Pass Cnt: ", bp.GetCurrentPassCount())
        print("    Thread: ", tid)
        print("   BP Type: ", bp.GetType()[0])
        return DbgEng.DEBUG_STATUS_GO

    def breakpoint(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_breakpoint
        self._catch_event(DbgEng.DEBUG_EVENT_BREAKPOINT, handler)

    def nobreakpoint(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_BREAKPOINT)

    @staticmethod
    def _ev_exception(*args):
        print("DEBUG_EVENT_EXCEPTION")
        if args[1]:
            print("  First Chance")
        else:
            print("  Second Chance")
        print("  Address: %x" % args[0].ExceptionAddress)
        print("     Code: %x" % args[0].ExceptionCode)
        print("    Flags: %x" % args[0].ExceptionFlags)
        print("   Record: %x" % args[0].ExceptionRecord)
        print("   Params: %x" % args[0].NumberParameters)

    def exception(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exception
        self._catch_event(DbgEng.DEBUG_EVENT_EXCEPTION, handler)

    def noexception(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_EXCEPTION)

    @staticmethod
    def _ev_loadmodule(*args):
        print("DEBUG_EVENT_LOAD_MODULE")
        print("  ImageFileHandle: ", args[0])
        print("  BaseOffset: ", args[1])
        print("  ModuleSize: ", args[2])
        print("  ModuleName: ", args[3])
        print("   ImageName: ", args[4])
        print("    CheckSum: ", args[5])
        print("    TimeDate: ", args[6])
        return DbgEng.DEBUG_STATUS_BREAK

    def module_load(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_loadmodule
        self._catch_event(DbgEng.DEBUG_EVENT_LOAD_MODULE, handler)

    def nomodule_load(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_LOAD_MODULE)

    @staticmethod
    def _ev_unloadmodule(*args):
        print("DEBUG_EVENT_UNLOAD_MODULE")
        print("  ImageBaseName: ", args[0])
        print("     BaseOffset: ", args[1])
        return DbgEng.DEBUG_STATUS_BREAK

    def unload_module(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_unloadmodule
        self._catch_event(DbgEng.DEBUG_EVENT_UNLOAD_MODULE, handler)

    def nounload_module(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_UNLOAD_MODULE)

    @staticmethod
    def _ev_createthread(*args):
        print("DEBUG_EVENT_CREATE_THREAD")
        print("       Handle: ", args[0])
        print("   DataOffset: %x" % args[1])
        print("  StartOffset: %x" % args[2])
        return DbgEng.DEBUG_STATUS_BREAK

    def create_thread(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_createthread
        self._catch_event(DbgEng.DEBUG_EVENT_CREATE_THREAD, handler)

    def nocreate_thread(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_CREATE_THREAD)

    @staticmethod
    def _ev_createprocess(*args):
        print("DEBUG_EVENT_CREATE_PROCESS")
        print("  ImageFileHandle: ", args[0])
        print("  Handle: ", args[1])
        print("  BaseOffset: ", args[2])
        print("  ModuleSize: ", args[3])
        print("  ModuleName: ", args[4])
        print("   ImageName: ", args[5])
        print("    CheckSum: ", args[6])
        print("    TimeDate: ", args[7])
        print("  InitialThreadHandle: ", args[8])
        print("  ThreadDataOffset: ", args[9])
        print("  StartOffset: ", args[10])
        return DbgEng.DEBUG_STATUS_BREAK

    def create_process(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_createprocess
        self._catch_event(DbgEng.DEBUG_EVENT_CREATE_PROCESS, handler)

    def nocreate_process(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_CREATE_PROCESS)

    @staticmethod
    def _ev_exitprocess(*args):
        print("DEBUG_EVENT_EXIT_PROCESS")
        print("  ExitCode: ", args[0])
        return DbgEng.DEBUG_STATUS_BREAK

    def exit_process(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exitprocess
        self._catch_event(DbgEng.DEBUG_EVENT_EXIT_PROCESS, handler)

    def noexit_process(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_EXIT_PROCESS)

    @staticmethod
    def _ev_exitthread(*args):
        print("DEBUG_EVENT_EXIT_THREAD")
        print("  ExitCode: ", args[0])
        return DbgEng.DEBUG_STATUS_BREAK

    def exit_thread(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_exitthread
        self._catch_event(DbgEng.DEBUG_EVENT_EXIT_THREAD, handler)

    def noexit_thread(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_EXIT_THREAD)

    @staticmethod
    def _ev_systemerror(*args):
        print("DEBUG_EVENT_SYSTEM_ERROR")
        print("  Error: ", args[0])
        print("  Level: ", args[1])

    def system_error(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_systemerror
        self._catch_event(DbgEng.DEBUG_EVENT_SYSTEM_ERROR, handler)

    def nosystem_error(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_SYSTEM_ERROR)

    @staticmethod
    def _ev_sessionstatus(*args):
        print("DEBUG_EVENT_SESSION_STATUS")
        sdict = {DbgEng.DEBUG_SESSION_ACTIVE:
                    "DEBUG_SESSION_ACTIVE",
                DbgEng.DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE:
                    "DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE",
                DbgEng.DEBUG_SESSION_END_SESSION_ACTIVE_DETACH:
                    "DEBUG_SESSION_END_SESSION_ACTIVE_DETACH",
                DbgEng.DEBUG_SESSION_END_SESSION_PASSIVE:
                    "DEBUG_SESSION_END_SESSION_PASSIVE",
                DbgEng.DEBUG_SESSION_END:
                    "DEBUG_SESSION_END",
                DbgEng.DEBUG_SESSION_REBOOT:
                    "DEBUG_SESSION_REBOOT",
                DbgEng.DEBUG_SESSION_HIBERNATE:
                    "DEBUG_SESSION_HIBERNATE",
                DbgEng.DEBUG_SESSION_FAILURE:
                    "DEBUG_SESSION_FAILURE"}
        print("  Status: ", sdict[args[0]])

    def session_status(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_sessionstatus
        self._catch_event(DbgEng.DEBUG_EVENT_SESSION_STATUS, handler)

    def nosession_status(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_SESSION_STATUS)

    @staticmethod
    def _ev_debuggeestate(*args):
        print("DEBUG_EVENT_CHANGE_DEBUGGEE_STATE")
        print("  Args: {}".format(args))
        fdict = {DbgEng.DEBUG_CDS_ALL:       'DEBUG_CDS_ALL',
                DbgEng.DEBUG_CDS_REGISTERS: 'DEBUG_CDS_REGISTERS',
                DbgEng.DEBUG_CDS_DATA:      'DEBUG_CDS_DATA'}
        print("  Flags: ", fdict[args[0]])
        if args[0] == DbgEng.DEBUG_CDS_DATA:
            adict = {DbgEng.DEBUG_DATA_SPACE_VIRTUAL:
                        'DEBUG_DATA_SPACE_VIRTUAL',
                    DbgEng.DEBUG_DATA_SPACE_PHYSICAL:
                        'DEBUG_DATA_SPACE_PHYSICAL',
                    DbgEng.DEBUG_DATA_SPACE_CONTROL:
                        'DEBUG_DATA_SPACE_CONTROL',
                    DbgEng.DEBUG_DATA_SPACE_IO:
                        'DEBUG_DATA_SPACE_IO',
                    DbgEng.DEBUG_DATA_SPACE_MSR:
                        'DEBUG_DATA_SPACE_MSR',
                    DbgEng.DEBUG_DATA_SPACE_BUS_DATA:
                        'DEBUG_DATA_SPACE_BUS_DATA'}
            print("  Argument: ", adict[args[1]])
        else:
            print("  Argument: ", args[1])

    def debuggee_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_debuggeestate
        self._catch_event(DbgEng.DEBUG_EVENT_CHANGE_DEBUGGEE_STATE, handler)

    def nodebuggee_state(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_CHANGE_DEBUGGEE_STATE)

    @staticmethod
    def _ev_enginestate(*args):
        print("DEBUG_EVENT_CHANGE_ENGINE_STATE")
        fdict = {DbgEng.DEBUG_CES_CURRENT_THREAD:
                    "DEBUG_CES_CURRENT_THREAD",
                DbgEng.DEBUG_CES_EFFECTIVE_PROCESSOR:
                    "DEBUG_CES_EFFECTIVE_PROCESSOR",
                DbgEng.DEBUG_CES_BREAKPOINTS:
                    "DEBUG_CES_BREAKPOINTS",
                DbgEng.DEBUG_CES_CODE_LEVEL:
                    "DEBUG_CES_CODE_LEVEL",
                DbgEng.DEBUG_CES_EXECUTION_STATUS:
                    "DEBUG_CES_EXECUTION_STATUS",
                DbgEng.DEBUG_CES_ENGINE_OPTIONS:
                    "DEBUG_CES_ENGINE_OPTIONS",
                DbgEng.DEBUG_CES_LOG_FILE:
                    "DEBUG_CES_LOG_FILE",
                DbgEng.DEBUG_CES_RADIX:
                    "DEBUG_CES_RADIX",
                DbgEng.DEBUG_CES_EVENT_FILTERS:
                    "DEBUG_CES_EVENT_FILTERS",
                DbgEng.DEBUG_CES_PROCESS_OPTIONS:
                    "DEBUG_CES_PROCESS_OPTIONS",
                DbgEng.DEBUG_CES_EXTENSIONS:
                    "DEBUG_CES_EXTENSIONS",
                DbgEng.DEBUG_CES_SYSTEMS:
                    "DEBUG_CES_SYSTEMS",
                DbgEng.DEBUG_CES_ASSEMBLY_OPTIONS:
                    "DEBUG_CES_ASSEMBLY_OPTIONS",
                DbgEng.DEBUG_CES_EXPRESSION_SYNTAX:
                    "DEBUG_CES_EXPRESSION_SYNTAX",
                DbgEng.DEBUG_CES_TEXT_REPLACEMENTS:
                    "DEBUG_CES_TEXT_REPLACEMENTS"}
        try:
            print("  Flags: ", fdict[args[0]])
        except:
            pass
        if args[0] == DbgEng.DEBUG_CES_EXECUTION_STATUS:
            adict = {DbgEng.DEBUG_STATUS_NO_DEBUGGEE:
                        "DEBUG_STATUS_NO_DEBUGGEE",
                    DbgEng.DEBUG_STATUS_BREAK:
                        "DEBUG_STATUS_BREAK",
                    DbgEng.DEBUG_STATUS_STEP_INTO:
                        "DEBUG_STATUS_STEP_INTO",
                    DbgEng.DEBUG_STATUS_STEP_BRANCH:
                        "DEBUG_STATUS_STEP_BRANCH",
                    DbgEng.DEBUG_STATUS_STEP_OVER:
                        "DEBUG_STATUS_STEP_OVER",
                    DbgEng.DEBUG_STATUS_GO_NOT_HANDLED:
                        "DEBUG_STATUS_GO_NOT_HANDLED",
                    DbgEng.DEBUG_STATUS_GO_HANDLED:
                        "DEBUG_STATUS_GO_HANDLED",
                    DbgEng.DEBUG_STATUS_GO:
                        "DEBUG_STATUS_GO",
                    DbgEng.DEBUG_STATUS_IGNORE_EVENT:
                        "DEBUG_STATUS_IGNORE_EVENT",
                    DbgEng.DEBUG_STATUS_RESTART_REQUESTED:
                        "DEBUG_STATUS_RESTART_REQUESTED", 
                    DbgEng.DEBUG_STATUS_NO_CHANGE:
                        "DEBUG_STATUS_NO_CHANGE"}
            a = args[1] & 0xfffffff
            b = args[1] & DbgEng.DEBUG_STATUS_INSIDE_WAIT
            if b:
                b = "DEBUG_STATUS_INSIDE_WAIT"
            c = args[1] & DbgEng.DEBUG_STATUS_WAIT_TIMEOUT
            if c:
                c = "DEBUG_STATUS_WAIT_TIMEOUT"
            print("  Argument: ", adict[a], b, c)
        else:
            print("  Argument: ", args[1])
        print("")

    def engine_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_enginestate
        self._catch_event(DbgEng.DEBUG_EVENT_CHANGE_ENGINE_STATE, handler)

    def noengine_state(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_CHANGE_ENGINE_STATE)

    @staticmethod
    def _ev_symbolstate(*args):
        print("DEBUG_EVENT_CHANGE_SYMBOL_STATE")
        fdict = {DbgEng.DEBUG_CSS_LOADS:          "DEBUG_CSS_LOADS",
                DbgEng.DEBUG_CSS_UNLOADS:        "DEBUG_CSS_UNLOADS",
                DbgEng.DEBUG_CSS_SCOPE:          "DEBUG_CSS_SCOPE",
                DbgEng.DEBUG_CSS_PATHS:          "DEBUG_CSS_PATHS",
                DbgEng.DEBUG_CSS_SYMBOL_OPTIONS: "DEBUG_CSS_SYMBOL_OPTIONS",
                DbgEng.DEBUG_CSS_TYPE_OPTIONS:   "DEBUG_CSS_TYPE_OPTIONS"}
        print("Flags: ", fdict[args[0]])
        print("Argument: ", args[1])

    def symbol_state(self, handler=None, verbose=False):
        if verbose and handler is None:
            handler = self._ev_symbolstate
        self._catch_event(DbgEng.DEBUG_EVENT_CHANGE_SYMBOL_STATE, handler)

    def nosymbol_state(self):
        self._ignore_event(DbgEng.DEBUG_EVENT_CHANGE_SYMBOL_STATE)


class DbgEngCallbacks(CoClass):
    #_reg_clsid_ = GUID('{EAC5ACAA-7BD0-4f1f-8DEB-DF2862A7E85B}')
    _reg_clsid_ = GUID('{337BE28B-5036-4D72-B6BF-C45FBB9F2EAA}')
    _reg_threading_ = "Both"
    _reg_progid_ = "dbgeng.DbgEngCallbacks.1"
    _reg_novers_progid_ = "dbgeng.DbgEngCallbacks"
    _reg_desc_ = "Callbacks"
    _reg_clsctx_ = comtypes.CLSCTX_INPROC_SERVER

    _com_interfaces_ = [DbgEng.IDebugEventCallbacks,
                        DbgEng.IDebugOutputCallbacks,
                        comtypes.typeinfo.IProvideClassInfo2,
                        comtypes.errorinfo.ISupportErrorInfo,
                        comtypes.connectionpoints.IConnectionPointContainer]

    def __init__(self, event_handler, stdout):
        super().__init__()
        self._stdout_orig = stdout
        self.stdout = stdout
        self._ev = event_handler

    @property
    def stdout(self):
        return self._stdout

    @stdout.setter
    def stdout(self, stdout):
        self._stdout = stdout

    def reset_stdout(self):
        self.stdout = self._stdout_orig

    def IDebugOutputCallbacks_Output(self, mask, text):
        #print("OutputCallbacks Output called {}".format(mask))
        self.stdout.write(text.decode('utf-8'))  # XXX - can just be decode? no utf-8

    #
    # The Event callbacks need a 'this' parameter. It is special to comtypes and lets us
    # properly pass the result back from the callbacks
    # These are called - low-level method implementations
    #
    def IDebugEventCallbacks_Breakpoint(self, this, *args):
        #print("Event Breakpoint")
        return self._ev.callback(DbgEng.DEBUG_EVENT_BREAKPOINT, *args)

    def IDebugEventCallbacks_ChangeDebuggeeState(self, this, *args):
        #print("Event ChangeDebuggeeState")
        return self._ev.callback(DbgEng.DEBUG_EVENT_CHANGE_DEBUGGEE_STATE, *args)

    def IDebugEventCallbacks_ChangeEngineState(self, this, *args):
        #print("Event ChangeEngineState")
        return self._ev.callback(DbgEng.DEBUG_EVENT_CHANGE_ENGINE_STATE, *args)

    def IDebugEventCallbacks_Exception(self, this, *args):
        #print("Event Exception")
        return self._ev.callback(DbgEng.DEBUG_EVENT_EXCEPTION, *args)

    def IDebugEventCallbacks_GetInterestMask(self, mask):
        #print("GetInterestMask : {}".format(self._ev._mask))
        mask.contents.value = self._ev._mask
        return S_OK

    def IDebugEventCallbacks_LoadModule(self, this, *args):
        #print("Event LoadModule")
        return self._ev.callback(DbgEng.DEBUG_EVENT_LOAD_MODULE, *args)

    def IDebugEventCallbacks_UnloadModule(self, this, *args):
        #print("Event UnloadModule")
        return self._ev.callback(DbgEng.DEBUG_EVENT_UNLOAD_MODULE, *args)

    def IDebugEventCallbacks_CreateProcess(self, this, *args):
        #print("Event CreateProcess")
        return self._ev.callback(DbgEng.DEBUG_EVENT_CREATE_PROCESS, *args)

    def IDebugEventCallbacks_ExitProcess(self, this, *args):
        #print("Event ExitProcess")
        return self._ev.callback(DbgEng.DEBUG_EVENT_EXIT_PROCESS, *args)

    def IDebugEventCallbacks_SessionStatus(self, this, *args):
        #print("Event SessionStatus")
        return self._ev.callback(DbgEng.DEBUG_EVENT_SESSION_STATUS, *args)

    def IDebugEventCallbacks_ChangeSymbolState(self, this, *args):
        #print("Event ChangeSymbolState")
        return self._ev.callback(DbgEng.DEBUG_EVENT_CHANGE_SYMBOL_STATE, *args)

    def IDebugEventCallbacks_SystemError(self, this, *args):
        #print("Event SystemError")
        return self._ev.callback(DbgEng.DEBUG_EVENT_SYSTEM_ERROR, *args)

    def IDebugEventCallbacks_CreateThread(self, this, *args):
        #print("Event CreateThread")
        return self._ev.callback(DbgEng.DEBUG_EVENT_CREATE_THREAD, *args)

    def IDebugEventCallbacks_ExitThread(self, this, *args):
        #print("Event ExitThread")
        return self._ev.callback(DbgEng.DEBUG_EVENT_EXIT_THREAD, *args)

