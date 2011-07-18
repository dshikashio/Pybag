import struct
import sys
from pybag import pydbgeng, pywindbg

w = pywindbg.Userdbg()
w.create('notepad.exe')
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
    data = cls.read(cls.reg.rdx, cls.reg.r8)
    pywindbg.hexdump(data, cls.reg.rdx)
    return pydbgeng.DEBUG_STATUS_GO

def rfhandler(bp, cls):
    rdx, r8 = cls.reg.rdx, cls.reg.r9
    retaddr = cls.backtrace_list()[0].ReturnOffset

    def read_print(cls, addr, lenptr):
        print "+++ ReadFile"
        try:
            count = struct.unpack("<I", cls.read(lenptr, 4))[0]
            data = cls.read(addr, min(count, 0x100))
            pywindbg.hexdump(data, addr)
        except WindowsError:
            print " No data"
        return pydbgeng.DEBUG_STATUS_GO
    def ret_handler(bp2, cls2):
        return read_print(cls2, rdx, r8)

    cls.bp(retaddr, handler=ret_handler, oneshot=True)
    return pydbgeng.DEBUG_STATUS_GO

w.bp("CreateFileW", cfhandler)
w.bp("WriteFile", wfhandler)
w.bp("ReadFile", rfhandler)
w.go()

