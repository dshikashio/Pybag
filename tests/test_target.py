import unittest
import os

from pybag import DbgEng, UserDbg

target = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'target', 'target.exe')

class TestTarget(unittest.TestCase):
    def test_init(self):
        dbg = UserDbg()
        dbg.create(target)
        dbg.r()


if __name__ == '__main__':
    unittest.main()
