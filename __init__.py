import ctypes
import platform
import os


# reg query "HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots" /v KitsRoot10

if platform.architecture()[0] == '64bit':
    #dbgdir = r'C:\Program Files\Debugging Tools for Windows (x64)'
    dbgdir = r'C:\Program Files (x86)\Windows Kits\10\Debuggers\x64'
else:
    #dbgdir = r'C:\Program Files (x86)\Debugging Tools for Windows (x86)'
    dbgdir = r'C:\Program Files (x86)\Windows Kits\10\Debuggers\x86'

# preload these to get correct DLLs loaded
ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgmodel.dll'))
ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbghelp.dll'))
ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgeng.dll'))

del platform
del os
del dbgdir
del ctypes

from pywindbg import *

