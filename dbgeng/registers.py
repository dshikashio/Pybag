import collections
import ctypes

from . import core as DbgEng
from . import exception
from .dbgengstructs import DebugValue

class Registers(collections.Sequence, collections.Mapping):
    def __init__(self, DebugRegistersObj):
        self.__dict__['_reg'] = DebugRegistersObj

    def _get_register_idx(self, name):
        return self._reg.GetIndexByName(name)

    def _get_register(self, name):
        try:
            idx = self._get_register_idx(name)
            return self._get_register_by_index(idx)
        except exception.E_NOINTERFACE_Error:
            raise AttributeError(name)

    def _get_register_by_index(self, index):
        return self._reg.GetValue(index)

    def _set_register(self, name, value):
        try:
            idx = self._get_register_idx(name)
            self._set_register_by_index(idx, value)
        except exception.E_NOINTERFACE_Error:
            raise AttributeError(name)

    def _set_register_by_index(self, index, value):
        if not isinstance(value, int):
            raise ValueError('value must be of type int')
        try:
            self._reg.SetValue(index, value)
        except exception.E_NOINTERFACE_Error:
            raise KeyError(index)

    def __len__(self):
        return self._reg.GetNumberRegisters()

    def __iter__(self):
        return iter(self.register_list())

    def __contains__(self, item):
        try:
            self.__getitem__(item)
            return True
        except:
            return False

    def __getitem__(self, key):
        if isinstance(key, str):
            try:
                return self._get_register(key)
            except AttributeError:
                raise KeyError(key)
        else:
            try:
                return self._get_register_by_index(key)
            except exception.E_NOINTERFACE_Error:
                raise KeyError(key)

    def __setitem__(self, key, value):
        if isinstance(key, str):
            try:
                self._set_register(key, value)
            except AttributeError:
                raise KeyError(key)
        else:
            try:
                self._set_register_by_index(key, value)
            except exception.E_NOINTERFACE_Error:
                raise KeyError(key)

    def __getattr__(self, name):
        if name in self.__dict__:
            return self.__dict__[name]
        else:
            return self._get_register(name) 

    def __setattr__(self, name, value):
        if name in self.__dict__:
            self.__dict__[name] = value
        else:
            self._set_register(name, value) 

    def describe(self, key):
        if isinstance(key, str):
            try:
                index = self._get_register_idx(key)
            except exception.E_NOINTERFACE_Error:
                raise KeyError(key)
        else:
            index = key
        return self._reg.GetDescription(index)

    def output(self, source=DbgEng.DEBUG_REGSRC_DEBUGGEE, all=False):
        if all:
            flags = DbgEng.DEBUG_REGISTERS_ALL
        else:
            flags = DbgEng.DEBUG_REGISTERS_DEFAULT
        self._reg.OutputRegisters2(source, flags)

    def register_list(self):
        itemlist = []
        for i in range(0, len(self)):
            name = self._reg.GetDescription(i)[0]
            val  = self._get_register_by_index(i)
            itemlist.append((name, val))
        return itemlist

    def get_sp(self):
        return self._reg.GetStackOffset()

    def get_pc(self):
        return self._reg.GetInstructionOffset()

    def get_bp(self):
        return self._reg.GetFrameOffset()