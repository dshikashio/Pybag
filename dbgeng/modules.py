import collections

from . import core as DbgEng
from . import exception
from . import pefile2

class Module(object):
    def __init__(self, addr, size, fnread, name=""):
        self.addr = addr
        self.size = size
        self.fnread = fnread
        self.name = name
        #self._pe = pefile2.PE(addr, size, fnread)
        self.pe = None

    def entry_point(self):
        #self._pe.OPTIONAL_HEADER.AddressOfEntryPoint
        return 0

    def export_list(self):
        #return self._pe.DIRECTORY_ENTRY_EXPORT.symbols
        return []

    def exports(self):
        for e in self.export_list():
            if e.forwarder:
                name = "%s -> %s" % (e.name, e.forwarder)
                addr = 0
            else:
                if e.address == self.addr:
                    continue
                if e.name:
                    name = "%s" % e.name
                elif e.ordinal:
                    name = "ORD(%d)" % e.ordinal
                else:
                    name = "[UNKNOWN]"
                addr = e.address
            print("%015x  %s" % (addr, name))

    def import_list(self):
        # XXX - Getting the right values here?
        #return self._pe.DIRECTORY_ENTRY_IMPORT
        return []

    def imports(self):
        for i in self.import_list():
            for fn in i.imports:
                if fn.import_by_ordinal:
                    name = "%s!ORD(%d)" % (i.dll, fn.ordinal)
                else:
                    name = "%s!%s" % (i.dll, fn.name)
                print("%015x  %s" % (fn.address, name))

    def section_list(self):
        #return self._pe.sections
        return []

    def sections(self):
        for s in self.section_list():
            print("SECTION HEADER")
            print("%16s name" % s.Name.strip('\x00'))
            print("%16x virtual size" % s.Misc_VirtualSize)
            print("%16x virtual address (%016x to %016x" % (s.VirtualAddress,
                    (s.VirtualAddress + self.addr),
                    (s.VirtualAddress + self.addr + s.Misc_VirtualSize)))
            print("%16x size of raw data" % s.SizeOfRawData)
            print("%16x flags" % s.Characteristics)
            if s.IMAGE_SCN_CNT_CODE:
                print("%16s Code" % ' ')
            if s.IMAGE_SCN_CNT_INITIALIZED_DATA:
                print("%16s Initialized Data" % ' ')
            if s.IMAGE_SCN_CNT_UNINITIALIZED_DATA:
                print("%16s Uninitialized Data" % ' ')
            prot = []
            if s.IMAGE_SCN_MEM_EXECUTE:
                prot.append("Execute")
            if s.IMAGE_SCN_MEM_READ:
                prot.append("Read")
            if s.IMAGE_SCN_MEM_WRITE:
                prot.append("Write")
            if s.IMAGE_SCN_MEM_SHARED:
                prot.append("Shared")
            print("%16s %s" % (' ', ' '.join(prot)))

    def addr_to_section(self, addr):
        for s in self.section_list():
            start = s.VirtualAddress + self.addr
            end = start + s.SizeOfRawData
            if addr >= start and addr <= end:
                return s
        return None

    def file_header(self):
        # XXX - See dumpbin
        pass

    def optional_header(self):
        # XXX - See dumpbin
        pass

    def function_list(self):
        """function_list() -> list of all functions"""
        # grab exports that point into an executable section
        flist = []
        for e in self.export_list():
            if e.forwarder:
                continue
            if e.address == self.addr:
                continue

            if e.name:
                name = "%s" % e.name
            elif e.ordinal:
                name = "ORD(%d)" % e.ordinal
            else:
                name = "[UNKNOWN]"

            s = self.addr_to_section(e.address)
            if not s:
                continue
            if s.IMAGE_SCN_MEM_EXECUTE:
                flist.append((e.address, name))
        return flist

    def functions(self):
        """functions() -> print all module functions"""
        for f in self.function_list():
            print("%015x  %s" % f)

    def seh_status(self):
        """seh_status() -> module seh status"""
        #if self._pe.OPTIONAL_HEADER.DllCharacteristics & 0x400:
        #    return (False, "NO_SEH Flag Enabled")
        # IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG = 10
        #ldconf = self._pe.OPTIONAL_HEADER.DATA_DIRECTORY[10]
        #ldconf.Size
        #ldconf.VirtualAddress
        #return (False, "LOAD_CONFIG Size 0x%x" % ldconf.Size)
        return (False, "")
        

class Modules(collections.Sequence, collections.Mapping):
    def __init__(self, DebugDataSpacesObj, DebugSymbolsObj):
        self._data = DebugDataSpacesObj
        self._sym = DebugSymbolsObj

    def _get_module(self, name):
        try:
            addr = self._sym.GetModuleByModuleName(name)[1]
            return self.get_module(addr)
        except exception.E_INVALIDARG_Error:
            raise AttributeError(name)

    def _get_module_index(self, index):
        try:
            addr = self._sym.GetModuleByIndex(i)
            return self.get_module(addr)
        except pydbgeng.DbgEngError:
            raise IndexError(index)
        except TypeError:
            return 0

    def __len__(self):
        return self._sym.GetNumberModules()[0]

    def __iter__(self):
        return iter(self.modules())

    def __contains__(self, item):
        try:
            self.__getitem__(item)
            return True
        except:
            return False

    def __getitem__(self, key):
        if isinstance(key, str):
            try:
                return self._get_module(key)
            except AttributeError:
                raise KeyError(key)
        else:
            return self._get_module_index(key)

    def __getattr__(self, name):
        if name in self.__dict__:
            return self.__dict__[name]
        else:
            return self._get_module(name)

    def addr_to_name(self, addr):
        for name,params in self.modules():
            if (params.Base <= addr and 
                    addr <= (params.Base + params.Size)):
                return name
        return ""

    def get_module(self, addr):
        size = self._sym.GetModuleParameters(addr).Size
        return Module(addr, size, self._data.ReadVirtual, self.addr_to_name(addr))

    def module_names(self):
        """module_names() -> list of all module names"""
        return [self._sym.GetModuleNames(0, index=i)[1] for i in range(len(self))]

    def modules(self):
        """modules() -> list of (name, modparams) tuples"""
        return [
            (self._sym.GetModuleNames(0, index=i),
             self._sym.GetModuleParameters(self._sym.GetModuleByIndex(i)))
            for i in range(len(self))]

    def seh_status(self):
        """seh_status() -> list of all modules and SEH status"""
        #for name in self.module_names():
        #    (status, reason) = self[name].seh_status()
        #    print("%s %s" % (name, reason))
        pass

