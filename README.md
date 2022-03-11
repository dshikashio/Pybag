# Pybag
> Python bindings for Microsoft DbgEng

Pybag provides helper functions on top of Python bindings for Microsoft Debug Engine to facilitate Windows native debugging.

[![Build Master](https://github.com/dshikashio/Pybag/actions/workflows/python-package.yml/badge.svg?branch=master)](https://github.com/dshikashio/Pybag/actions/workflows/python-package.yml)

## Installation

Windows:

```sh
python setup.py install
```

## Usage example

Use Python's repl as a command shell if desired. Ctrl-c will break-in to the debugger if you are in a wait() call.

### Local user debugging
```python
from pybag import *

def handler(bp, dbg):
    print("*********** BREAK")
    return DbgEng.DEBUG_STATUS_GO

dbg = UserDbg()
dbg.create("notepad.exe")
dbg.bp("Kernel32!WriteFile", handler)
dbg.go()
```
### Remote user debugging
```python
from pybag import *

def handler(bp, dbg):
    print("*********** BREAK")
    return DbgEng.DEBUG_STATUS_GO

dbg = UserDbg()
d.connect("tcp:server=192.168.1.10,port=5555")
dbg.create("notepad.exe")
dbg.bp("Kernel32!WriteFile", handler)
dbg.go()
```

### Remote kernel debugging
```python
from pybag import *

k = KernelDbg()
k.attach("net:port=50000,key=1.2.3.4")
```


## Release History
* 2.2.5
  * Fixed be command
  * Better search for Windbg install (and DLLs)
* 2.2.4
  * Fixed lint issues
* 2.2.3 
  * Bug fix in WriteVirtual  
* 2.2.2
  * Python bindings rewritten to use comtypes
  * Moved to Python 3
  * Support multiple debugging uses
    * Local user debugging
    * Remote kernel debugging
    * Remote user debugging using dbgsrv


## Meta

Distributed under the MIT license. See ``LICENSE`` for more information.

[https://github.com/dshikashio](https://github.com/dshikashio/)
