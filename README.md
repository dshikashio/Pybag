# Pybag
> Python bindings for Microsoft DbgEng

Pybag provides helper functions on top of Python bindings for Microsoft Debug Engine to facilitate Windows native debugging.

![](header.png)

## Installation

Windows:

```sh
python setup.py install
```

## Usage example

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

* 2.2.0
  * Python bindings rewritten to use comtypes
  * Moved to Python 3
  * Support multiple debugging uses
    * Local user debugging
    * Remote kernel debugging
    * Remote user debugging using dbgsrv


## Meta

Distributed under the MIT license. See ``LICENSE`` for more information.

[https://github.com/dshikashio](https://github.com/dshikashio/)
