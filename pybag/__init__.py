import ctypes
import platform
import os

__all__ = ['UserDbg', 'KernelDbg', 'CrashDbg', 'DbgEng']

# reg query "HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots" /v KitsRoot10

if platform.architecture()[0] == '64bit':
    dbgdirs = [r'C:\Program Files\Windows Kits\10\Debuggers\x64',
               r'C:\Program Files (x86)\Windows Kits\10\Debuggers\x64']
else:
    dbgdirs = [r'C:\Program Files\Windows Kits\10\Debuggers\x86',
               r'C:\Program Files (x86)\Windows Kits\10\Debuggers\x86']
dbgdir = None
for _dir in dbgdirs:
    if os.path.exists(_dir):
        dbgdir = _dir
        break

if not dbgdir:
    raise RuntimeError("Windbg install directory not found!")

# preload these to get correct DLLs loaded
try:
    ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgmodel.dll'))
except:
    pass

ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbghelp.dll'))
ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgeng.dll'))

del platform
del os
del _dir
del dbgdir
del dbgdirs
del ctypes

from .pydbg      import DbgEng
from .crashdbg   import CrashDbg
from .kerneldbg  import KernelDbg
from .userdbg    import UserDbg

