import unittest
import os
import pprint
import subprocess

from pybag import DbgEng, UserDbg

target1 = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'target', 'target.exe')
target2 = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'target', 'target2.exe')

class TestTargetCreate(unittest.TestCase):
    def setUp(self):
        dbg = UserDbg()
        dbg.create(target1)
        self.dbg = dbg

    def tearDown(self):
        self.dbg.terminate()
        self.dbg = None

    def test_base_functions(self):
        self.dbg.r()
        self.dbg.address()
        self.dbg.disasm()
        self.dbg.handles()
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
        self.proc = subprocess.Popen([target2])

    def tearDown(self):
        self.proc.terminate()
        self.dbg = None

    def test_attach_detach(self):
        self.dbg.attach(self.proc.pid)
        self.assertEqual(self.dbg.pid, self.proc.pid)
        self.assertEqual(self.dbg.exec_status(), 'BREAK')
        self.dbg.detach()

class TestTargetAttach2(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()
        self.proc = subprocess.Popen([target2])

    def tearDown(self):
        self.proc.terminate()
        self.dbg = None

    def test_attach_terminate(self):
        self.dbg.attach(self.proc.pid)
        self.dbg.terminate()

class TestBasic(unittest.TestCase):
    def setUp(self):
        self.dbg = UserDbg()

    def tearDown(self):
        self.dbg = None

    def test_ps(self):
        self.dbg.ps()

    def test_pids_by_name(self):
        pids = self.dbg.pids_by_name("svchost.exe")
        pprint.pprint(pids)

    def test_exec_status(self):
        self.assertEqual(self.dbg.exec_status(), 'NO_DEBUGGEE')

if __name__ == '__main__':
    unittest.main()
