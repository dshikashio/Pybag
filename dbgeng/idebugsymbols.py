from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE

from . import core as DbgEng
from . import exception

class DebugSymbols(object):
    def __init__(self, symbols):
        self._sym = symbols
        exception.wrap_comclass(self._sym)

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
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetNumberModules()
        #exception.check_err(hr)
        #return (loaded, unloaded)

    def GetModuleByIndex(self, index):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetNumberModules()
        #exception.check_err(hr)
        #return base

    def GetModuleByModuleName(self, name):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetModuleByModuleName()
        #exception.check_err(hr)
        #return base

    def GetModuleByOffset(self, offset):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetModuleByModuleName()
        #exception.check_err(hr)
        #return (index, base)

    def GetModuleNames(self):
        raise exception.E_NOTIMPL_Error

    def GetModuleParameters(self):
        raise exception.E_NOTIMPL_Error

    def GetSymbolModule(self, symbol):
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetSymbolModule()
        #exception.check_err(hr)
        #return base

    def GetTypeName(self):
        raise exception.E_NOTIMPL_Error

    def GetTypeId(self):
        raise exception.E_NOTIMPL_Error

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

    def OutputTypedDataVirtual(self):
        raise exception.E_NOTIMPL_Error

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

    def StartSymbolMatch(self):
        raise exception.E_NOTIMPL_Error

    def GetNextSymbolMatch(self):
        raise exception.E_NOTIMPL_Error

    def EndSymbolMatch(self):
        raise exception.E_NOTIMPL_Error

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
