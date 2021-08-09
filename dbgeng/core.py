from ctypes                 import *
from comtypes               import HRESULT, CoClass, GUID
from comtypes.automation    import IID
from comtypes.hresult       import S_OK

DBGENG_DLL = r'C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\dbgeng.dll'

try:
    from comtypes.gen.DbgEng import *
except:
    import comtypes.client
    tlb = os.path.join(util.module_locator(), 'tlb', 'dbgeng.tlb')
    comtypes.client.GetModule(tlb)
    from comtypes.gen.DbgEng import *

from .exception import check_err

def DebugCreate():
    dbgeng = windll.LoadLibrary(DBGENG_DLL)
    proto  = WINFUNCTYPE(HRESULT, POINTER(IID), POINTER(POINTER(IDebugClient7)))
    create = proto(("DebugCreate", dbgeng)) 
    client = POINTER(IDebugClient7)()
    cliptr = POINTER(POINTER(IDebugClient7))(client)
    hr = create(IDebugClient7._iid_, cliptr)
    check_err(hr)
    return client

def DebugConnect(options):
    dbgeng  = windll.LoadLibrary(DBGENG_DLL)
    proto   = WINFUNCTYPE(HRESULT, c_char_p, POINTER(IID), POINTER(POINTER(IDebugClient7)))
    connect = proto(("DebugConnect", dbgeng)) 
    client  = POINTER(IDebugClient7)()
    cliptr  = POINTER(POINTER(IDebugClient7))(client)
    hr = connect(options.encode(), IDebugClient7._iid_, cliptr)
    check_err(hr)
    return client