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
        raise exception.E_NOTIMPL_Error
        #hr = self._sym.GetOffsetByName()
        #if hr == S_OK:
        #   ret = True
        #elif hr == S_FALSE:
        #   ret = False
        #else:
        #   exception.check_err(hr)
        #return (ret, offset)

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



"""


    STDMETHOD(GetScopeSymbolGroup2)(
        THIS_
         ULONG Flags,
         PDEBUG_SYMBOL_GROUP2 Update,
         PDEBUG_SYMBOL_GROUP2* Symbols
        ) PURE;

    STDMETHOD(CreateSymbolGroup2)(
        THIS_
         PDEBUG_SYMBOL_GROUP2* Group
        ) PURE;

    STDMETHOD(StartSymbolMatchWide)(
        THIS_
         PCWSTR Pattern,
         PULONG64 Handle
        ) PURE;
    STDMETHOD(GetNextSymbolMatchWide)(
        THIS_
         ULONG64 Handle,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG MatchSize,
         PULONG64 Offset
        ) PURE;

    STDMETHOD(ReloadWide)(
        THIS_
         PCWSTR Module
        ) PURE;

    STDMETHOD(GetSymbolPathWide)(
        THIS_
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG PathSize
        ) PURE;
    STDMETHOD(SetSymbolPathWide)(
        THIS_
         PCWSTR Path
        ) PURE;
    STDMETHOD(AppendSymbolPathWide)(
        THIS_
         PCWSTR Addition
        ) PURE;

    STDMETHOD(GetImagePathWide)(
        THIS_
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG PathSize
        ) PURE;
    STDMETHOD(SetImagePathWide)(
        THIS_
         PCWSTR Path
        ) PURE;
    STDMETHOD(AppendImagePathWide)(
        THIS_
         PCWSTR Addition
        ) PURE;

    STDMETHOD(GetSourcePathWide)(
        THIS_
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG PathSize
        ) PURE;
    STDMETHOD(GetSourcePathElementWide)(
        THIS_
         ULONG Index,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG ElementSize
        ) PURE;
    STDMETHOD(SetSourcePathWide)(
        THIS_
         PCWSTR Path
        ) PURE;
    STDMETHOD(AppendSourcePathWide)(
        THIS_
         PCWSTR Addition
        ) PURE;
    STDMETHOD(FindSourceFileWide)(
        THIS_
         ULONG StartElement,
         PCWSTR File,
         ULONG Flags,
         PULONG FoundElement,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG FoundSize
        ) PURE;
    STDMETHOD(GetSourceFileLineOffsetsWide)(
        THIS_
         PCWSTR File,
        PULONG64 Buffer,
         ULONG BufferLines,
         PULONG FileLines
        ) PURE;

    STDMETHOD(GetModuleVersionInformationWide)(
        THIS_
         ULONG Index,
         ULONG64 Base,
         PCWSTR Item,
        PVOID Buffer,
         ULONG BufferSize,
         PULONG VerInfoSize
        ) PURE;
    STDMETHOD(GetModuleNameStringWide)(
        THIS_
         ULONG Which,
         ULONG Index,
         ULONG64 Base,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG NameSize
        ) PURE;

    STDMETHOD(GetConstantNameWide)(
        THIS_
         ULONG64 Module,
         ULONG TypeId,
         ULONG64 Value,
        PWSTR NameBuffer,
         ULONG NameBufferSize,
         PULONG NameSize
        ) PURE;

    STDMETHOD(GetFieldNameWide)(
        THIS_
         ULONG64 Module,
         ULONG TypeId,
         ULONG FieldIndex,
        PWSTR NameBuffer,
         ULONG NameBufferSize,
         PULONG NameSize
        ) PURE;

    // Returns S_OK if the engine is using managed
    // debugging support when retriving information
    // for the given module.  This can be expensive
    // to check.
    STDMETHOD(IsManagedModule)(
        THIS_
         ULONG Index,
         ULONG64 Base
        ) PURE;

    // The module name may not be unique.
    // This method returns the first match.
    STDMETHOD(GetModuleByModuleName2)(
        THIS_
         PCSTR Name,
         ULONG StartIndex,
         ULONG Flags,
         PULONG Index,
         PULONG64 Base
        ) PURE;
    STDMETHOD(GetModuleByModuleName2Wide)(
        THIS_
         PCWSTR Name,
         ULONG StartIndex,
         ULONG Flags,
         PULONG Index,
         PULONG64 Base
        ) PURE;
    // Offset can be any offset within
    // the module extent.  Extents may
    // not be unique when including unloaded
    // drivers.  This method returns the
    // first match.
    STDMETHOD(GetModuleByOffset2)(
        THIS_
         ULONG64 Offset,
         ULONG StartIndex,
         ULONG Flags,
         PULONG Index,
         PULONG64 Base
        ) PURE;

    // A caller can create artificial loaded modules in
    // the engine's module list if desired.
    // These modules only serve as names for
    // a region of addresses.  They cannot have
    // real symbols loaded for them; if that
    // is desired Reload can be used with explicit
    // parameters to create a true module entry.
    // The region must not be in use by any other
    // module.
    // A general reload will discard any synthetic modules.
    STDMETHOD(AddSyntheticModule)(
        THIS_
         ULONG64 Base,
         ULONG Size,
         PCSTR ImagePath,
         PCSTR ModuleName,
         ULONG Flags
        ) PURE;
    STDMETHOD(AddSyntheticModuleWide)(
        THIS_
         ULONG64 Base,
         ULONG Size,
         PCWSTR ImagePath,
         PCWSTR ModuleName,
         ULONG Flags
        ) PURE;
    STDMETHOD(RemoveSyntheticModule)(
        THIS_
         ULONG64 Base
        ) PURE;

    // Modify the current frame used for scoping.
    // This is equivalent to the '.frame' command.
    STDMETHOD(GetCurrentScopeFrameIndex)(
        THIS_
         PULONG Index
        ) PURE;
    STDMETHOD(SetScopeFrameByIndex)(
        THIS_
         ULONG Index
        ) PURE;

    // Recovers JIT_DEBUG_INFO information at the given
    // address from the debuggee and sets current
    // debugger scope context from it.
    // Equivalent to '.jdinfo' command.
    STDMETHOD(SetScopeFromJitDebugInfo)(
        THIS_
         ULONG OutputControl,
         ULONG64 InfoOffset
        ) PURE;

    // Switches the current debugger scope to
    // the stored event information.
    // Equivalent to the '.ecxr' command.
    STDMETHOD(SetScopeFromStoredEvent)(
        THIS
        ) PURE;

    // Takes the first symbol hit and outputs it.
    // Controlled with DEBUG_OUTSYM_* flags.
    STDMETHOD(OutputSymbolByOffset)(
        THIS_
         ULONG OutputControl,
         ULONG Flags,
         ULONG64 Offset
        ) PURE;

    // Function entry information for a particular
    // piece of code can be retrieved by this method.
    // The actual data returned is system-dependent.
    STDMETHOD(GetFunctionEntryByOffset)(
        THIS_
         ULONG64 Offset,
         ULONG Flags,
        PVOID Buffer,
         ULONG BufferSize,
         PULONG BufferNeeded
        ) PURE;

    // Given a type which can contain members
    // this method returns the type ID and offset of a
    // particular member within the type.
    // Field gives the dot-separated path
    // to the field of interest.
    STDMETHOD(GetFieldTypeAndOffset)(
        THIS_
         ULONG64 Module,
         ULONG ContainerTypeId,
         PCSTR Field,
         PULONG FieldTypeId,
         PULONG Offset
        ) PURE;
    STDMETHOD(GetFieldTypeAndOffsetWide)(
        THIS_
         ULONG64 Module,
         ULONG ContainerTypeId,
         PCWSTR Field,
         PULONG FieldTypeId,
         PULONG Offset
        ) PURE;

    // Artificial symbols can be created in any
    // existing module as a way to name an address.
    // The address must not already have symbol
    // information.
    // A reload will discard synthetic symbols
    // for all address regions reloaded.
    STDMETHOD(AddSyntheticSymbol)(
        THIS_
         ULONG64 Offset,
         ULONG Size,
         PCSTR Name,
         ULONG Flags,
         PDEBUG_MODULE_AND_ID Id
        ) PURE;
    STDMETHOD(AddSyntheticSymbolWide)(
        THIS_
         ULONG64 Offset,
         ULONG Size,
         PCWSTR Name,
         ULONG Flags,
         PDEBUG_MODULE_AND_ID Id
        ) PURE;
    STDMETHOD(RemoveSyntheticSymbol)(
        THIS_
         PDEBUG_MODULE_AND_ID Id
        ) PURE;

    // The following methods can return multiple
    // hits for symbol lookups to allow for all
    // possible hits to be returned.
    STDMETHOD(GetSymbolEntriesByOffset)(
        THIS_
         ULONG64 Offset,
         ULONG Flags,
        PDEBUG_MODULE_AND_ID Ids,
        PULONG64 Displacements,
         ULONG IdsCount,
         PULONG Entries
        ) PURE;
    STDMETHOD(GetSymbolEntriesByName)(
        THIS_
         PCSTR Symbol,
         ULONG Flags,
        PDEBUG_MODULE_AND_ID Ids,
         ULONG IdsCount,
         PULONG Entries
        ) PURE;
    STDMETHOD(GetSymbolEntriesByNameWide)(
        THIS_
         PCWSTR Symbol,
         ULONG Flags,
        PDEBUG_MODULE_AND_ID Ids,
         ULONG IdsCount,
         PULONG Entries
        ) PURE;
    // Symbol lookup by managed metadata token.
    STDMETHOD(GetSymbolEntryByToken)(
        THIS_
         ULONG64 ModuleBase,
         ULONG Token,
         PDEBUG_MODULE_AND_ID Id
        ) PURE;

    // Retrieves full symbol entry information from an ID.
    STDMETHOD(GetSymbolEntryInformation)(
        THIS_
         PDEBUG_MODULE_AND_ID Id,
         PDEBUG_SYMBOL_ENTRY Info
        ) PURE;
    STDMETHOD(GetSymbolEntryString)(
        THIS_
         PDEBUG_MODULE_AND_ID Id,
         ULONG Which,
        PSTR Buffer,
         ULONG BufferSize,
         PULONG StringSize
        ) PURE;
    STDMETHOD(GetSymbolEntryStringWide)(
        THIS_
         PDEBUG_MODULE_AND_ID Id,
         ULONG Which,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG StringSize
        ) PURE;
    // Returns all known memory regions associated
    // with the given symbol.  Simple symbols will
    // have a single region starting from their base.
    // More complicated regions, such as functions
    // with multiple code areas, can have an arbitrarily
    // large number of regions.
    // The quality of information returned is highly
    // dependent on the symbolic information availble.
    STDMETHOD(GetSymbolEntryOffsetRegions)(
        THIS_
         PDEBUG_MODULE_AND_ID Id,
         ULONG Flags,
        PDEBUG_OFFSET_REGION Regions,
         ULONG RegionsCount,
         PULONG RegionsAvail
        ) PURE;

    // This method allows navigating within the
    // symbol entry hierarchy.
    STDMETHOD(GetSymbolEntryBySymbolEntry)(
        THIS_
         PDEBUG_MODULE_AND_ID FromId,
         ULONG Flags,
         PDEBUG_MODULE_AND_ID ToId
        ) PURE;

    // The following methods can return multiple
    // hits for source lookups to allow for all
    // possible hits to be returned.
    STDMETHOD(GetSourceEntriesByOffset)(
        THIS_
         ULONG64 Offset,
         ULONG Flags,
        PDEBUG_SYMBOL_SOURCE_ENTRY Entries,
         ULONG EntriesCount,
         PULONG EntriesAvail
        ) PURE;
    STDMETHOD(GetSourceEntriesByLine)(
        THIS_
         ULONG Line,
         PCSTR File,
         ULONG Flags,
        PDEBUG_SYMBOL_SOURCE_ENTRY Entries,
         ULONG EntriesCount,
         PULONG EntriesAvail
        ) PURE;
    STDMETHOD(GetSourceEntriesByLineWide)(
        THIS_
         ULONG Line,
         PCWSTR File,
         ULONG Flags,
        PDEBUG_SYMBOL_SOURCE_ENTRY Entries,
         ULONG EntriesCount,
         PULONG EntriesAvail
        ) PURE;

    STDMETHOD(GetSourceEntryString)(
        THIS_
         PDEBUG_SYMBOL_SOURCE_ENTRY Entry,
         ULONG Which,
        PSTR Buffer,
         ULONG BufferSize,
         PULONG StringSize
        ) PURE;
    STDMETHOD(GetSourceEntryStringWide)(
        THIS_
         PDEBUG_SYMBOL_SOURCE_ENTRY Entry,
         ULONG Which,
        PWSTR Buffer,
         ULONG BufferSize,
         PULONG StringSize
        ) PURE;
    // Returns all known memory regions associated
    // with the given source entry.  As with
    // GetSymbolEntryOffsetRegions the regions available
    // are variable.
    STDMETHOD(GetSourceEntryOffsetRegions)(
        THIS_
         PDEBUG_SYMBOL_SOURCE_ENTRY Entry,
         ULONG Flags,
        PDEBUG_OFFSET_REGION Regions,
         ULONG RegionsCount,
         PULONG RegionsAvail
        ) PURE;

    // This method allows navigating within the
    // source entries.
    STDMETHOD(GetSourceEntryBySourceEntry)(
        THIS_
         PDEBUG_SYMBOL_SOURCE_ENTRY FromEntry,
         ULONG Flags,
         PDEBUG_SYMBOL_SOURCE_ENTRY ToEntry
        ) PURE;

    // IDebugSymbols4
    STDMETHOD(GetScopeEx)(
        THIS_
         PULONG64 InstructionOffset,
         PDEBUG_STACK_FRAME_EX ScopeFrame,
        PVOID ScopeContext,
         ULONG ScopeContextSize
        ) PURE;

    STDMETHOD(SetScopeEx)(
        THIS_
         ULONG64 InstructionOffset,
         PDEBUG_STACK_FRAME_EX ScopeFrame,
        PVOID ScopeContext,
         ULONG ScopeContextSize
        ) PURE;

    STDMETHOD(GetNameByInlineContext)(
        THIS_
         ULONG64 Offset,
         ULONG InlineContext,
        PSTR NameBuffer,
         ULONG NameBufferSize,
         PULONG NameSize,
         PULONG64 Displacement
        ) PURE;

    STDMETHOD(GetNameByInlineContextWide)(
        THIS_
         ULONG64 Offset,
         ULONG InlineContext,
        PWSTR NameBuffer,
         ULONG NameBufferSize,
         PULONG NameSize,
         PULONG64 Displacement
        ) PURE;

    STDMETHOD(GetLineByInlineContext)(
        THIS_
         ULONG64 Offset,
         ULONG InlineContext,
         PULONG Line,
        PSTR FileBuffer,
         ULONG FileBufferSize,
         PULONG FileSize,
         PULONG64 Displacement
        ) PURE;

    STDMETHOD(GetLineByInlineContextWide)(
        THIS_
         ULONG64 Offset,
         ULONG InlineContext,
         PULONG Line,
        PWSTR FileBuffer,
         ULONG FileBufferSize,
         PULONG FileSize,
         PULONG64 Displacement
        ) PURE;

    STDMETHOD(OutputSymbolByInlineContext)(
        THIS_
         ULONG OutputControl,
         ULONG Flags,
         ULONG64 Offset,
         ULONG InlineContext
        ) PURE;

    // IDebugSymbols5
    STDMETHOD(GetCurrentScopeFrameIndexEx)(
        THIS_
         ULONG Flags,
         PULONG Index
        ) PURE;
    STDMETHOD(SetScopeFrameByIndexEx)(
        THIS_
         ULONG Flags,
         ULONG Index
        ) PURE;

"""

