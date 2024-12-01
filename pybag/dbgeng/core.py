import os
from ctypes                 import *
from comtypes               import HRESULT
from comtypes.automation    import IID

DBGENG_DLL = 'dbgeng.dll'

try:
    from comtypes.gen.DbgEng import *
except:
    import comtypes.client
    tlb = os.path.join(util.module_locator(), 'tlb', 'dbgeng.tlb')
    comtypes.client.GetModule(tlb)
    from comtypes.gen.DbgEng import *

# Fix, comtypes made this -1
WAIT_INFINITE = 0xffffffff

# Some defines that comtypes wouldn't handle for us
DEBUG_STATUS_INSIDE_WAIT  = 0x100000000
DEBUG_STATUS_WAIT_TIMEOUT = 0x200000000

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


"""
Utilities that need DbgEng
"""

def str_execution_status(status):
    status_map = {DEBUG_STATUS_BREAK         : "BREAK",
                  DEBUG_STATUS_GO            : "GO",
                  DEBUG_STATUS_STEP_BRANCH   : "STEP_BRANCH",
                  DEBUG_STATUS_STEP_INTO     : "STEP_INTO",
                  DEBUG_STATUS_STEP_OVER     : "STEP_OVER",
                  DEBUG_STATUS_NO_DEBUGGEE   : "NO_DEBUGGEE"}
    try:
        return status_map[status]
    except KeyError:
        return "UNKNOWN - {}".format(status)
