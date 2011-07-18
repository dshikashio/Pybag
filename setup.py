from distutils.core import setup, Extension
import platform


arch = platform.architecture()[0]
if arch == '64bit':
    SDK_LIB = 'dependencies/sdk/lib/amd64'
    SDK_BIN = ['dbgtools-x64/*.dll',
               'dbgtools-x64/winext/*.dll',
               'dbgtools-x64/winxp/*.dll']
    DLL_BIN = ['dbgtools-x64/dbghelp.dll', 'dbgtools-x64/dbgeng.dll']
else:
    SDK_LIB = 'dependencies/sdk/lib/i386'
    SDK_BIN = ['dbgtools-x86/*.dll',
               'dbgtools-x86/w2kchk/*.dll',
               'dbgtools-x86/w2kfre/*.dll',
               'dbgtools-x86/winext/*.dll',
               'dbgtools-x86/winxp/*.dll']
    DLL_BIN = ['dbgtools-x86/dbghelp.dll', 
               'dbgtools-x86/dbgeng.dll']

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
            include_dirs=['dependencies/sdk/inc'],
            library_dirs=[SDK_LIB],
            libraries=['dbgeng'],
            )
    ],
    py_modules=['pybag.pywindbg', 'pybag.pefile'],
    package_data = {'pybag' : SDK_BIN},
    data_files = [('', DLL_BIN)],
    scripts = ['bin/filewatch.py'],
)

