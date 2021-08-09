import unittest
from dbgeng.idebugclient import DebugClient,DbgEng

class TestIDebugClient(unittest.TestCase):
    def test_init(self):
        cli = DebugClient()


if __name__ == '__main__':
    unittest.main()
