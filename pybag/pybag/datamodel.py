#
# Bits borrowed from: https://github.com/NationalSecurityAgency/ghidra/blob/master/Ghidra/Debug/Debugger-agent-dbgeng/src/main/py/src/ghidradbg/util.py
#
## ###
#  IP: GHIDRA
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#       http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
##

import functools

def split_path(pathString):
    list = []
    segs = pathString.split(".")
    for s in segs:
        if s.endswith("]"):
            index = s.index("[")
            list.append(s[:index])
            list.append(s[index:])
        else:
            list.append(s)
    return list

def checker(f):
    @functools.wraps(f)
    def wrapper(self, *args, **kwargs):
        if self._access != None:
            return f(self, *args, **kwargs)
        else:
            return None
    return wrapper




"""

def objchecker(f):
    @functools.wraps(f)
    def wrapper(self, *args, **kwargs):
        if self.obj != None:
            return f(self, *args, **kwargs)
        else:
            return None
    return wrapper

class DataObject(object):
    def __init__(self, obj):
        self.obj = obj

    @objchecker
    def get_attributes(self):
        return self.obj.GetAttributes()

    @objchecker
    def get_elements(self):
        return self.obj.GetElements()

    @objchecker
    def get_kind(self):
        kind = self.obj.GetKind()
        if kind is None:
            return None
        return self.obj.GetKind().value

    @objchecker
    def get_type(self):
        return self.obj.GetTypeKind()

    @objchecker
    def get_value(self):
        return self.obj.GetValue()

    @objchecker
    def get_intrinsic_value(self):
        return self.obj.GetIntrinsicValue()

    @objchecker
    def get_target_info(self):
        return self.obj.GetTargetInfo()

    @objchecker
    def get_type_info(self):
        return self.obj.GetTypeInfo()

    @objchecker
    def get_name(self):
        return self.obj.GetName().value

    @objchecker
    def to_display_string(self):
        return self.obj.ToDisplayString()

    @objchecker
    def get_location(self):
        try:
            loc = self.obj.GetLocation()
            if loc is None:
                return None
            return hex(loc.Offset)
        except:
            return None
"""

class DataModel(object):
    def __init__(self, Dbg):
        try:
            from pybag.dbgmodel import DbgMod
            from pybag.dbgmodel.ihostdatamodelaccess import HostDataModelAccess
            self._access = HostDataModelAccess(Dbg._client._cli.QueryInterface(interface=DbgMod.IHostDataModelAccess))
            self._mgr,self._host = self._access.GetDataModel()
            self._root = self._mgr.GetRootNamespace()
        except Exception as e:
            print(e)
            import traceback
            traceback.print_exc()
            self._access = None

    @checker
    def get_object(self, relpath=''):
        pathstr = 'Debugger'
        if relpath:
            pathstr += '.' + relpath
        path = split_path(pathstr)
        return self._root.GetOffspring(path)

