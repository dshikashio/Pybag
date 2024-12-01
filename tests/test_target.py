import unittest
import os
import pprint
import subprocess

from pybag.pybag import UserDbg

target1 = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'target', 'target.exe')
target2 = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'target', 'target2.exe')


class TestTargetCreate(unittest.TestCase):
    def setUp(self):
        dbg = UserDbg()
        dbg.cmd(".sympath SRV*c:\\sym")
        dbg.create(target1)
        self.dbg = dbg

    def tearDown(self):
        self.dbg.terminate()
        self.dbg.Release()

    def test_base_functions(self):
        self.dbg.r()
        self.dbg.address()
        self.dbg.disasm()
        #self.dbg.handles()
        self.dbg.threads()
        self.dbg.teb()
        self.dbg.peb()
        self.dbg.backtrace()
        self.dbg.lm()
        self.dbg.imports('target')
        self.dbg.exports('kernel32')
        self.assertEqual(self.dbg.bitness(), '64')

class TestTargetAttach1(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()
        self.dbg.cmd(".sympath SRV*c:\\sym")
        self.proc = subprocess.Popen([target2])

    def tearDown(self):
        self.proc.terminate()
        self.dbg.Release()

    def test_attach_detach(self):
        self.dbg.attach(self.proc.pid)
        self.assertEqual(self.dbg.pid, self.proc.pid)
        self.assertEqual(self.dbg.exec_status(), 'BREAK')
        self.dbg.detach()


class TestTargetAttach2(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()
        self.dbg.cmd(".sympath SRV*c:\\sym")
        self.proc = subprocess.Popen([target2])

    def tearDown(self):
        self.proc.terminate()
        self.dbg.Release()

    def test_attach_terminate(self):
        self.dbg.attach(self.proc.pid)
        self.dbg.terminate()


class TestBasic(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()
        self.dbg.cmd(".sympath SRV*c:\\sym")

    def tearDown(self):
        self.dbg.Release()

    def test_processes(self):
        self.assertEqual(self.dbg.exec_status(), 'NO_DEBUGGEE')
        self.dbg.ps()
        pids = self.dbg.pids_by_name("svchost.exe")
        pprint.pprint(pids)


def breakin(bp, dbg):
    pass

def go(bp, dbg):
    return pybag.DbgEng.DEBUG_STATUS_GO

class TestStringCmd(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()
        self.dbg.cmd(".sympath SRV*c:\\sym")
        self.dbg.create(target1)
        self.dbg.cmd(".reload /f")
        self.dbg.bp("target!wmain", breakin)
        self.dbg.go()

    def tearDown(self):
        self.dbg.terminate()
        self.dbg.Release()

    def test_string(self):
        # Ascii string
        aptr = self.dbg.symbol('target!astring')
        astring = self.dbg.readptr(aptr)[0]
        self.assertEqual(self.dbg.readstr(astring), 'ascii string')
        # Wide string
        wptr = self.dbg.symbol('target!wstring')
        wstring = self.dbg.readptr(wptr)[0]
        self.assertEqual(self.dbg.readstr(wstring, True), 'wide string')

        astring = self.dbg.symbol("target!aArray")
        self.assertEqual(self.dbg.readstr(astring), 'ascii array')

        wstring = self.dbg.symbol("target!wArray")
        self.assertEqual(self.dbg.readstr(wstring, True), 'wide array')

if __name__ == '__main__':
    unittest.main()
