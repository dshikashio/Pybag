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

