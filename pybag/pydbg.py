import io
import itertools
import queue
import struct
import sys
import threading

from .dbgeng import core as DbgEng
from .dbgeng import exception
from .dbgeng import util
from .dbgeng.idebugclient    import DebugClient
from .dbgeng.breakpoints     import Breakpoints
from .dbgeng.callbacks       import EventHandler
from .dbgeng.callbacks       import DbgEngCallbacks
from .dbgeng.modules         import Modules
from .dbgeng.registers       import Registers


class WorkItem(object):
    def __init__(self, task, timeout, args=None):
        self.task = task
        self.timeout = timeout
        self.args = args


def InitComObjects(Dbg):
    Dbg._advanced       = Dbg._client.IDebugAdvanced()
    Dbg._control        = Dbg._client.IDebugControl()
    Dbg._registers      = Dbg._client.IDebugRegisters()
    Dbg._dataspaces     = Dbg._client.IDebugDataSpaces()
    Dbg._symbols        = Dbg._client.IDebugSymbols()
    Dbg._systems        = Dbg._client.IDebugSystemObjects()
    Dbg.reg             = Registers(Dbg._registers)
    Dbg.mod             = Modules(Dbg._dataspaces, Dbg._symbols)
    Dbg.events          = EventHandler(Dbg)
    Dbg.breakpoints     = Breakpoints(Dbg._control)
    Dbg.callbacks       = DbgEngCallbacks(Dbg.events, sys.stdout)

    Dbg.events.breakpoint(Dbg.breakpoints)

    #Dbg.events.engine_state(verbose=True)
    #Dbg.events.debuggee_state(verbose=True)
    #Dbg.events.session_status(verbose=True)
    #Dbg.events.symbol_state(verbose=True)
    #Dbg.events.system_error(verbose=True)

    Dbg._client.SetOutputCallbacks(Dbg.callbacks)
    Dbg._client.SetEventCallbacks(Dbg.callbacks)


def FiniComObjects(Dbg):
    #Dbg.breakpoints.remove_all()
    Dbg._client.SetOutputCallbacks(None)
    Dbg._client.SetEventCallbacks(None)
    Dbg.callbacks = None
    Dbg.reg = None
    Dbg.mod = None
    Dbg.events = None
    Dbg.breakpoints = None

    """
    x = Dbg._advanced.Release()
    print(f"{x=}")
    x = Dbg._control.Release()
    print(f"{x=}")
    x = Dbg._registers.Release()
    print(f"{x=}")
    x = Dbg._dataspaces.Release()
    print(f"{x=}")
    x = Dbg._symbols.Release()
    print(f"{x=}")
    x = Dbg._systems.Release()
    print(f"{x=}")
    x = Dbg._client.Release()
    print(f"{x=}")
    """
    Dbg._advanced = None
    Dbg._control = None
    Dbg._registers = None
    Dbg._dataspaces = None
    Dbg._symbols = None
    Dbg._systems = None
    Dbg._client = None


def EventThread(Dbg, Ev, WorkQ):
    Dbg._client = DebugClient()
    InitComObjects(Dbg)
    Ev.set()
    run = True

    while run:
        item = WorkQ.get(True)
        if item.task == 'WaitForEvent':
            try:
                #print("Call WaitForEvent {:x}".format(item.timeout))
                Dbg._control.WaitForEvent(item.timeout)
            except Exception as ex:
                #print("WaitForEvent exception", ex)
                pass
        elif item.task == 'DispatchCallbacks':
            try:
                #print("Call DispatchCallbacks {}".format(item.timeout))
                Dbg._client.DispatchCallbacks(item.timeout)
            except Exception as ex:
                #print("DispatchCallbacks exception", ex)
                pass
        elif item.task == 'AttachKernel':
            try:
                #print("Calling AttachKernel {}".format(item.args[0]))
                Dbg._client.AttachKernel(item.args[0])
            except Exception as ex:
                #print("DispatchCallbacks exception", ex)
                pass
        elif item.task == 'Quit':
            run = False

        WorkQ.task_done()
        Ev.set()


class DebuggerBase(object):
    def __init__(self, client=None, standalone=False):
        self.standalone = standalone

        if standalone:
            self._ev = threading.Event()
            self._q = queue.Queue()
            self._thread = threading.Thread(target=EventThread,
                                            args=(self, self._ev, self._q))
            self._thread.daemon = True
            self._thread.start()
            self._ev.wait(5)
            if not self._ev.is_set():
                raise RuntimeError
        else:
            if client:
                self._client = client
            else:
                self._client = DebugClient()

            InitComObjects(self)

    def Release(self):
        if self.standalone:
            self._worker_wait("Quit")
            self._thread = None
        else:
            pass
        #FiniComObjects(self)

    def _reset_callbacks(self):
        self._client.SetEventCallbacks(self.callbacks)

    """
    DEBUG_OUTPUT_NORMAL  Normal output.
    DEBUG_OUTPUT_ERROR  Error output.
    DEBUG_OUTPUT_WARNING  Warnings.
    DEBUG_OUTPUT_VERBOSE  Additional output.
    DEBUG_OUTPUT_PROMPT  Prompt output.
    DEBUG_OUTPUT_PROMPT_REGISTERS  Register dump before prompt.
    DEBUG_OUTPUT_EXTENSION_WARNING  Warnings specific to extension operation.
    DEBUG_OUTPUT_DEBUGGEE  Debug output from the target (for example, OutputDebugString or DbgPrint).
    DEBUG_OUTPUT_DEBUGGEE_PROMPT  Debug input expected by the target (for example, DbgPrompt).
    DEBUG_OUTPUT_SYMBOLS  Symbol messages (for example, !sym noisy).
    """
    def set_output_mask(self, mask):
        self._client.SetOutputMask(mask)

    def get_output_mask(self):
        return self._client.GetOutputMask()

    def exec_status(self):
        st = self._control.GetExecutionStatus()
        return DbgEng.str_execution_status(st)

    def go(self, timeout=-1):
        """go(timeout) -> Continue execution"""
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_GO)
        return self.wait(timeout)

    def goto(self, expr):
        """goto(expr) -> Continue and stop at expr"""
        # Set one time bp on expr for thread
        tid = self.get_thread()
        self.bp(expr, oneshot=True, threadid=tid)
        return self.go()

    def go_handled(self, timeout=DbgEng.WAIT_INFINITE):
        """go_handled(timeout) -> Continue with exception handled"""
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_GO_HANDLED)
        return self.wait(timeout)

    def go_nothandled(self, timeout=DbgEng.WAIT_INFINITE):
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_GO_NOT_HANDLED)
        return self.wait(timeout)

    def stepi(self, count=1):
        """stepi(count=1) -> step into count instructions"""
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_STEP_INTO)
        for tmp in range(count):
            self.wait()

    def stepo(self, count=1):
        """stepo(count=1) -> step over count instructions"""
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_STEP_OVER)
        for tmp in range(count):
            self.wait()

    def stepbr(self, count=1):
        """stepbr(count=1) -> step branch count instructions"""
        self._control.SetExecutionStatus(DbgEng.DEBUG_STATUS_STEP_BRANCH)
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

    def trace(self, count=1, registers=True):
        """trace() -> step pc and print context"""
        for i in range(count):
            self.stepi()
            if registers:
                self.registers()
            else:
                self.pc()

    def traceto(self, addr, registers=True, max=None):
        """traceto(addr, [max]) -> trace until addr or max instructions"""
        for i in itertools.count():
            if max and i >= max:
                return False
            if self.reg.get_pc() == addr:
                return True
            self.trace(registers)

    def dispatch_events(self):
        if not self._worker_wait('DispatchCallbacks', 1):
            self._client.ExitDispatch()
            return False
        else:
            return True

    def _worker_wait(self, msg, timeout=DbgEng.WAIT_INFINITE, args=None):
        if timeout == -1:
            timeout = 0xffffffff
        if timeout != 0xffffffff:
            timeout *= 1000

        item = WorkItem(msg, timeout, args)
        self._ev.clear()
        self._q.put(item)
        try:
            for i in itertools.repeat(1, int(timeout / 1000)):
                if self._ev.is_set():
                    break
                self._ev.wait(1)
        except KeyboardInterrupt:
            pass

        if self._ev.is_set():
            return True
        else:
            return False

    def wait(self, timeout=DbgEng.WAIT_INFINITE):
        """wait(timeout=WAIT_INFINITE) -> Wait timeout seconds for an event"""
        if self.standalone:
            if not self._worker_wait('WaitForEvent', timeout):
                self._control.SetInterrupt(DbgEng.DEBUG_INTERRUPT_ACTIVE)
                return False
            else:
                return True
        else:
            self._control.WaitForEvent(timeout)

    def cmd(self, cmdline, quiet=True):
        """cmd(cmdline) -> execute a windbg console command"""
        buffer = io.StringIO()
        self.callbacks.stdout = buffer
        self._control.Execute(cmdline)
        self.callbacks.reset_stdout()
        buffer.seek(0)
        data = buffer.read()
        if not quiet:
            print(data)
        return data

    def bitness(self):
        """bitness() -> Return target bitness"""
        if self._control.IsPointer64Bit():
            return '64'
        else:
            return '32'

    def read(self, addr, len=1):
        """read(addr,len) -> read len bytes from addr"""
        return self._dataspaces.ReadVirtual(addr, len)

    def readstr(self, addr, wchar=False):
        """readstr(addr, wchar=False) -> return string"""
        MAX_LEN = 0x1000
        if wchar:
            width = 2
            enc = 'utf-16-le'
        else:
            width = 1
            enc = 'utf-8'
        data = []
        for i in range(0, MAX_LEN, width):
            x = self.read(addr + i, width)
            if x == b"\x00"*width:
                break
            else:
                data.append(x)
        return b''.join(data).decode(enc)

    def readptr(self, addr, count=1):
        """readptr(addr,count=1) -> read and return size_t dwords from addr"""
        if self.bitness() == '64':
            width = 8
            fmt = "Q"
        else:
            width = 4
            fmt = "I"
        data = self.read(addr, count * width)
        return struct.unpack("<" + fmt*count, data)

    def poi(self, addr):
        """poi(addr) -> read pointer from addr"""
        return self.readptr(addr)[0]

    def write(self, addr, data):
        """write(addr,data) -> write data to addr"""
        return self._dataspaces.WriteVirtual(addr, data)

    def writeptr(self, addr, ptr, bitness=None):
        if bitness is None:
            bitness = self.bitness()
        if bitness == '64':
            fmt = "Q"
        else:
            fmt = "I"
        data = struct.pack("<"+fmt, ptr)
        return self.write(addr, data)

    def memory_list(self):
        """memory_list() -> Return a list of all process memory regions"""
        memlist = []
        try:
            addr = 0
            while True:
                info = self._dataspaces.QueryVirtual(addr)
                memlist.append(info)
                addr = info.BaseAddress + info.RegionSize
        except exception.E_NOINTERFACE_Error:
            pass
        return memlist

    def address(self, addr=None):
        """address(addr) -> Return information about an addr"""
        if addr is not None:
            print(util.str_memory_info(self._dataspaces.QueryVirtual(addr)))
        else:
            for info in self.memory_list():
                print(util.str_memory_info(info))

    def instruction_at(self, addr=None):
        """instruction_at(addr) -> Return instruction at addr"""
        if not addr:
            addr = self.reg.get_pc()
        data = self.read(addr, 15)
        return util.disassemble_string(self.bitness(), addr, data)

    def _disasm(self, addr):
        """_disasm(addr) -> disassemble instructions generator"""
        try:
            while True:
                ins = util.disassemble_instruction(self.bitness(), addr, self.read(addr, 15))
                yield(addr, ins)
                addr += ins.size
        except OSError:
            # Catch OSError for when read(addr) is an invalid address
            pass

    def disasm(self, addr=None, count=10):
        """disasm(addr=pc, count=10) -> disassemble as code to str"""
        if not addr:
            addr = self.reg.get_pc()
        print("{}".format(self.get_name_by_offset(addr)))
        for ins in itertools.islice(self._disasm(addr), count):
            print(util.str_instruction(ins[1], self.bitness()))

    def pc(self):
        """pc() -> print program counter"""
        addr   = self.reg.get_pc()
        instr  = self.instruction_at(addr)
        name   = self.get_name_by_offset(addr)
        ibytes = "%02x"*instr[1]
        ibytes = ibytes % struct.unpack("B"*instr[1], self.read(addr, instr[1]))
        print("%s" % name)
        print("%015x %-15s  %s\n" % (addr, ibytes, instr[0]))

    def r(self):
        """r() -> registers() alias"""
        self.registers()

    def registers(self):
        """registers() -> print current register state"""
        self.reg.output()
        print("")
        self.pc()

    def get_name_by_offset(self, addr):
        """get_name_by_offset(addr) -> return name near addr"""
        try:
            (name, disp) = self._symbols.GetNameByOffset(addr)
            name = "%s+0x%x" % (name.decode(), disp)
        except exception.E_FAIL_Error:
            name = "0x%x" % addr
        return name

    def symbol(self, name):
        """symbol(name) -> resolve a symbol"""
        try:
            return self._symbols.GetOffsetByName(name)[1]
        except exception.E_FAIL_Error:
            return -1

    def find_symbol(self, pattern):
        """symbol(name) -> resolve a symbol"""
        syms = []
        hnd = self._symbols.StartSymbolMatch(pattern)
        try:
            while True:
                syms.append(self._symbols.GetNextSymbolMatch(hnd))
        except exception.E_NOINTERFACE_Error:
            pass
        finally:
            self._symbols.EndSymbolMatch(hnd)
        return syms

    def whereami(self, addr=None):
        """whereami([addr]) -> heuristic to find where addr located"""
        if addr is None:
            addr = self.reg.get_pc()
        sym = self.get_name_by_offset(addr)
        if sym:
            print(sym)
        else:
            print("[unknown]")

    def dd(self, addr, count=5, width=16):
        """dd(addr, count=5, width=16) -> dump data"""
        data = self.read(addr, (width * count))
        util.hexdump(data, vaddr=addr, width=width)

    def dp(self, addr, count=10):
        """dp(addr, count=10) -> dump data pointers (stack like)"""
        if self.bitness() == '64':
            mult = 8
            strfmt = '<Q'
            prnfmt = "%016x: %016x  %s"
        else:
            mult = 4
            strfmt = '<I'
            prnfmt = "%08x: %08x  %s"
        data = self.read(addr, count * mult)
        while data:
            val = struct.unpack(strfmt, data[:mult])[0]
            print(prnfmt % (addr, val, self.get_name_by_offset(val)))
            addr += mult
            data = data[mult:]

    def ds(self, addr, wchar=False):
        """ds(addr, wchar=False) -> display string"""
        print(self.readstr(addr,wchar))

    def bl(self):
        """bl() -> List breakpoints"""
        # Make a copy so we can remove stale if needed
        ids = [bpid for bpid in self.breakpoints]
        for bpid in ids:
            try:
                bp = self._control.GetBreakpointById(bpid)
            except exception.E_NOINTERFACE_Error:
                self.breakpoints._remove_stale(bpid)
                continue

            if bp.GetFlags() & DbgEng.DEBUG_BREAKPOINT_ENABLED:
                status = 'e'
            else:
                status = 'd'
            if bp.GetFlags() & DbgEng.DEBUG_BREAKPOINT_DEFERRED:
                offset = "[Deferred]"
                expr = bp.GetOffsetExpression()
            else:
                offset = "%016x" % bp.GetOffset()
                expr = self.get_name_by_offset(bp.GetOffset())
            try:
                tid = bp.GetMatchThreadId()
                tid = "%04x" % tid
            except exception.E_NOINTERFACE_Error:
                tid = "****"

            if bp.GetType()[0] == DbgEng.DEBUG_BREAKPOINT_DATA:
                width, prot = bp.GetDataParameters()
                width = str(width)
                prot = {4: 'e', 2: 'w', 1: 'r'}[prot] 
            else:
                width = ' '
                prot  = ' '
            print("%d %s %16s %s %s %04d %04d  0:%s %s" % (
                        bp.GetId(), status, offset, prot, width,
                        bp.GetCurrentPassCount(), bp.GetPassCount(),
                        tid, expr))

    def bc(self, id):
        """bc(id) -> Clear (delete) breakpoint"""
        self.breakpoints.remove(id)

    def bd(self, id):
        """bd(id) -> Disable breakpoint"""
        self.breakpoints.disable(id)

    def be(self, id):
        """be(id) -> Enable breakpoint"""
        self.breakpoints.enable(id)

    def bp(self, expr=None, handler=None, windbgcmd=None, oneshot=False,
            passcount=None, threadid=None):
        """bp(expr,handler,windbgcmd) -> Breakpoint on expression"""
        if expr is None:
            expr = self.reg.get_pc()
        #if threadid is None:
        #    threadid = self._systems.GetCurrentThreadId()
        if handler:
            handler = util.bp_wrap(self, handler)
        return self.breakpoints.set(expr,
                                    handler, 
                                    DbgEng.DEBUG_BREAKPOINT_CODE,
                                    windbgcmd,
                                    oneshot,
                                    passcount,
                                    threadid)

    def ba(self, expr=None, handler=None, windbgcmd=None, oneshot=False,
            passcount=None, threadid=None, size=None, access=None):
        
        if expr is None:
            expr = self.reg.get_pc()
        if threadid is None:
            threadid = self._systems.GetCurrentThreadId()
        if handler:
            handler = util.bp_wrap(self, handler)
        if size is None:
            size = 1
        if access is None:
            access = DbgEng.DEBUG_BREAK_EXECUTE
        return self.breakpoints.set(expr,
                                    handler,
                                    DbgEng.DEBUG_BREAKPOINT_DATA,
                                    windbgcmd,
                                    oneshot,
                                    passcount,
                                    threadid,
                                    size,
                                    access)

    def handle_list(self, max_handle=0x10000):
        """handle_list() -> Return a list of all handles"""
        hlist = []
        for i in range(4, max_handle, 4):
            try:
                hlist.append((i,
                            self._dataspaces.ReadHandleData(i,
                                DbgEng.DEBUG_HANDLE_DATA_TYPE_TYPE_NAME),
                            self._dataspaces.ReadHandleData(i,
                                DbgEng.DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME)))
            except OSError:
                pass
        return hlist

    def handles(self, max_handle=0x10000):
        """handles() -> print current handles"""
        for h in self.handle_list(max_handle):
            print("%08x : %s : %s" % h)

    def get_thread(self):
        """get_threadId() -> Return current thread index"""
        return self._systems.GetCurrentThreadId()

    def set_thread(self, id):
        """set_thread(id) -> Set current thread by index"""
        self._systems.SetCurrentThreadId(id)

    def apply_threads(self, fn, tid=None):
        """apply_threads(fn, id=None) -> Run a function in all thread ctx"""
        if tid is None:
            ids, trash = self._systems.GetThreadIdsByIndex()
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
        ids, sysids = self._systems.GetThreadIdsByIndex()
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
        for i, t in enumerate(self.thread_list()):
            print("%d: %d - %08x %s" % (i, t[0], t[1], t[2]))

    def teb_addr(self):
        """teb_addr() -> return teb address for current thread"""
        return self._systems.GetCurrentThreadDataOffset()

    def teb(self, addr=None):
        """teb() -> display teb"""
        if addr is None:
            addr = self.teb_addr()
        id = self._symbols.GetTypeId("ntdll!_TEB")
        self._symbols.OutputTypedDataVirtual(
                        addr, self.mod.ntdll.addr, id, 0)

    def peb_addr(self):
        """peb_addr() -> return peb address"""
        return self._systems.GetCurrentProcessDataOffset()

    def peb(self, addr=None):
        """peb() -> display peb"""
        if addr is None:
            addr = self.peb_addr()

        id = self._symbols.GetTypeId("ntdll!_PEB")
        self._symbols.OutputTypedDataVirtual(
                        addr, self.mod.ntdll.addr, id, 0)

    def backtrace_list(self):
        return self._control.GetStackTrace()

    def backtrace(self):
        """backtrace(self) -> current backtrace"""
        print("   %-16s %-16s %s" % ("Stack", "Return", "Instruction"))
        fmt = "%02d %016x %016x %s"
        for frame in self.backtrace_list():
            print(fmt % (frame.FrameNumber,
                        frame.StackOffset,
                        frame.ReturnOffset,
                        self.get_name_by_offset(frame.InstructionOffset)))

    def module_list(self):
        """module_list() -> returns the module list of tuples (name, modparams)"""
        return self.mod.modules()

    def lm(self):
        for m in self.module_list():
            print("%016x %016x  %s" % (m[1].Base, m[1].Base + m[1].Size, m[0][0]))

    def exports(self, name):
        """exports(name) -> returns the export list for name"""
        self.mod[name].exports()

    def imports(self, name):
        """imports(name) -> returns the import list for name"""
        self.mod[name].imports()
