from distutils.core import setup, Extension
import platform
import os


arch = platform.architecture()[0]

SDK_DIR = r'C:\Program Files\Debugging Tools for Windows (x64)\sdk'
INC_DIR = os.path.join(SDK_DIR, 'inc')
if arch == '64bit':
    LIB_DIR = os.path.join(SDK_DIR, 'lib', 'amd64')
else:
    LIB_DIR = os.path.join(SDK_DIR, 'lib', 'i368')


setup(
    name='pybag',
    version='0.1.0',
    packages=['pybag'],
    package_dir = {'pybag' : ''},
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
            include_dirs=[INC_DIR],
            library_dirs=[LIB_DIR],
            libraries=['dbgeng'],
            )
    ],
    py_modules=['pybag.pywindbg', 'pybag.pefile'],
    scripts = ['examples/filewatch.py'],
)

