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

    if cls.is64bit():
        addr = cls.reg.rdx
        data = cls.read(addr, cls.reg.r8)
    else:
        addr = cls.readptr(w.reg.esp + 8)[0]
        dlen = cls.readptr(w.reg.esp + 12)[0]
        data = cls.read(addr, dlen)
    pywindbg.hexdump(data, addr)
    return pydbgeng.DEBUG_STATUS_GO

def rfhandler(bp, cls):

    if cls.is64bit():
        data_addr, data_len = cls.reg.rdx, cls.reg.r9
    else:
        data_addr = cls.readptr(w.reg.esp + 8)[0]
        data_len = cls.readptr(w.reg.esp + 16)[0]

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
        return read_print(cls2, data_addr, data_len)

    cls.bp(retaddr, handler=ret_handler, oneshot=True)
    return pydbgeng.DEBUG_STATUS_GO

w.bp("kernel32!CreateFileW", cfhandler)
w.bp("kernel32!WriteFile", wfhandler)
w.bp("kernel32!ReadFile", rfhandler)
w.go()
