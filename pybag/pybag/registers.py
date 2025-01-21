from collections.abc import Sequence, Mapping
import ctypes

import pybag.dbgeng as DbgEng
from pybag.dbgeng import exception


class Registers(Sequence, Mapping):
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
        return self._get_register(name)

    def __setattr__(self, name, value):
        self._set_register(name, value)

    def __repr__(self):
        return str(self.register_dict())

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
            try:
                val  = self._get_register_by_index(i)
            except:
                val = 'unknown'
            itemlist.append((name, val))
        return itemlist

    def register_dict(self):
        return dict(self.register_list())

    def get_sp(self):
        return self._reg.GetStackOffset()

    def get_pc(self):
        return self._reg.GetInstructionOffset()

    def get_bp(self):
        return self._reg.GetFrameOffset()

