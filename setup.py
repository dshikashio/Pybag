#from distutils.core import setup, Extension
from setuptools import setup, find_packages, Extension
import platform
import os


arch = platform.architecture()[0]


#SDK_DIR = r'C:\Program Files\Debugging Tools for Windows (x64)\sdk'
#SDK_DIR_X64 = r'C:\Program Files (x86)\Windows Kits\8.1'
#SDK_DIR_X86 = r'C:\Program Files\Windows Kits\8.1'


#if not os.path.isdir(SDK_DIR_X64):
#    if not os.path.isdir(SDK_DIR_X86):
#        raise RuntimeError('Windows SDK not found!')
#    else:
#        SDK_DIR = SDK_DIR_X86
#else:
#    SDK_DIR = SDK_DIR_X64

# XXX - Lookup from the registry
SDK_DIR = r'C:\Program Files (x86)\Windows Kits\10'    
if not os.path.isdir(SDK_DIR):
    raise RuntimeError('Windows 10 Kit not found!')
        
INC_DIRS = [
        os.path.join(SDK_DIR, 'Debuggers', 'inc'),
        os.path.join(SDK_DIR, 'Include', '10.0.15063.0', 'um'),
        os.path.join(SDK_DIR, 'Include', '10.0.15063.0', 'shared'),
        ]

if arch == '64bit':
    LIB_DIR = os.path.join(SDK_DIR, 'Debuggers', 'lib', 'x64')
else:
    LIB_DIR = os.path.join(SDK_DIR, 'Debuggers', 'lib', 'x86')


setup(
    name='pybag',
    version='1.1.0',
    packages=['pybag'],
    package_dir = {'pybag' : ''},
    install_requires = [
        'capstone-windows'
    ],
    ext_modules = [
        Extension('pybag.pydbgeng', 
            sources = [
             'python-dbgeng/constants.cpp',
             'python-dbgeng/debugadvanced.cpp',
             'python-dbgeng/debugbreakpoint.cpp',
             'python-dbgeng/debugclient.cpp',
             'python-dbgeng/debugcontrol.cpp',
             'python-dbgeng/debugdataspaces.cpp',
             'python-dbgeng/debugregisters.cpp',
             'python-dbgeng/debugsymbolgroup.cpp',
             'python-dbgeng/debugsymbols.cpp',
             'python-dbgeng/debugsystemsobject.cpp',
             'python-dbgeng/eventcallbacks.cpp',
             'python-dbgeng/exceptions.cpp',
             'python-dbgeng/outputcallbacks.cpp',
             'python-dbgeng/pydbgeng.cpp',
             'python-dbgeng/winstructs.cpp'],
            include_dirs=INC_DIRS,
            library_dirs=[LIB_DIR],
            libraries=['dbgeng'],
            )
    ],
    py_modules=['pybag.pywindbg', 'pybag.pefile'],
    scripts = ['examples/filewatch.py'],
)

