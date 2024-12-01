from ctypes import *
import os

try:
    from comtypes.gen import DbgEng
except:
    import comtypes.client
    tlb = os.path.join(util.module_locator(), 'tlb', 'dbgeng.tlb')
    comtypes.client.GetModule(tlb)
    from comtypes.gen import DbgEng


class DebugValue(object):
    def __init__(self, dv=None):
        if dv == None:
            self.dv = DbgEng._DEBUG_VALUE()
        else:
            self.dv = dv

    def __getattr__(self, name):
        if name == 'value':
            return self.get_value()
        elif name == 'type':
            return self.get_type()
        else:
            object.__getattr__(self, name)

    def __setattr__(self, name, value):
        if name == 'value':
            self.set_value(value)
        else:
            object.__setattr__(self, name, value)

    def _get_type_name(self, typeid):
        # XXX - Should we care about more types?
        mapping = {
            0  : 'RawBytes',
            1  : 'I8',
            2  : 'I16',
            3  : 'I32',
            4  : 'I64.I64',
            5  : 'F32',
            6  : 'F64',
            7  : 'F80Bytes',
            8  : 'F82Bytes',
            9  : 'F128Bytes',
            10 : 'VI32',
            11 : 'VI64'
        }
        return mapping[typeid]

    def get_type(self):
        return self._get_type_name(self.dv.Type)

    def get_value(self):
        name = self._get_type_name(self.dv.Type)
        if name == 'I64.I64':
            return self.dv.u.I64.I64
        else:
            return getattr(self.dv.u, name)

    def set_value(self, value, typeid=None):
        if typeid is None:
            if isinstance(value, int):
                self.dv.Type = 4
                self.dv.u.I64.I64 = value
            elif isinstance(value, float):
                self.dv.Type = 6
                self.dv.u.F64 = value
        else:
            name = self._get_type_name(typeid)
            if name == 'I64.I64':
                self.dv.u.I64.I64 = value
            else:
                setattr(self.dv.u, name, value)
            self.dv.Type = typeid

