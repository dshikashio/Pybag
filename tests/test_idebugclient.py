import unittest

from pybag.dbgeng.idebugclient import DebugClient


class TestIDebugClient(unittest.TestCase):
    def test_init(self):
        cli = DebugClient()
        adv = cli.IDebugAdvanced()
        ctr = cli.IDebugControl()
        dat = cli.IDebugDataSpaces()
        reg = cli.IDebugRegisters()
        sym = cli.IDebugSymbols()
        sys = cli.IDebugSystemObjects()

if __name__ == '__main__':
    unittest.main()
