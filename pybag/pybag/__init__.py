import ctypes
import platform
import os
import winreg

__all__ = ['UserDbg', 'KernelDbg', 'CrashDbg', 'DbgEng']


def get_arch():
    if platform.architecture()[0] == '64bit':
        return 'amd64'
    else:
        return 'x86'


def append_arch(dbgroot):
    if platform.architecture()[0] == '64bit':
        return os.path.join(dbgroot, r'Debuggers\x64')
    else:
        return os.path.join(dbgroot, r'Debuggers\x86')

def find_dbgdir():
    dbgdir = os.getenv('WINDBG_DIR')
    if dbgdir is not None and os.path.exists(dbgdir):
        return dbgdir
    try:
        roots_key = winreg.OpenKey(
            winreg.HKEY_LOCAL_MACHINE,
            r'SOFTWARE\Microsoft\Windows Kits\Installed Roots')
        dbgroot = winreg.QueryValueEx(roots_key, 'KitsRoot10')[0]
        dbgdir = append_arch(dbgroot)
        if os.path.exists(dbgdir):
            return dbgdir
    except FileNotFoundError:
        pass
    default_roots = [r'C:\Program Files\Windows Kits\10',
                     r'C:\Program Files (x86)\Windows Kits\10']
    for dbgroot in default_roots:
        dbgdir = append_arch(dbgroot)
        if os.path.exists(dbgdir):
            return dbgdir

    from pybag.scripts.install_windbg import install_target
    dbgdir = os.path.join(install_target, get_arch())
    print(dbgdir)
    if os.path.exists(dbgdir):
        return dbgdir

    raise RuntimeError("Windbg install directory not found!")


dbgdir = find_dbgdir()


# preload these to get correct DLLs loaded
try:
    ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgmodel.dll'))
except:
    pass

ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbghelp.dll'))
ctypes.windll.LoadLibrary(os.path.join(dbgdir, 'dbgeng.dll'))

del platform
del os
del dbgdir
del ctypes
del winreg

#from pybag import dbgeng
from .crashdbg   import CrashDbg
from .kerneldbg  import KernelDbg
from .userdbg    import UserDbg

