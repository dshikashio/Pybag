import itertools
import queue
import struct
import sys
import threading

from dbgeng import core as DbgEng
from dbgeng.idebugclient    import DebugClient
from dbgeng.breakpoints     import Breakpoints
from dbgeng.callbacks       import EventHandler
from dbgeng.callbacks       import DbgEngCallbacks
from dbgeng.modules         import Modules
from dbgeng.registers       import Registers

import dbgeng.exception as exception
import dbgeng.util as util

class WorkItem(object):
    def __init__(self, task, timeout):
        self.task = task
        self.timeout = timeout

def EventThread(Dbg, Ev, WorkQ):
    Dbg._client         = DebugClient()
    Dbg._advanced       = Dbg._client.IDebugAdvanced()
    Dbg._control        = Dbg._client.IDebugControl()
    Dbg._registers      = Dbg._client.IDebugRegisters()
    Dbg._dataspaces     = Dbg._client.IDebugDataSpaces()
    Dbg._symbols        = Dbg._client.IDebugSymbols()
    Dbg._systems        = Dbg._client.IDebugSystemObjects()

    Dbg.reg             = Registers(Dbg._registers)
    Dbg.mod             = Modules(Dbg._dataspaces, Dbg._symbols)
    Dbg.events          = EventHandler()
    Dbg.breakpoints     = Breakpoints(Dbg._control)
    Dbg.callbacks       = DbgEngCallbacks(Dbg.events, sys.stdout.write)

    Dbg.events.breakpoint(Dbg.breakpoints)
    Dbg.events.engine_state(verbose=True)
    Dbg.events.debuggee_state(verbose=True)
    Dbg.events.session_status(verbose=True)
    Dbg.events.system_error(verbose=True)
    Dbg._client.SetOutputCallbacks(Dbg.callbacks)
    Dbg._client.SetEventCallbacks(Dbg.callbacks)

    Ev.set()

    while True:
        item = WorkQ.get(True)
        if item.task == 'WaitForEvent':
            try:
                print("Call WaitForEvent {}".format(item.timeout))
                Dbg._control.WaitForEvent(item.timeout)
            except Exception as ex:
                print("WaitForEvent exception", ex)
        elif item.task == 'DispatchCallbacks':
            try:
                print("Call DispatchCallbacks {}".format(item.timeout))
                Dbg._client.DispatchCallbacks(item.timeout)
            except Exception as ex:
                print("DispatchCallbacks exception", ex)

        WorkQ.task_done()
        Ev.set()


class Debugger(object):
    def __init__(self):
        self._ev = threading.Event()
        self._q = queue.Queue()
        self._thread = threading.Thread(target=EventThread, 
                                        args=(self, self._ev, self._q))
        self._thread.daemon = True
        self._thread.start()
        self._ev.wait(5)
        if not self._ev.is_set():
            raise RuntimeError

    def create(self, path, initial_break=True):
        """create(path, initial_break=True) -> Create a new process to debug"""
        self._client.CreateProcess(path, DbgEng.DEBUG_PROCESS)
        if initial_break:
            self._control.AddEngineOptions(DbgEng.DEBUG_ENGINITIAL_BREAK)
        self.wait()

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

    def _worker_wait(self, msg, timeout=-1):
        if timeout == -1:
            # python having issues on 32-bit
            timeout = 0xfffffff
            pass
        else:
            timeout *= 1000

        item = WorkItem(msg, timeout)
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
        if not self._worker_wait('WaitForEvent', timeout):
            self._control.SetInterrupt(DbgEng.DEBUG_INTERRUPT_ACTIVE)
            return False
        else:
            return True

    def cmd(self, cmdline):
        """cmd(cmdline) -> execute a windbg console command"""
        self._control.Execute(cmdline)

    def bitness(self):
        """bitness() -> Return target bitness"""
        if self._control.IsPointer64Bit():
            return '64'
        else:
            return '32'

    def read(self, addr, len=1):
        """read(addr,len) -> read len bytes from addr"""
        return self._dataspaces.ReadVirtual(addr, len)

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
            (name,disp) = self._symbols.GetNameByOffset(addr)
            name = "%s+0x%x" % (name.decode(), disp)
        except exception.E_FAIL_Error:
            name = "0x%x" % addr
        return name

    def symbol(self, name):
        """symbol(name) -> resolve a symbol"""
        return self._symbols.GetOffsetByName(name)[1]

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
        MAX_LEN=0x1000
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
        print(''.join(data).decode(enc))

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
                width,prot = bp.GetDataParameters()
                width = str(width)
                prot = {4 : 'e', 2 : 'w', 1 : 'r'}[prot] 
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
                                    DbgEng.DEBUG_BREAKPOINT_CODE,
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
                                    DbgEng.DEBUG_BREAKPOINT_DATA,
                                    windbgcmd,
                                    oneshot,
                                    passcount,
                                    threadid,
                                    size,
                                    access)

    def get_thread(self):
        """get_threadId() -> Return current thread index"""
        return self._systems.GetCurrentThreadId()

    def set_thread(self, id):
        """set_thread(id) -> Set current thread by index"""
        self._systems.SetCurrentThreadId(id)

'''
    def quiet(self):
        """quiet() -> shut dbgeng up"""
        self._client.SetOutputMask(0)

    def verbose(self, flags=DbgEng.DEBUG_OUTPUT_NORMAL):
        """verbose(flags=DEBUG_OUTPUT_NORMAL) -> allow dbgeng to output"""
        self._client.SetOutputMask(flags)
'''

d = Debugger()
d.create('c:\\windows\\notepad.exe', True)
#d.create('c:\\windows\\notepad.exe', False)

#d.cmd("r") -- takes a long time first time due to symbol resolution
#-- made worse by firewall setup
#-- setup symbols to avoid some initial pauses
