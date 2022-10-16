from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception

class DebugSymbols(object):
    def __init__(self, symbols):
        self._sym = symbols
        exception.wrap_comclass(self._sym)

    def Release(self):
        cnt = self._sym.Release()
        if cnt == 0:
            self._sym = None
        return cnt

    # IDebugSymbols

    def GetSymbolOptions(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetSymbolOptions()
        #exception.check_err(hr)
        #return options

    def AddSymbolOptions(self, options):
        hr = self._sym.AddSymbolOptions(options)
        exception.check_err(hr)

    def RemoveSymbolOptions(self, options):
        hr = self._sym.RemoveSymbolOptions(options)
        exception.check_err(hr)

    def SetSymbolOptions(self, options):
        hr = self._sym.SetSymbolOptions(options)
        exception.check_err(hr)

    def GetNameByOffset(self, offset):
        name = create_string_buffer(256)
        size = c_ulong()
        disp = c_ulonglong()
        hr = self._sym.GetNameByOffset(offset, name, 256, byref(size), byref(disp))
        exception.check_err(hr)
        name = name[:size.value]
        name = name.rstrip(b'\x00')
        return (name, disp.value)

    def GetOffsetByName(self, name):
        if isinstance(name, str):
            name = name.encode()
        offset = c_ulonglong()
        hr = self._sym.GetOffsetByName(name, byref(offset))
        if hr == S_OK:
            ret = True
        elif hr == S_FALSE:
            ret = False
        else:
            exception.check_err(hr)
        return (ret, offset.value)

    def GetNearNameByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetLineByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetOffsetByLine(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberModules(self):
        loaded = c_ulong()
        unloaded = c_ulong()
        hr = self._sym.GetNumberModules(byref(loaded), byref(unloaded))
        exception.check_err(hr)
        return (loaded.value, unloaded.value)

    def GetModuleByIndex(self, index):
        base = c_ulonglong()
        hr = self._sym.GetModuleByIndex(index, byref(base))
        exception.check_err(hr)
        return base.value

    def GetModuleByModuleName(self, name, start=0):
        if isinstance(name, str):
            name = name.encode()
        index = c_ulong()
        base  = c_ulonglong()
        hr = self._sym.GetModuleByModuleName(name, start, byref(index), byref(base))
        exception.check_err(hr)
        return (index.value, base.value)

    def GetModuleByOffset(self, offset):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetModuleByModuleName()
        #exception.check_err(hr)
        #return (index, base)

    def GetModuleNames(self, base, index=DbgEng.DEBUG_ANY_ID):
        if index != DbgEng.DEBUG_ANY_ID:
            base = 0
        image_name  = create_string_buffer(256)
        image_size  = c_ulong()
        module_name = create_string_buffer(256)
        module_size = c_ulong()
        loaded_name = create_string_buffer(256)
        loaded_size = c_ulong()

        hr = self._sym.GetModuleNames(index, base,
            image_name, 256, byref(image_size),
            module_name, 256, byref(module_size),
            loaded_name, 256, byref(loaded_size))
        exception.check_err(hr)
        image_name = image_name[:image_size.value].rstrip(b'\x00').decode()
        module_name = module_name[:module_size.value].rstrip(b'\x00').decode()
        loaded_name = loaded_name[:loaded_size.value].rstrip(b'\x00').decode()
        return (image_name, module_name, loaded_name)

    def GetModuleParameters(self, base):
        bases = (c_ulonglong * 1)()
        bases[0] = base
        params = (DbgEng._DEBUG_MODULE_PARAMETERS * 1)()
        hr = self._sym.GetModuleParameters(1, bases, 0, params)
        exception.check_err(hr)
        return params[0]
        

    def GetSymbolModule(self, symbol):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetSymbolModule()
        #exception.check_err(hr)
        #return base

    def GetTypeName(self):
        raise exception.E_NOTIMPL_Error

    def GetTypeId(self, name, module=0):
        if isinstance(name, str):
            name = name.encode()
        typeid = c_ulong()
        hr = self._sym.GetTypeId(0, name, byref(typeid))
        exception.check_err(hr)
        return typeid.value

    def GetTypeSize(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolTypeId(self):
        raise exception.E_NOTIMPL_Error

    def GetOffsetTypeId(self):
        raise exception.E_NOTIMPL_Error

    def ReadTypedDataVirtual(self):
        raise exception.E_NOTIMPL_Error

    def WriteTypedDataVirtual(self):
        raise exception.E_NOTIMPL_Error

    def OutputTypedDataVirtual(self, offset, module, typeid, flags=0):
        outctl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        hr = self._sym.OutputTypedDataVirtual(outctl, offset, module, typeid, flags)
        exception.check_err(hr)

    def ReadTypedDataPhysical(self):
        raise exception.E_NOTIMPL_Error

    def WriteTypedDataPhysical(self):
        raise exception.E_NOTIMPL_Error

    def OutputTypedDataPhysical(self):
        raise exception.E_NOTIMPL_Error

    def GetScope(self):
        raise exception.E_NOTIMPL_Error

    def SetScope(self):
        raise exception.E_NOTIMPL_Error

    def ResetScope(self):
        raise exception.E_NOTIMPL_Error

    def GetScopeSymbolGroup(self):
        raise exception.E_NOTIMPL_Error

    def CreateSymbolGroup(self):
        raise exception.E_NOTIMPL_Error

    def StartSymbolMatch(self, pattern):
        if isinstance(pattern, str):
            pattern = pattern.encode()
        handle = c_ulonglong()
        hr = self._sym.StartSymbolMatch(pattern, byref(handle))
        exception.check_err(hr)
        return handle.value

    def GetNextSymbolMatch(self, handle):
        name = create_string_buffer(256)
        size = c_ulong()
        offset = c_ulonglong()
        hr = self._sym.GetNextSymbolMatch(handle, name, 256, byref(size), byref(offset))
        exception.check_err(hr)
        return (offset.value, name[:size.value].rstrip(b'\x00'))

    def EndSymbolMatch(self, handle):
        hr = self._sym.EndSymbolMatch(handle)
        exception.check_err(hr)

    def Reload(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolPath(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetSymbolPath()
        #exception.check_err(hr)
        #return path

    def SetSymbolPath(self, path):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.SetSymbolPath()
        #exception.check_err(hr)

    def AppendSymbolPath(self, addition):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.AppendSymbolPath()
        #exception.check_err(hr)

    def GetImagePath(self):
        raise exception.E_NOTIMPL_Error

    def SetImagePath(self):
        raise exception.E_NOTIMPL_Error

    def AppendImagePath(self):
        raise exception.E_NOTIMPL_Error

    def GetSourcePath(self):
        raise exception.E_NOTIMPL_Error

    def GetSourcePathElement(self):
        raise exception.E_NOTIMPL_Error

    def SetSourcePath(self):
        raise exception.E_NOTIMPL_Error

    def AppendSourcePath(self):
        raise exception.E_NOTIMPL_Error

    def FindSourceFile(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceFileLineOffsets(self):
        raise exception.E_NOTIMPL_Error

    # IDebugSymbols2

    def GetModuleVersionInformation(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleNameString(self):
        raise exception.E_NOTIMPL_Error

    def GetConstantName(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldName(self):
        raise exception.E_NOTIMPL_Error

    def GetTypeOptions(self):
        raise exception.E_NOTIMPL_Error

    def AddTypeOptions(self):
        raise exception.E_NOTIMPL_Error

    def RemoveTypeOptions(self):
        raise exception.E_NOTIMPL_Error

    def SetTypeOptions(self):
        raise exception.E_NOTIMPL_Error

    # IDebugSymbols3

    def GetNameByOffsetWide(self):
        raise exception.E_NOTIMPL_Error

    def GetOffsetByNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetNearNameByOffsetWide(self):
        raise exception.E_NOTIMPL_Error

    def GetLineByOffsetWide(self):
        raise exception.E_NOTIMPL_Error

    def GetOffsetByLineWide(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleByModuleNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolModuleWide(self):
        raise exception.E_NOTIMPL_Error

    def GetTypeNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetTypeIdWide(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldOffsetWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolTypeIdWide(self):
        raise exception.E_NOTIMPL_Error

    def GetScopeSymbolGroup2(self):
        raise exception.E_NOTIMPL_Error

    def CreateSymbolGroup2(self):
        raise exception.E_NOTIMPL_Error

    def StartSymbolMatchWide(self):
        raise exception.E_NOTIMPL_Error

    def GetNextSymbolMatchWide(self):
        raise exception.E_NOTIMPL_Error

    def ReloadWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolPathWide(self):
        raise exception.E_NOTIMPL_Error

    def SetSymbolPathWide(self):
        raise exception.E_NOTIMPL_Error

    def AppendSymbolPathWide(self):
        raise exception.E_NOTIMPL_Error

    def GetImagePathWide(self):
        raise exception.E_NOTIMPL_Error

    def SetImagePathWide(self):
        raise exception.E_NOTIMPL_Error

    def AppendImagePathWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSourcePathWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSourcePathElementWide(self):
        raise exception.E_NOTIMPL_Error

    def SetSourcePathWide(self):
        raise exception.E_NOTIMPL_Error

    def AppendSourcePathWide(self):
        raise exception.E_NOTIMPL_Error

    def FindSourceFileWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceFileLineOffsetsWide(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleVersionInformationWide(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleNameStringWide(self):
        raise exception.E_NOTIMPL_Error

    def GetConstantNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldNameWide(self):
        raise exception.E_NOTIMPL_Error

    def IsManagedModule(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleByModuleName2(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleByModuleName2Wide(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleByOffset2(self):
        raise exception.E_NOTIMPL_Error

    def AddSyntheticModule(self):
        raise exception.E_NOTIMPL_Error

    def AddSyntheticModuleWide(self):
        raise exception.E_NOTIMPL_Error

    def RemoveSyntheticModule(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentScopeFrameIndex(self):
        raise exception.E_NOTIMPL_Error

    def SetScopeFrameByIndex(self):
        raise exception.E_NOTIMPL_Error

    def SetScopeFromJitDebugInfo(self):
        raise exception.E_NOTIMPL_Error

    def SetScopeFromStoredEvent(self):
        raise exception.E_NOTIMPL_Error

    def OutputSymbolByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetFunctionEntryByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldTypeAndOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetFieldTypeAndOffsetWide(self):
        raise exception.E_NOTIMPL_Error

    def AddSyntheticSymbol(self):
        raise exception.E_NOTIMPL_Error

    def AddSyntheticSymbolWide(self):
        raise exception.E_NOTIMPL_Error

    def RemoveSyntheticSymbol(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntriesByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntriesByName(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntriesByNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryByToken(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryInformation(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryString(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryStringWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryOffsetRegions(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolEntryBySymbolEntry(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntriesByOffset(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntriesByLine(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntriesByLineWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntryString(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntryStringWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntryOffsetRegions(self):
        raise exception.E_NOTIMPL_Error

    def GetSourceEntryBySourceEntry(self):
        raise exception.E_NOTIMPL_Error

    # IDebugSymbols4

    def GetScopeEx(self):
        raise exception.E_NOTIMPL_Error

    def SetScopeEx(self):
        raise exception.E_NOTIMPL_Error

    def GetNameByInlineContext(self):
        raise exception.E_NOTIMPL_Error

    def GetNameByInlineContextWide(self):
        raise exception.E_NOTIMPL_Error

    def GetLineByInlineContext(self):
        raise exception.E_NOTIMPL_Error

    def GetLineByInlineContextWide(self):
        raise exception.E_NOTIMPL_Error

    def OutputSymbolByInlineContext(self):
        raise exception.E_NOTIMPL_Error

    # IDebugSymbols5

    def GetCurrentScopeFrameIndexEx(self):
        raise exception.E_NOTIMPL_Error

    def SetScopeFrameByIndexEx(self):
        raise exception.E_NOTIMPL_Error
