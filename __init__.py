import platform
import os
import sys

base = os.path.dirname(os.path.realpath(__file__))
arch = platform.architecture()[0]
if arch == '64bit':
    dbgdir = os.path.join(base, 'dbgtools-x64')
else:
    dbgdir = os.path.join(base, 'dbgtools-x86')

sys.path.append(dbgdir)
os.environ['PATH'] = dbgdir + ';' + os.environ['PATH']

del arch
del base
del dbgdir
del platform
del os
del sys

from pywindbg import *

