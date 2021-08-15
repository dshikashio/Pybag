import unittest

from pybag import DbgEng
from pybag.dbgeng.idebugclient import DebugClient


class TestIDebugClient(unittest.TestCase):
    def test_init(self):
        cli = DebugClient()


if __name__ == '__main__':
    unittest.main()
