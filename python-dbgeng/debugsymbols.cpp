#include "pydbgeng.h"

static PyObject *
dbgsyms_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugSymbolsObject *self;

    self = (PyDebugSymbolsObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->syms = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgsyms_dealloc(PyDebugSymbolsObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->syms != NULL) {
        self->syms->Release();
        self->syms = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgsyms_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugSymbolsObject *self = (PyDebugSymbolsObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugSymbols3 *debugSymbols = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->syms) {
        self->syms->Release();
        self->syms = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugSymbols",
                kwlist, &PyDebugClientType, &client))
        return -1;
    if (client->client == NULL) {
        err_noclient();
        return -1;
    }
    if ((hr = client->client->QueryInterface(__uuidof(IDebugSymbols3),
                    (void **)&debugSymbols)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->syms = debugSymbols;
    return 0;
}

static PyObject *
dbgsyms_GetConstantName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG TypeId;
    ULONG64 Value;
    PSTR Name = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KIK:GetConstantName", &Module, &TypeId, &Value))
        return NULL;
    hr = self->syms->GetConstantName(Module, TypeId, Value, NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    Name = (PSTR)PyMem_Malloc(NameSize + 2);
    if (Name == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->GetConstantName(Module, TypeId, Value, 
                    Name, NameSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto error;
    }
    ret = Py_BuildValue("s", Name);
error:
    if (Name) PyMem_Free(Name);
    return ret;
}

static PyObject *
dbgsyms_OutputTypedDataPhysical(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG OutputControl;
    ULONG64 Offset;
    ULONG64 Module;
    ULONG TypeId;
    ULONG Flags;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    Flags = DEBUG_TYPEOPTS_LONGSTATUS_DISPLAY | DEBUG_TYPEOPTS_FORCERADIX_OUTPUT;
    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KKI|I:OutputTypedDataPhysical", 
                &Offset, &Module, &TypeId, &Flags))
        return NULL;
    if ((hr = self->syms->OutputTypedDataPhysical(OutputControl,
                    Offset, Module, TypeId, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_OutputTypedDataVirtual(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG OutputControl;
    ULONG64 Offset;
    ULONG64 Module;
    ULONG TypeId;
    ULONG Flags;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    Flags = DEBUG_TYPEOPTS_LONGSTATUS_DISPLAY | DEBUG_TYPEOPTS_FORCERADIX_OUTPUT;
    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KKI|I:OutputTypedDataVirtual", 
                &Offset, &Module, &TypeId, &Flags))
        return NULL;
    if ((hr = self->syms->OutputTypedDataVirtual(OutputControl,
                    Offset, Module, TypeId, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetFieldName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG TypeId;
    ULONG FieldIndex;
    PSTR NameBuffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KII:GetFieldName", 
                &Module, &TypeId, &FieldIndex))
        return NULL;
    hr = self->syms->GetFieldName(Module, TypeId, FieldIndex, 
            NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    NameBuffer = (PSTR)PyMem_Malloc(NameSize + 2);
    if (NameBuffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetFieldName(Module, TypeId, FieldIndex, 
            NameBuffer, NameSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }
    ret = Py_BuildValue("s", NameBuffer);

done:
    if (NameBuffer) PyMem_Free(NameBuffer);
    return ret;
}

static PyObject *
dbgsyms_GetFieldOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG TypeId;
    PSTR Field;
    ULONG Offset;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KIs:GetFieldOffset", 
                &Module, &TypeId, &Field))
        return NULL;
    hr = self->syms->GetFieldOffset(Module, TypeId, Field, &Offset);
    if (hr == S_OK)
        return Py_BuildValue("I", Offset);
    else if (hr == E_NOINTERFACE)
        return err_nointerface("The field Field could not be found in "
                "the type specified by TypeId.");
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgsyms_GetFieldTypeAndOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG ContainerTypeId;
    PSTR Field;
    ULONG FieldTypeId;
    ULONG Offset;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KIs:GetFieldTypeAndOffset", 
                &Module, &ContainerTypeId, &Field))
        return NULL;
    hr = self->syms->GetFieldTypeAndOffset(Module, ContainerTypeId, 
            Field, &FieldTypeId, &Offset);
    if (hr == S_OK)
        return Py_BuildValue("II", FieldTypeId, Offset);
    else if (hr == E_NOINTERFACE)
        return err_nointerface("The field Field could not be found in "
                "the type specified by ContainerTypeId.");
    else
        return err_dbgeng(hr);
}


static PyObject *
dbgsyms_AppendImagePath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Addition;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:AppendImagePath", &Addition))
        return NULL;
    if ((hr = self->syms->AppendImagePath(Addition)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetImagePath(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG PathSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    hr = self->syms->GetImagePath(Buffer, 0, &PathSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(PathSize * sizeof(*Buffer));
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetImagePath(Buffer, PathSize, NULL)) != S_OK) {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_SetImagePath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Path;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:SetImagePath", &Path))
        return NULL;
    if ((hr = self->syms->SetImagePath(Path)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetLineByOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Line = 0;
    PSTR FileBuffer = NULL;
    ULONG FileSize = 0;
    ULONG64 Displacement;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:GetLineByOffset", &Offset))
        return NULL;
    hr = self->syms->GetLineByOffset(Offset, &Line, FileBuffer, 0, 
            &FileSize, &Displacement);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    FileBuffer = (PSTR)PyMem_Malloc(FileSize * sizeof(*FileBuffer));
    if (FileBuffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetLineByOffset(Offset, &Line, FileBuffer, 0, 
            &FileSize, &Displacement)) != S_OK)
    {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("IsK", Line, FileBuffer, Displacement);
    }

    if (FileBuffer) PyMem_Free(FileBuffer);
    return ret;
}

static PyObject *
dbgsyms_GetOffsetByLine(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Line = 0;
    PSTR File = NULL;
    ULONG64 Offset;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "Is:GetOffsetByLine", &Line, &File))
        return NULL;
    if ((hr = self->syms->GetOffsetByLine(Line, File, &Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsyms_GetModuleByIndex(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:GetModuleByIndex", &Index))
        return NULL;
    if ((hr = self->syms->GetModuleByIndex(Index, &Base)) != S_OK) {
        if (hr == S_FALSE)
            return err_false("The specified module was not loaded, and "
                "information about the module was not available.");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("K", Base);
}

static PyObject *
dbgsyms_GetModuleByModuleName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Name;
    ULONG StartIndex;
    ULONG Index;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "sI:GetModuleByModuleName", &Name, &StartIndex))
        return NULL;
    if ((hr = self->syms->GetModuleByModuleName(Name, 
                    StartIndex, &Index, &Base)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", Index, Base);
}

static PyObject *
dbgsyms_GetModuleByModuleName2(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Name;
    ULONG StartIndex;
    ULONG Flags;
    ULONG Index;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "sII:GetModuleByModuleName2", 
                &Name, &StartIndex, &Flags))
        return NULL;
    if ((hr = self->syms->GetModuleByModuleName2(Name, 
                    StartIndex, Flags, &Index, &Base)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", Index, Base);
}

static PyObject *
dbgsyms_GetModuleNames(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG64 Base = 0;
    PSTR ImageNameBuffer = NULL;
    ULONG ImageNameSize;
    PSTR ModuleNameBuffer = NULL;
    ULONG ModuleNameSize;
    PSTR LoadedImageNameBuffer = NULL;
    ULONG LoadedImageNameSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I|K:GetModuleNames", &Index, &Base))
        return NULL;
    hr = self->syms->GetModuleNames(Index, Base,
            NULL, 0, &ImageNameSize,
            NULL, 0, &ModuleNameSize,
            NULL, 0, &LoadedImageNameSize);
    if (hr != S_OK && hr != S_FALSE) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("The specified module was not found.");
        else
            return err_dbgeng(hr);
    }

    ImageNameBuffer = (PSTR)PyMem_Malloc(ImageNameSize * sizeof(*ImageNameBuffer));
    ModuleNameBuffer = (PSTR)PyMem_Malloc(ModuleNameSize * sizeof(*ModuleNameBuffer));
    LoadedImageNameBuffer = (PSTR)PyMem_Malloc(LoadedImageNameSize * 
                                         sizeof(*LoadedImageNameBuffer));
    if (ImageNameBuffer == NULL ||
        ModuleNameBuffer == NULL ||
        LoadedImageNameBuffer == NULL)
    {
        PyErr_NoMemory();
        goto error;
    }

    if ((hr = self->syms->GetModuleNames(Index, Base,
                    ImageNameBuffer, ImageNameSize, NULL,
                    ModuleNameBuffer, ModuleNameSize, NULL,
                    LoadedImageNameBuffer, LoadedImageNameSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto error;
    }

    ret = Py_BuildValue("sss", ImageNameBuffer, ModuleNameBuffer, 
            LoadedImageNameBuffer);

error:
    if (ImageNameBuffer) PyMem_Free(ImageNameBuffer);
    if (ModuleNameBuffer) PyMem_Free(ModuleNameBuffer);
    if (LoadedImageNameBuffer) PyMem_Free(LoadedImageNameBuffer);
    return ret;
}

static PyObject *
dbgsyms_GetModuleNameString(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Which;
    ULONG Index;
    ULONG64 Base;
    PSTR Buffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "IIK:GetModuleNameString", &Which, &Index, &Base))
        return NULL;
    hr = self->syms->GetModuleNameString(Which, Index, Base,
            NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(NameSize * sizeof(*Buffer));
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->GetModuleNameString(Which, Index, Base,
                    Buffer, NameSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_GetNumberModules(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    ULONG Loaded;
    ULONG Unloaded;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->GetNumberModules(&Loaded, &Unloaded)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("II", Loaded, Unloaded);
}

static PyObject *
dbgsyms_GetModuleByOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG StartIndex = 0;
    ULONG Index;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K|I:GetModuleByOffset", &Offset, &StartIndex))
        return NULL;
    if ((hr = self->syms->GetModuleByOffset(Offset, StartIndex, 
                    &Index, &Base)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", Index, Base);
}

static PyObject *
dbgsyms_GetModuleByOffset2(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG StartIndex = 0;
    ULONG Flags = 0;
    ULONG Index;
    ULONG64 Base;

    Flags = DEBUG_GETMOD_DEFAULT;
    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K|II:GetModuleByOffset2", 
                &Offset, &StartIndex, &Flags))
        return NULL;
    if ((hr = self->syms->GetModuleByOffset2(Offset, StartIndex, 
                    Flags, &Index, &Base)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", Index, Base);
}

static PyObject *
dbgsyms_GetModuleParameters(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Base;
    DEBUG_MODULE_PARAMETERS Params = {0};
    PyDebugModuleParametersObject *obj = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:GetModuleParameters", &Base))
        return NULL;

    if ((hr = self->syms->GetModuleParameters(1, &Base, 0, &Params)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugModuleParametersObject *)PyObject_CallObject(
            (PyObject *)&PyDebugModuleParametersType, NULL);
    if (obj == NULL)
        return NULL;
    obj->dmp = Params;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsyms_AddSyntheticModule(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Base;
    ULONG Size;
    PCSTR ImagePath = NULL;
    PCSTR ModuleName = NULL;
    ULONG Flags = DEBUG_ADDSYNTHMOD_DEFAULT;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KIss:AddSyntheticModule", &Base, &Size,
                &ImagePath, &ModuleName))
        return NULL;
    if ((hr = self->syms->AddSyntheticModule(Base, Size, ImagePath, ModuleName,
                    Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_RemoveSyntheticModule(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:RemoveSyntheticModule", &Base))
        return NULL;
    if ((hr = self->syms->RemoveSyntheticModule(Base)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetNameByOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PSTR NameBuffer = NULL;
    ULONG NameSize;
    ULONG64 Displacement;
    PyObject *ret = NULL;
    CHAR TmpBuf;


    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:GetNameByOffset", &Offset))
        return NULL;

    // TmpBuf is not optional - returns an error
    hr = self->syms->GetNameByOffset(Offset, &TmpBuf, 1, &NameSize, &Displacement);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    NameBuffer = (PSTR)PyMem_Malloc(NameSize * sizeof(*NameBuffer));
    if (NameBuffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr == self->syms->GetNameByOffset(Offset, NameBuffer, NameSize,
                    NULL, &Displacement)) != S_OK)
    {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("sK", NameBuffer, Displacement);
    }

    if (NameBuffer) PyMem_Free(NameBuffer);
    return ret;
}

static PyObject *
dbgsyms_GetOffsetByName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Symbol;
    ULONG64 Offset;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:GetOffsetByName", &Symbol))
        return NULL;
    if ((hr = self->syms->GetOffsetByName(Symbol, &Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgsyms_Reload(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Module;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:Reload", &Module))
        return NULL;
    if ((hr = self->syms->Reload(Module)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetScope(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    ULONG64 InstructionOffset;
    DEBUG_STACK_FRAME ScopeFrame = {0};
    PyDebugStackFrameObject *sfobj;
    //XXX
    //PVOID ScopeContext;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->GetScope(&InstructionOffset, &ScopeFrame, NULL, 0)) != S_OK)
        return err_dbgeng(hr);
    sfobj = (PyDebugStackFrameObject *)PyObject_CallObject(
            (PyObject *)&PyDebugStackFrameType, NULL);
    if (sfobj == NULL)
        Py_RETURN_NONE;
    sfobj->frame = ScopeFrame;
    return Py_BuildValue("KN", InstructionOffset, sfobj);
}

static PyObject *
dbgsyms_ResetScope(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->ResetScope()) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_SetScopeFrameByIndex(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:SetScopeFrameByIndex", &Index))
        return NULL;
    if ((hr = self->syms->SetScopeFrameByIndex(Index)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetCurrentScopeFrameIndex(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    ULONG Index;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->GetCurrentScopeFrameIndex(&Index)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Index);
}

static PyObject *
dbgsyms_SetScopeFromStoredEvent(PyDebugSymbolsObject *self)
{
    HRESULT hr;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->SetScopeFromStoredEvent()) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetScopeSymbolGroup(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Flags;
    IDebugSymbolGroup2 *Symbols = NULL;
    PyDebugSymbolGroupObject *obj = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:GetScopeSymbolGroup", &Flags))
        return NULL;
    if ((hr = self->syms->GetScopeSymbolGroup2(Flags, NULL, &Symbols)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugSymbolGroupObject *)PyObject_CallObject(
            (PyObject *)&PyDebugSymbolGroupType, NULL);
    if (obj == NULL)
        return NULL;
    obj->sg = Symbols;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsyms_FindSourceFile(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG StartElement;
    PSTR File;
    ULONG Flags;
    ULONG FoundElement;
    PSTR Buffer = NULL;
    ULONG FoundSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "IsI:FindSourceFile", &StartElement,
                &File, &Flags))
        return NULL;
    hr = self->syms->FindSourceFile(StartElement, File, Flags,
                    NULL, NULL, 0, &FoundSize);
    if (hr == E_NOINTERFACE)
        return err_nointerface("File was not found on the source path.");
    else if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(FoundSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->FindSourceFile(StartElement, File, Flags,
                    &FoundElement, Buffer, FoundSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }
    
    ret = Py_BuildValue("(Is)", FoundElement, Buffer);
done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_AppendSourcePath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Addition;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:AppendSourcePath", &Addition))
        return NULL;
    if ((hr = self->syms->AppendSourcePath(Addition)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetSourcePath(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG PathSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    hr = self->syms->GetSourcePath(Buffer, 0, &PathSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(PathSize * sizeof(*Buffer));
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetSourcePath(Buffer, PathSize, NULL)) != S_OK) {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_SetSourcePath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Path;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:SetSourcePath", &Path))
        return NULL;
    if ((hr = self->syms->SetSourcePath(Path)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetSourcePathElement(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    PSTR Buffer = NULL;
    ULONG ElementSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:GetSourcePathElement", &Index))
        return NULL;
    hr = self->syms->GetSourcePathElement(Index, NULL, 0, &ElementSize);
    if (hr == E_NOINTERFACE)
        return err_nointerface("The source path contains fewer than Index elements.");
    else if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(ElementSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->GetSourcePathElement(Index, Buffer, 
                    ElementSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }
    
    ret = Py_BuildValue("s", Buffer);
done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_GetSymbolEntryInformation(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    DEBUG_MODULE_AND_ID Id;
    DEBUG_SYMBOL_ENTRY Info;
    PyDebugSymbolEntryObject *obj = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KK:GetSymbolEntryInformation", 
                &Id.ModuleBase, &Id.Id))
        return NULL;
    if ((hr = self->syms->GetSymbolEntryInformation(&Id, &Info)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugSymbolEntryObject *)PyObject_CallObject(
            (PyObject *)&PyDebugSymbolEntryType, NULL);
    if (obj == NULL)
        return NULL;
    obj->sym = Info;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsyms_GetSymbolEntriesByName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Symbol = NULL;
    PDEBUG_MODULE_AND_ID Ids = NULL;
    ULONG Entries;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:GetSymbolEntriesByName", &Symbol))
        return NULL;

    hr = self->syms->GetSymbolEntriesByName(Symbol, 0, NULL, 0, &Entries);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Ids = (PDEBUG_MODULE_AND_ID)PyMem_Malloc(Entries * sizeof(DEBUG_MODULE_AND_ID));
    if (Ids == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->GetSymbolEntriesByName(Symbol, 0, Ids, 
                    Entries, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    // XXX - Need to build up a list of DEBUG_MODULE_AND_IDs
    ret = Py_BuildValue("I", Entries);
done:
    if (Ids) PyMem_Free(Ids);
    return ret;
}

static PyObject *
dbgsyms_GetSymbolEntryString(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    DEBUG_MODULE_AND_ID Id;
    ULONG Which;
    PSTR Buffer = NULL;
    ULONG StringSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KKI:GetSymbolEntryString", 
                &Id.ModuleBase, &Id.Id, &Which))
        return NULL;
    hr = self->syms->GetSymbolEntryString(&Id, Which, NULL, 0, &StringSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(StringSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->syms->GetSymbolEntryString(&Id, Which, Buffer,
                    StringSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }
    
    ret = Py_BuildValue("s", Buffer);
done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_CreateSymbolGroup(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    IDebugSymbolGroup2 *Group = NULL;
    PyDebugSymbolGroupObject *obj = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->CreateSymbolGroup2(&Group)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugSymbolGroupObject *)PyObject_CallObject(
            (PyObject *)&PyDebugSymbolGroupType, NULL);
    if (obj == NULL)
        return NULL;
    obj->sg = Group;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsyms_EndSymbolMatch(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Handle;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:EndSymbolMatch", &Handle))
        return NULL;
    if ((hr = self->syms->EndSymbolMatch(Handle)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetNextSymbolMatch(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Handle;
    PSTR Buffer = NULL;
    ULONG MatchSize;
    ULONG64 Offset;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:GetNextSymbolMatch", &Handle))
        return NULL;
    hr = self->syms->GetNextSymbolMatch(Handle, NULL, 0, &MatchSize, NULL);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(MatchSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    memset(Buffer, 0, MatchSize + 2);

    if ((hr = self->syms->GetNextSymbolMatch(Handle, Buffer, MatchSize,
                    NULL, &Offset)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }
    ret = Py_BuildValue("Ks", Offset, Buffer);
done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_StartSymbolMatch(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR Pattern = NULL;
    ULONG64 Handle;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:StartSymbolMatch", &Pattern))
        return NULL;
    if ((hr = self->syms->StartSymbolMatch(Pattern, &Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}


static PyObject *
dbgsyms_GetSymbolModule(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Symbol;
    ULONG64 Base;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:GetSymbolModule", &Symbol))
        return NULL;
    if ((hr = self->syms->GetSymbolModule(Symbol, &Base)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Base);
}

static PyObject *
dbgsyms_OutputSymbolByOffset(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG OutputControl;
    ULONG Flags;
    ULONG64 Offset;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "IK:OutputSymbolByOffset", &Flags, &Offset))
        return NULL;
    if ((hr = self->syms->OutputSymbolByOffset(OutputControl,
                    Flags, Offset)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_AddSymbolOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:AddSymbolOptions", &Options))
        return NULL;
    if ((hr = self->syms->AddSymbolOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_SetSymbolOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:SetSymbolOptions", &Options))
        return NULL;
    if ((hr = self->syms->SetSymbolOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetSymbolOptions(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->GetSymbolOptions(&Options)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Options);
}

static PyObject *
dbgsyms_RemoveSymbolOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:RemoveSymbolOptions", &Options))
        return NULL;
    if ((hr = self->syms->RemoveSymbolOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_AppendSymbolPath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Addition;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:AppendSymbolPath", &Addition))
        return NULL;
    if ((hr = self->syms->AppendSymbolPath(Addition)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetSymbolPath(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG PathSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    hr = self->syms->GetSymbolPath(Buffer, 0, &PathSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(PathSize * sizeof(*Buffer));
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetSymbolPath(Buffer, PathSize, NULL)) != S_OK) {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", Buffer);
    }

    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgsyms_SetSymbolPath(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Path;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:SetSymbolPath", &Path))
        return NULL;
    if ((hr = self->syms->SetSymbolPath(Path)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_AddSyntheticSymbol(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Size;
    PSTR Name;
    ULONG Flags;
    DEBUG_MODULE_AND_ID Id;
    PyDebugModuleAndIdObject *obj;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KIsI:AddSyntheticSymbol", &Offset, &Size,
                &Name, &Flags))
        return NULL;
    if ((hr = self->syms->AddSyntheticSymbol(Offset, Size, Name, Flags, &Id)) != S_OK)
        return err_dbgeng(hr);

    obj = (PyDebugModuleAndIdObject *)PyObject_CallObject(
            (PyObject *)&PyDebugModuleAndIdType, NULL);
    if (obj == NULL)
        Py_RETURN_NONE;
    obj->ModuleBase = Id.ModuleBase;
    obj->Id = Id.Id;
    return Py_BuildValue("N", obj);
}

static PyObject *
dbgsyms_RemoveSyntheticSymbol(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    DEBUG_MODULE_AND_ID Id;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "O!:RemoveSyntheticSymbol", 
                &PyDebugModuleAndIdType, &Id))
        return NULL;
    if ((hr = self->syms->RemoveSyntheticSymbol(&Id)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetTypeId(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    PCSTR Name;
    ULONG TypeId;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "Ks:GetTypeId", &Module, &Name))
        return NULL;
    if ((hr = self->syms->GetTypeId(Module, Name, &TypeId)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", TypeId);
}

static PyObject *
dbgsyms_GetOffsetTypeId(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG TypeId;
    ULONG64 Module;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "K:GetOffsetTypeId", &Offset))
        return NULL;
    if ((hr = self->syms->GetOffsetTypeId(Offset, &TypeId, &Module)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", TypeId, Module);
}

static PyObject *
dbgsyms_GetSymbolTypeId(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    PCSTR Symbol;
    ULONG TypeId;
    ULONG64 Module;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "s:GetSymbolTypeId", &Symbol))
        return NULL;
    if ((hr = self->syms->GetSymbolTypeId(Symbol, &TypeId, &Module)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IK", TypeId, Module);
}

static PyObject *
dbgsyms_GetTypeName(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG TypeId;
    PSTR NameBuffer = NULL;
    ULONG NameSize;
    PyObject *ret = NULL;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KI:GetTypeName", &Module, &TypeId))
        return NULL;
    hr = self->syms->GetTypeName(Module, TypeId, NULL, 0, &NameSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    NameBuffer = (PSTR)PyMem_Malloc(NameSize * sizeof(*NameBuffer));
    if (NameBuffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if ((hr = self->syms->GetTypeName(Module, TypeId, NameBuffer, 
                    NameSize, NULL)) != S_OK) {
        err_dbgeng(hr);
    }
    else {
        ret = Py_BuildValue("s", NameBuffer);
    }

    if (NameBuffer) PyMem_Free(NameBuffer);
    return ret;
}

static PyObject *
dbgsyms_GetTypeSize(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Module;
    ULONG TypeId;
    ULONG Size;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "KI:GetTypeSize", &Module, &TypeId))
        return NULL;
    if ((hr = self->syms->GetTypeSize(Module, TypeId, &Size)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", Size);
}

static PyObject *
dbgsyms_AddTypeOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:AddTypeOptions", &Options))
        return NULL;
    if ((hr = self->syms->AddTypeOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_GetTypeOptions(PyDebugSymbolsObject *self)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if ((hr = self->syms->GetTypeOptions(&Options)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Options);
}

static PyObject *
dbgsyms_RemoveTypeOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:RemoveTypeOptions", &Options))
        return NULL;
    if ((hr = self->syms->RemoveTypeOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgsyms_SetTypeOptions(PyDebugSymbolsObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Options;

    if (self->syms == NULL)
        return err_nosyms();
    if (!PyArg_ParseTuple(args, "I:SetTypeOptions", &Options))
        return NULL;
    if ((hr = self->syms->SetTypeOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugSymbols_methods[] = {
    {"GetConstantName", 
     (PyCFunction)dbgsyms_GetConstantName, METH_VARARGS, 
     "GetConstantName(Module, TypeId, Value) -> name\n"
     "  Return the name of the specified constant."
    },
// ReadTypedDataPhysical
    {"OutputTypedDataPhysical", 
     (PyCFunction)dbgsyms_OutputTypedDataPhysical, METH_VARARGS, 
     "OutputTypedDataPhysical(Offset, Module, TypeId, Flags=LONGSTATUS|RADIX)\n"
     "  Formats the contents of a variable in the target computer's physical memory\n"
     "  and sends it to the output callbacks.\n"
     "  Flags:\n"
     "    DEBUG_TYPEOPTS_UNICODE_DISPLAY\n"
     "    DEBUG_TYPEOPTS_LONGSTATUS_DISPLAY\n"
     "    DEBUG_TYPEOPTS_FORCERADIX_OUTPUT"
    },
// WriteTypedDataPhysical
// ReadTypedDataVirtual
    {"OutputTypedDataVirtual", 
     (PyCFunction)dbgsyms_OutputTypedDataVirtual, METH_VARARGS, 
     "OutputTypedDataVirtual(Offset, Module, TypeId, Flags=LONGSTATUS|RADIX)\n"
     "  Formats the contents of a variable in the target computer's virtual memory\n"
     "  and sends it to the output callbacks.\n"
     "  Flags:\n"
     "    DEBUG_TYPEOPTS_UNICODE_DISPLAY\n"
     "    DEBUG_TYPEOPTS_LONGSTATUS_DISPLAY\n"
     "    DEBUG_TYPEOPTS_FORCERADIX_OUTPUT"
    },
// WriteTypedDataVirtual
    {"GetFieldName", 
     (PyCFunction)dbgsyms_GetFieldName, METH_VARARGS, 
     "GetFieldName(Module, TypeId, FieldIndex) -> name\n"
     "  Return the name of a field within a structure."
    },
    {"GetFieldOffset", 
     (PyCFunction)dbgsyms_GetFieldOffset, METH_VARARGS, 
     "GetFieldOffset(Module, TypeId, Field) -> offset\n"
     "  Return the offset of a field from the base address of an instance of a type."
    },
    {"GetFieldTypeAndOffset", 
     (PyCFunction)dbgsyms_GetFieldTypeAndOffset, METH_VARARGS, 
     "GetFieldTypeAndOffset(Module, ContainterTypeId, Field) -> (FieldTypeId,Offset)\n"
     "  Return the type of a field and its offset within a container."
    },
// XXX - GetFunctionEntryByOffset (requires arch dependent FPO_DATA or imgfnentr
    {"AppendImagePath", 
     (PyCFunction)dbgsyms_AppendImagePath, METH_VARARGS, 
     "AppendImagePath(Path)\n"
     "  Append directories to the executable image path."
    },
    {"GetImagePath", 
     (PyCFunction)dbgsyms_GetImagePath, METH_NOARGS, 
     "GetImagePath() -> path\n"
     "  Return the executable image path."
    },
    {"SetImagePath", 
     (PyCFunction)dbgsyms_SetImagePath, METH_VARARGS, 
     "SetImagePath(Path)\n"
     "  Set the executable image path."
    },
    {"GetLineByOffset", 
     (PyCFunction)dbgsyms_GetLineByOffset, METH_VARARGS, 
     "GetLineByOffset(Offset) -> (Line, FileName, Displacement)\n"
     "  Return the source filename and the line number within the source of an\n"
     "  instruction in the target."
    },
    {"GetOffsetByLine", 
     (PyCFunction)dbgsyms_GetOffsetByLine, METH_VARARGS, 
     "GetOffsetByLine(Line, File) -> offset\n"
     "  Return the location of the instruction that corresponds to a specified\n"
     "  line in the source code."
    },
    {"GetModuleByIndex", 
     (PyCFunction)dbgsyms_GetModuleByIndex, METH_VARARGS, 
     "GetModuleByIndex(Index) -> base\n"
     "  Returns the location of the module with the specified index."
    },
    {"GetModuleByModuleName", 
     (PyCFunction)dbgsyms_GetModuleByModuleName, METH_VARARGS, 
     "GetModuleByModuleName(Name, StartIndex) -> (Index, Base)\n"
     "  Searches through the target's modules for one with the specified name."
    },
    {"GetModuleByModuleName2", 
     (PyCFunction)dbgsyms_GetModuleByModuleName2, METH_VARARGS, 
     "GetModuleByModuleName2(Name, StartIndex, Flags) -> (Index, Base)\n"
     "  Searches through the target's modules for one with the specified name.\n"
     "  Flags:\n"
     "    DEBUG_GETMOD_NO_LOADED_MODULES\n"
     "    DEBUG_GETMOD_NO_UNLOADED_MODULES"
    },
    {"GetModuleNames", 
     (PyCFunction)dbgsyms_GetModuleNames, METH_VARARGS, 
     "GetModuleNames(Index, Base=0) -> (ImageName, ModuleName, LoadedName)\n"
     "  Return the names of the specified module."
    },
    {"GetModuleNameString", 
     (PyCFunction)dbgsyms_GetModuleNameString, METH_VARARGS, 
     "GetModuleNameString(Which, Index, Base) -> name\n"
     "  Return the name of the specified module."
    },
    {"GetNumberModules", 
     (PyCFunction)dbgsyms_GetNumberModules, METH_NOARGS, 
     "GetNumberModules() -> (NumberLoaded, NumberUnloaded)\n"
     "  Returns the number of modules in the current process's module list."
    },
    {"GetModuleByOffset", 
     (PyCFunction)dbgsyms_GetModuleByOffset, METH_VARARGS, 
     "GetModuleByOffset(Offset, StartIndex=0) -> (Index, Base)\n"
     "  Searches through the target's modules for one whose memory allocation\n"
     "  includes the specified location."
    },
    {"GetModuleByOffset2", 
     (PyCFunction)dbgsyms_GetModuleByOffset2, METH_VARARGS, 
     "GetModuleByOffset2(Offset, StartIndex=0, Flags=0) -> (Index, Base)\n"
     "  Searches through the target's modules for one whose memory allocation\n"
     "  includes the specified location.\n"
     "  Flags:\n"
     "    DEBUG_GETMOD_NO_LOADED_MODULES\n"
     "    DEBUG_GETMOD_NO_UNLOADED_MODULES"
    },
    {"GetModuleParameters", 
     (PyCFunction)dbgsyms_GetModuleParameters, METH_VARARGS, 
     "GetModuleParameters(Base) -> DEBUG_MODULE_PARAMETERS\n"
     "  Returns parameters for the module in the target."
    },
    {"AddSyntheticModule", 
     (PyCFunction)dbgsyms_AddSyntheticModule, METH_VARARGS, 
     "AddSyntheticModule(Base, Size, Path, Name)\n"
     "  Add a synthetic module to the module list the debugger maintains for\n"
     "  the current process."
    },
    {"RemoveSyntheticModule", 
     (PyCFunction)dbgsyms_RemoveSyntheticModule, METH_VARARGS, 
     "RemoveSyntheticModule(Base)\n"
     "  Removes a synthetic module from the module list the debugger maintains\n"
     "  for the current process."
    },
// GetModuleVersionInformation
    {"GetNameByOffset", 
     (PyCFunction)dbgsyms_GetNameByOffset, METH_VARARGS, 
     "GetNameByOffset(Offset) -> (name, displacement)\n"
     "  Return the name of the symbol at the specified location in the target's\n"
     "  virtual address space."
    },
// GetNearNameByOffset
    {"GetOffsetByName", 
     (PyCFunction)dbgsyms_GetOffsetByName, METH_VARARGS, 
     "GetOffsetByName(Symbol) -> offset\n"
     "  Return the location of a symbol identified by name."
    },
    {"Reload", 
     (PyCFunction)dbgsyms_Reload, METH_VARARGS, 
     "Reload(Module)\n"
     "  Delete the engine's symbol information for the specified module and reload\n"
     "  these symbols as needed."
    },
    {"GetScope", 
     (PyCFunction)dbgsyms_GetScope, METH_NOARGS, 
     "GetScope() -> (offset, DEBUG_STACK_FRAME)\n"
     "  Returns information about the current scope."
    },
    {"ResetScope", 
     (PyCFunction)dbgsyms_ResetScope, METH_NOARGS, 
     "ResetScope()\n"
     "  Resets the current scope to the default scope of the current thread."
    },
// SetScope
    {"SetScopeFrameByIndex", 
     (PyCFunction)dbgsyms_SetScopeFrameByIndex, METH_VARARGS, 
     "SetScopeFrameByIndex(Index)\n"
     "  Sets the current scope to the scope of one of the frames on the call stack."
    },
    {"GetCurrentScopeFrameIndex", 
     (PyCFunction)dbgsyms_GetCurrentScopeFrameIndex, METH_NOARGS, 
     "GetCurrentScopeFrameIndex() -> index\n"
     "  Returns the index of the current stack frame in the call stack."
    },
    {"SetScopeFromStoredEvent", 
     (PyCFunction)dbgsyms_SetScopeFromStoredEvent, METH_NOARGS, 
     "SetScopeFromStoredEvent()\n"
     "  Sets the current scope to the scope of the stored event."
    },
    {"GetScopeSymbolGroup", 
     (PyCFunction)dbgsyms_GetScopeSymbolGroup, METH_VARARGS, 
     "GetScopeSymbolGroup(Flags) -> IDebugSymbolGroup\n"
     "  Return a symbol group containing the symbols in the current target's scope."
    },
    {"FindSourceFile", 
     (PyCFunction)dbgsyms_FindSourceFile, METH_VARARGS, 
     "FindSourceFile(StartElement, File, Flags) -> (FoundElement, path)\n"
     "  Search the source path for a specified source file."
    },
// GetSourceFileLineOffsets
    {"AppendSourcePath", 
     (PyCFunction)dbgsyms_AppendSourcePath, METH_VARARGS, 
     "AppendSourcePath(Path)\n"
     "  Append directories to the source path."
    },
    {"GetSourcePath", 
     (PyCFunction)dbgsyms_GetSourcePath, METH_NOARGS, 
     "GetSourcePath() -> path\n"
     "  Return the source path."
    },
    {"SetSourcePath", 
     (PyCFunction)dbgsyms_SetSourcePath, METH_VARARGS, 
     "SetSourcePath(Path)\n"
     "  Set the source path."
    },
    {"GetSourcePathElement", 
     (PyCFunction)dbgsyms_GetSourcePathElement, METH_VARARGS, 
     "GetSourcePathElement(Index) -> SourcePathElement\n"
     "  Return an element from the source path."
    },
// GetSourceEntriesByLine
// XXX - GetSourceEntriesByOffset (undocumented)
    {"GetSymbolEntryInformation", 
     (PyCFunction)dbgsyms_GetSymbolEntryInformation, METH_VARARGS, 
     "GetSymbolEntryInformation(ModuleBase, Id) -> DEBUG_SYMBOL_ENTRY\n"
     "  Returns the symbol entry information for a symbol."
    },
    {"GetSymbolEntriesByName", 
     (PyCFunction)dbgsyms_GetSymbolEntriesByName, METH_VARARGS, 
     "GetSymbolEntriesByName(Symbol) -> Entries\n"
     "  Return the symbols whose names match a given pattern."
    },
// GetSymbolEntriesByOffset
    {"GetSymbolEntryString", 
     (PyCFunction)dbgsyms_GetSymbolEntryString, METH_VARARGS, 
     "GetSymbolEntryString(ModuleBase, Id, Which) -> name\n"
     "  Return string information for the specified symbol."
    },
    {"CreateSymbolGroup", 
     (PyCFunction)dbgsyms_CreateSymbolGroup, METH_NOARGS, 
     "CreateSymbolGroup() -> IDebugSymbolGroup\n"
     "  Create a new symbol group."
    },
    {"EndSymbolMatch", 
     (PyCFunction)dbgsyms_EndSymbolMatch, METH_VARARGS, 
     "EndSymbolMatch(Handle)\n"
     "  Releases the resources used by a symbol search."
    },
    {"GetNextSymbolMatch", 
     (PyCFunction)dbgsyms_GetNextSymbolMatch, METH_VARARGS, 
     "GetNextSymbolMatch(Handle) -> (Offset, name)\n"
     "  Return the next symbol found in a symbol search."
    },
    {"StartSymbolMatch", 
     (PyCFunction)dbgsyms_StartSymbolMatch, METH_VARARGS, 
     "StartSymbolMatch(Pattern) -> Handle\n"
     "  Initialize a search for symbols whose names match a given pattern."
    },
    {"GetSymbolModule", 
     (PyCFunction)dbgsyms_GetSymbolModule, METH_VARARGS, 
     "GetSymbolModule(Symbol) -> base\n"
     "  Return the base address of module which contains the specified symbol."
    },
    {"OutputSymbolByOffset", 
     (PyCFunction)dbgsyms_OutputSymbolByOffset, METH_VARARGS, 
     "OutputSymbolByOffset(Flags, Offset)\n"
     "  Looks up a symbol by address and prints the symbol name and other symbol\n"
     "  information to the debugger console."
    },
    {"AddSymbolOptions", 
     (PyCFunction)dbgsyms_AddSymbolOptions, METH_VARARGS, 
     "AddSymbolOptions(Options)\n"
     "  Turns on some of the engine's global symbol options.\n"
     "  Options:\n"
     "    SYMOPT_CASE_INSENSITIVE\n"
     "    SYMOPT_UNDNAME\n"
     "    SYMOPT_DEFERRED_LOADS\n"
     "    SYMOPT_NO_CPP\n"
     "    SYMOPT_LOAD_LINES\n"
     "    SYMOPT_OMAP_FIND_NEAREST\n"
     "    SYMOPT_LOAD_ANYTHING\n"
     "    SYMOPT_IGNORE_CVREC\n"
     "    SYMOPT_NO_UNQUALIFIED_LOADS\n"
     "    SYMOPT_FAIL_CRITICAL_ERRORS\n"
     "    SYMOPT_EXACT_SYMBOLS\n"
     "    SYMOPT_ALLOW_ABSOLUTE_SYMBOLS\n"
     "    SYMOPT_IGNORE_NT_SYMPATH\n"
     "    SYMOPT_INCLUE_32BIT_MODULES\n"
     "    SYMOPT_PUBLICS_ONLY\n"
     "    SYMOPT_AUTO_PUBLICS\n"
     "    SYMOPT_NO_IMAGE_SEARCH\n"
     "    SYMOPT_SECURE\n"
     "    SYMOPT_NO_PROMPTS\n"
     "    SYMOPT_DEBUG"
    },
    {"GetSymbolOptions", 
     (PyCFunction)dbgsyms_GetSymbolOptions, METH_NOARGS, 
     "GetSymbolOptions() -> options\n"
     "  Returns the engine's global symbol options.\n"
     "  Options:\n"
     "    SYMOPT_CASE_INSENSITIVE\n"
     "    SYMOPT_UNDNAME\n"
     "    SYMOPT_DEFERRED_LOADS\n"
     "    SYMOPT_NO_CPP\n"
     "    SYMOPT_LOAD_LINES\n"
     "    SYMOPT_OMAP_FIND_NEAREST\n"
     "    SYMOPT_LOAD_ANYTHING\n"
     "    SYMOPT_IGNORE_CVREC\n"
     "    SYMOPT_NO_UNQUALIFIED_LOADS\n"
     "    SYMOPT_FAIL_CRITICAL_ERRORS\n"
     "    SYMOPT_EXACT_SYMBOLS\n"
     "    SYMOPT_ALLOW_ABSOLUTE_SYMBOLS\n"
     "    SYMOPT_IGNORE_NT_SYMPATH\n"
     "    SYMOPT_INCLUE_32BIT_MODULES\n"
     "    SYMOPT_PUBLICS_ONLY\n"
     "    SYMOPT_AUTO_PUBLICS\n"
     "    SYMOPT_NO_IMAGE_SEARCH\n"
     "    SYMOPT_SECURE\n"
     "    SYMOPT_NO_PROMPTS\n"
     "    SYMOPT_DEBUG"
    },
    {"RemoveSymbolOptions", 
     (PyCFunction)dbgsyms_RemoveSymbolOptions, METH_VARARGS, 
     "RemoveSymbolOptions(Options)\n"
     "  Turns off some of the engine's global symbol options.\n"
     "  Options:\n"
     "    SYMOPT_CASE_INSENSITIVE\n"
     "    SYMOPT_UNDNAME\n"
     "    SYMOPT_DEFERRED_LOADS\n"
     "    SYMOPT_NO_CPP\n"
     "    SYMOPT_LOAD_LINES\n"
     "    SYMOPT_OMAP_FIND_NEAREST\n"
     "    SYMOPT_LOAD_ANYTHING\n"
     "    SYMOPT_IGNORE_CVREC\n"
     "    SYMOPT_NO_UNQUALIFIED_LOADS\n"
     "    SYMOPT_FAIL_CRITICAL_ERRORS\n"
     "    SYMOPT_EXACT_SYMBOLS\n"
     "    SYMOPT_ALLOW_ABSOLUTE_SYMBOLS\n"
     "    SYMOPT_IGNORE_NT_SYMPATH\n"
     "    SYMOPT_INCLUE_32BIT_MODULES\n"
     "    SYMOPT_PUBLICS_ONLY\n"
     "    SYMOPT_AUTO_PUBLICS\n"
     "    SYMOPT_NO_IMAGE_SEARCH\n"
     "    SYMOPT_SECURE\n"
     "    SYMOPT_NO_PROMPTS\n"
     "    SYMOPT_DEBUG"
    },
    {"SetSymbolOptions", 
     (PyCFunction)dbgsyms_SetSymbolOptions, METH_VARARGS, 
     "SetSymbolOptions(Options)\n"
     "  Changes the engine's global symbol options.\n"
     "  Options:\n"
     "    SYMOPT_CASE_INSENSITIVE\n"
     "    SYMOPT_UNDNAME\n"
     "    SYMOPT_DEFERRED_LOADS\n"
     "    SYMOPT_NO_CPP\n"
     "    SYMOPT_LOAD_LINES\n"
     "    SYMOPT_OMAP_FIND_NEAREST\n"
     "    SYMOPT_LOAD_ANYTHING\n"
     "    SYMOPT_IGNORE_CVREC\n"
     "    SYMOPT_NO_UNQUALIFIED_LOADS\n"
     "    SYMOPT_FAIL_CRITICAL_ERRORS\n"
     "    SYMOPT_EXACT_SYMBOLS\n"
     "    SYMOPT_ALLOW_ABSOLUTE_SYMBOLS\n"
     "    SYMOPT_IGNORE_NT_SYMPATH\n"
     "    SYMOPT_INCLUE_32BIT_MODULES\n"
     "    SYMOPT_PUBLICS_ONLY\n"
     "    SYMOPT_AUTO_PUBLICS\n"
     "    SYMOPT_NO_IMAGE_SEARCH\n"
     "    SYMOPT_SECURE\n"
     "    SYMOPT_NO_PROMPTS\n"
     "    SYMOPT_DEBUG"
    },
    {"AppendSymbolPath", 
     (PyCFunction)dbgsyms_AppendSymbolPath, METH_VARARGS, 
     "AppendSymbolPath(Path)\n"
     "  Append directories to the symbol path."
    },
    {"GetSymbolPath", 
     (PyCFunction)dbgsyms_GetSymbolPath, METH_NOARGS, 
     "GetSymbolPath() -> path\n"
     "  Return the symbol path."
    },
    {"SetSymbolPath", 
     (PyCFunction)dbgsyms_SetSymbolPath, METH_VARARGS, 
     "SetSymbolPath(Path)\n"
     "  Set the symbol path."
    },
    {"AddSyntheticSymbol", 
     (PyCFunction)dbgsyms_AddSyntheticSymbol, METH_VARARGS, 
     "AddSyntheticSymbol(Offset, Size, Name, Flags) -> DEBUG_MODULE_AND_ID\n"
     "  Add a synthetic symbol to a module in the current process."
    },
    {"RemoveSyntheticSymbol", 
     (PyCFunction)dbgsyms_RemoveSyntheticSymbol, METH_VARARGS, 
     "RemoveSyntheticSymbol(Id)\n"
     "  Removes a synthetic symbol from a module in the current process."
    },
    {"GetTypeId", 
     (PyCFunction)dbgsyms_GetTypeId, METH_VARARGS, 
     "GetTypeId(Module, Name) -> TypeId\n"
     "  Look up the specified type and return its type ID."
    },
    {"GetOffsetTypeId", 
     (PyCFunction)dbgsyms_GetOffsetTypeId, METH_VARARGS, 
     "GetOffsetTypeId(Offset) -> (TypeId, Module)\n"
     "  Returns the type ID of the symbol closest to the specified memory location."
    },
    {"GetSymbolTypeId", 
     (PyCFunction)dbgsyms_GetSymbolTypeId, METH_VARARGS, 
     "GetSymbolTypeId(Symbol) -> (TypeId, Module)\n"
     "  Return the Type ID and module of the specified symbol."
    },
    {"GetTypeName", 
     (PyCFunction)dbgsyms_GetTypeName, METH_VARARGS, 
     "GetTypeName(Module, TypeId) -> name\n"
     "  Return the name of the type symbol specified by its type ID and module."
    },
    {"GetTypeSize", 
     (PyCFunction)dbgsyms_GetTypeSize, METH_VARARGS, 
     "GetTypeSize(Module, TypeId) -> size\n"
     "  Returns the number of bytes of memory an instance of the specified type\n"
     "  requires."
    },
    {"AddTypeOptions", 
     (PyCFunction)dbgsyms_AddTypeOptions, METH_VARARGS, 
     "AddTypeOptions(Options)\n"
     "  Turns on some type formatting options for output generated by the engine."
    },
    {"GetTypeOptions", 
     (PyCFunction)dbgsyms_GetTypeOptions, METH_NOARGS, 
     "GetTypeOptions() -> options\n"
     "  Returns the type formatting options for output generated by the engine."
    },
    {"RemoveTypeOptions", 
     (PyCFunction)dbgsyms_RemoveTypeOptions, METH_VARARGS, 
     "RemoveTypeOptions(Options)\n"
     "  Turns off some type formatting options for output generated by the engine."
    },
    {"SetTypeOptions", 
     (PyCFunction)dbgsyms_SetTypeOptions, METH_VARARGS, 
     "SetTypeOptions(Options)\n"
     "  Changes the type fomratting options for output generated by the engine."
    },

    {NULL, NULL}
};

PyTypeObject PyDebugSymbolsType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSymbols",   /*tp_name*/
    sizeof(PyDebugSymbolsObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgsyms_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "DebugSymbols objects",    /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugSymbolsObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugSymbols_methods,    /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsyms_init,              /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgsyms_new,               /* tp_new */
    PyObject_Del,              /* tp_free */
};


