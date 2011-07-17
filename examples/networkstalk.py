import random
import sys
import pydbgeng
import pywindbg

w = pywindbg.Userdbg()
w.quiet()
pids = w.pids_by_name('nc.exe')
if not pids:
    print "nc.exe not running"
    sys.exit(-1)

print "attaching to %d" % pids[0]
w.attach(pids[0])

def send_handler(bp, cls):
    print "+++ Send"
    if cls.is64bit():
        addr, len = cls.reg.rdx, cls.reg.r8
    else:
        addr = cls.readptr(w.reg.esp + 8)[0]
        len  = cls.readptr(w.reg.esp + 12)[0]
    data = cls.read(addr, len)
    pywindbg.hexdump(data, addr)
    return pydbgeng.DEBUG_STATUS_GO

def recv_handler(bp, cls):
    if cls.is64bit():
        buf = cls.reg.rdx
    else:
        buf = cls.readptr(w.reg.esp + 8)[0]
    retaddr = cls.backtrace_list()[0].ReturnOffset

    def read_print(cls, buf):
        print "+++ Recv"
        if cls.is64bit():
            rlen = cls.reg.rax
        else:
            rlen = cls.reg.eax
        if rlen not in (-1, 0xffffffff):
            try:
                data = cls.read(buf, rlen)
                pywindbg.hexdump(data, buf)
            except WindowsError:
                print " No data"
        return pydbgeng.DEBUG_STATUS_GO
    def ret_handler(bp2, cls2):
        return read_print(cls2, buf)

    cls.bp(retaddr, handler=ret_handler, oneshot=True)
    return pydbgeng.DEBUG_STATUS_GO

def send_garbage(bp, cls):

    print "+++ Send"
    if cls.is64bit():
        addr, len = cls.reg.rdx, cls.reg.r8
    else:
        addr = cls.readptr(w.reg.esp + 8)[0]
        len  = cls.readptr(w.reg.esp + 12)[0]
    data = cls.read(addr, len)
    print "Real Data"
    pywindbg.hexdump(data, addr)
    data = ''.join(chr(random.randint(0x41, 0x7f))
                    for i in range(len))
    print "New Data"
    pywindbg.hexdump(data, addr)
    cls.write(addr, data)
    return pydbgeng.DEBUG_STATUS_GO

#w.bp("send", send_handler)
w.bp("send", send_garbage)
w.bp("recv", recv_handler)
w.go()


