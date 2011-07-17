import struct
import sys
from pybag import Userdbg, pydbgeng, hexdump

w = Userdbg()
pids = w.pids_by_name('notepad.exe')
if not pids:
    print "notepad.exe not running"
    sys.exit(-1)

print "attaching to %d" % pids[0]
w.attach(pids[0])
w.quiet()


def cfhandler(bp, cls):
    print "+++ CreateFileW"
    if cls.is64bit():
        cls.ds(cls.reg.rcx, True)
    else:
        addr = cls.readptr(w.reg.esp + 4)[0]
        cls.ds(addr, True)
    return pydbgeng.DEBUG_STATUS_GO

def wfhandler(bp, cls):
    print "+++ WriteFile"
    if cls.is64bit():
        addr, len = cls.reg.rdx, cls.reg.r8
    else:
        addr = cls.readptr(w.reg.esp + 8)[0]
        len  = cls.readptr(w.reg.esp + 12)[0]
    data = cls.read(addr, len)
    hexdump(data, addr)
    return pydbgeng.DEBUG_STATUS_GO

def rfhandler(bp, cls):
    if cls.is64bit():
        rdx, r8 = cls.reg.rdx, cls.reg.r9
    else:
        rdx = cls.readptr(w.reg.esp + 8)[0]
        r8  = cls.readptr(w.reg.esp + 16)[0]

    retaddr = cls.backtrace_list()[0].ReturnOffset

    def read_print(cls, addr, lenptr):
        print "+++ ReadFile"
        try:
            count = struct.unpack("<I", cls.read(lenptr, 4))[0]
            data = cls.read(addr, min(count, 0x100))
            hexdump(data, addr)
        except WindowsError:
            print " No data"
        return pydbgeng.DEBUG_STATUS_GO
    def ret_handler(bp2, cls2):
        return read_print(cls2, rdx, r8)

    cls.bp(retaddr, handler=ret_handler, oneshot=True)
    return pydbgeng.DEBUG_STATUS_GO


def msgboxhandler(bp, cls):
    if cls.is64bit():
        ptext,pcaption = cls.reg.rdx, cls.reg.r8
    else:
        ptext = cls.readptr(w.reg.esp + 4)[0]
        pcaption = cls.readptr(w.reg.esp + 8)[0]
    print "+++ MsgboxA"
    print "Caption: ", cls.ds(pcaption)
    print "Text   : ", cls.ds(ptext)


w.bp("CreateFileW", cfhandler)
w.bp("WriteFile", wfhandler)
w.bp("ReadFile", rfhandler)
w.go()

