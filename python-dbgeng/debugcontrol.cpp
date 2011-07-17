#include "pydbgeng.h"

static PyObject *
dbgcontrol_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugControlObject *self;

    self = (PyDebugControlObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->control = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgcontrol_dealloc(PyDebugControlObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->control != NULL) {
        self->control->Release();
        self->control = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int dbgcontrol_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugControlObject *self = (PyDebugControlObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugControl4 *debugControl = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->control) {
        self->control->Release();
        self->control = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugControl",
                kwlist, &PyDebugClientType, &client))
        return -1;

    if ((hr = client->client->QueryInterface(__uuidof(IDebugControl4),
                    (void **)&debugControl)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->control = debugControl;
    return 0;
}

static PyObject *
dbgcontrol_Assemble(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 Offset = 0;
    PCSTR Instr;
    ULONG64 EndOffset;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "Ks:Assemble", &Offset, &Instr))
        return NULL;
    if ((hr = self->control->Assemble(Offset, Instr, &EndOffset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", EndOffset);
}

static PyObject *
dbgcontrol_AddAssemblyOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:AddAssemblyOptions", &Options))
        return NULL;
    if ((hr = self->control->AddAssemblyOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetAssemblyOptions(PyDebugControlObject *self)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetAssemblyOptions(&Options)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Options);
}

static PyObject *
dbgcontrol_RemoveAssemblyOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:RemoveAssemblyOptions", &Options))
        return NULL;
    if ((hr = self->control->RemoveAssemblyOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_SetAssemblyOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetAssemblyOptions", &Options))
        return NULL;
    if ((hr = self->control->SetAssemblyOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_Disassemble(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 offset = 0;
    ULONG flags = DEBUG_DISASM_EFFECTIVE_ADDRESS;
    ULONG size;
    ULONG64 end;
    HRESULT hr;
    char buf[512];

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "K|I:Disassemble", &offset, &flags))
        return NULL;
    if ((hr = self->control->Disassemble(offset, flags, buf, 512, 
                    &size, &end)) != S_OK)
    {
        return err_dbgeng(hr);
    }
    buf[size] = '\0';
    return Py_BuildValue("sK", buf, end);
}

/*
 * XXX - Need DEBUG_VALUE convertor
 *  maybe some switch statement which Py_Builds the correct thing
 *
 */
static PyObject *
dbgcontrol_Evaluate(PyDebugControlObject *self, PyObject *args)
{
    PCSTR Expression;
    DEBUG_VALUE Value;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s:Evaluate", &Expression))
        return NULL;
    if ((hr = self->control->Evaluate(Expression, DEBUG_VALUE_INT64,
                    &Value, NULL)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("K", Value.I64);
}

static PyObject *
dbgcontrol_Execute(PyDebugControlObject *self, PyObject *args)
{
    PCSTR Command;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s:Execute", &Command))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->Execute(DEBUG_OUTCTL_ALL_CLIENTS, Command, 0);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_ExecuteCommandFile(PyDebugControlObject *self, PyObject *args)
{
    PCSTR CommandFile;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s:ExecuteCommandFile", &CommandFile))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->Execute(DEBUG_OUTCTL_ALL_CLIENTS, 
                        CommandFile, 0);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_AddBreakpoint(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Type = DEBUG_BREAKPOINT_CODE;
    ULONG DesiredId = DEBUG_ANY_ID;
    IDebugBreakpoint *bp = NULL;
    PyDebugBreakpointObject *obp;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "|II:AddBreakpoint", &Type, &DesiredId))
        return NULL;
    if (Type != DEBUG_BREAKPOINT_CODE && Type != DEBUG_BREAKPOINT_DATA) {
        PyErr_SetString(PyExc_TypeError, "Invalid type");
        return NULL;
    }
    if ((hr = self->control->AddBreakpoint(Type, DesiredId, &bp)) != S_OK) {
        if (hr == E_INVALIDARG)
            return err_invalidarg("The breakpoint couldn't be created with the"
                "desired ID or the value of Type was not recognized.");
        else
            return err_dbgeng(hr);
    }

    obp = (PyDebugBreakpointObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakpointType, NULL);
    if (obp == NULL)
        Py_RETURN_NONE;
    obp->bp = bp;
    return Py_BuildValue("N", obp);
}

static PyObject *
dbgcontrol_RemoveBreakpoint(PyDebugControlObject *self, PyObject *args)
{
    PyDebugBreakpointObject *bp = NULL;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "O!:RemoveBreakpoint", &PyDebugBreakpointType, &bp))
        return NULL;
    if (bp->bp == NULL)
        return err_nobp();
    if ((hr = self->control->RemoveBreakpoint(bp->bp)) != S_OK)
        return err_dbgeng(hr);
    bp->bp = NULL;
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetBreakpointById(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    PyDebugBreakpointObject *obp;
    IDebugBreakpoint *bp = NULL;
    ULONG Id;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:GetBreakpointById", &Id))
        return NULL;
    if ((hr = self->control->GetBreakpointById(Id, &bp)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("No breakpoint was found with the given ID.");
        else
            return err_dbgeng(hr);
    }

    obp = (PyDebugBreakpointObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakpointType, NULL);
    if (obp == NULL)
        Py_RETURN_NONE;
    obp->bp = bp;
    return Py_BuildValue("N", obp);
}

static PyObject *
dbgcontrol_GetBreakpointByIndex(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    PyDebugBreakpointObject *obp;
    IDebugBreakpoint *bp = NULL;
    ULONG Index;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:GetBreakpointByIndex", &Index))
        return NULL;
    if ((hr = self->control->GetBreakpointByIndex(Index, &bp)) != S_OK) {
        if (hr == E_NOINTERFACE)
            return err_nointerface("No breakpoint was found with the given Index.");
        else
            return err_dbgeng(hr);
    }

    obp = (PyDebugBreakpointObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakpointType, NULL);
    if (obp == NULL)
        Py_RETURN_NONE;
    obp->bp = bp;
    return Py_BuildValue("N", obp);
}

static PyObject *
dbgcontrol_GetNumberBreakpoints(PyDebugControlObject *self)
{
    ULONG Number;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetNumberBreakpoints(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgcontrol_GetBreakpointParameters(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    DEBUG_BREAKPOINT_PARAMETERS params;
    PyDebugBreakPointParametersObject *obp;
    ULONG Start;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:GetBreakpointParameters", &Start))
        return NULL;
    // XXX - Could react differently for S_FALSE
    if ((hr = self->control->GetBreakpointParameters(1, NULL, Start, &params)) != S_OK)
        return err_dbgeng(hr);
    obp = (PyDebugBreakPointParametersObject *)PyObject_CallObject(
            (PyObject *)&PyDebugBreakPointParametersType, NULL);
    if (obp == NULL)
        Py_RETURN_NONE;
    obp->bpp = params;
    return Py_BuildValue("N", obp);
}

static PyObject *
dbgcontrol_ReadBugCheckData(PyDebugControlObject *self)
{
    ULONG Code;
    ULONG64 Arg1;
    ULONG64 Arg2;
    ULONG64 Arg3;
    ULONG64 Arg4;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->ReadBugCheckData(&Code, &Arg1, &Arg2, 
                    &Arg3, &Arg4)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IKKKK", Code, Arg1, Arg2, Arg3, Arg4);
}

static PyObject *
dbgcontrol_GetCodeLevel(PyDebugControlObject *self)
{
    ULONG Level;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetCodeLevel(&Level)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Level);
}

static PyObject *
dbgcontrol_SetCodeLevel(PyDebugControlObject *self, PyObject *args)
{
    ULONG Level;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetCodeLevel", &Level))
        return NULL;
    if ((hr = self->control->SetCodeLevel(Level)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetDebuggeeType(PyDebugControlObject *self)
{
    ULONG Class;
    ULONG Qualifier;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetDebuggeeType(&Class, &Qualifier)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("II", Class, Qualifier);
}

static PyObject *
dbgcontrol_GetDisassembleEffectiveOffset(PyDebugControlObject *self)
{
    ULONG64 Offset;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetDisassembleEffectiveOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgcontrol_OutputDisassembly(PyDebugControlObject *self, PyObject *args)
{
    ULONG OutputControl;
    ULONG64 Offset;
    ULONG Flags;
    ULONG64 EndOffset;
    HRESULT hr;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    Flags = DEBUG_DISASM_EFFECTIVE_ADDRESS | DEBUG_DISASM_MATCHING_SYMBOLS |
            DEBUG_DISASM_SOURCE_LINE_NUMBER | DEBUG_DISASM_SOURCE_FILE_NAME;
    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "K|I:OutputDisassembly", &Offset, &Flags))
        return NULL;
    if ((hr = self->control->OutputDisassembly(OutputControl, Offset,
                    Flags, &EndOffset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", EndOffset);
}

static PyObject *
dbgcontrol_OutputDisassemblyLines(PyDebugControlObject *self, PyObject *args)
{
    ULONG OutputControl;
    ULONG PreviousLines;
    ULONG TotalLines;
    ULONG64 Offset;
    ULONG Flags;
    
    ULONG OffsetLine;
    ULONG64 StartOffset;
    ULONG64 EndOffset;
    // XXX should be a list - ULONG64 LineOffsets;
    HRESULT hr;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
    Flags = DEBUG_DISASM_EFFECTIVE_ADDRESS | DEBUG_DISASM_MATCHING_SYMBOLS |
            DEBUG_DISASM_SOURCE_LINE_NUMBER | DEBUG_DISASM_SOURCE_FILE_NAME;
    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "IIK|I:OutputDisassembly", 
                &PreviousLines, &TotalLines, &Offset, &Flags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->OutputDisassemblyLines(OutputControl,
                        PreviousLines, TotalLines, Offset, Flags,
                        &OffsetLine, &StartOffset, &EndOffset, NULL);
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("IKK", OffsetLine, StartOffset, EndOffset);
}

static PyObject *
dbgcontrol_GetDumpFormatFlags(PyDebugControlObject *self)
{
    ULONG FormatFlags;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetDumpFormatFlags(&FormatFlags)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", FormatFlags);
}

static PyObject *
dbgcontrol_AddEngineOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:AddEngineOptions", &Options))
        return NULL;
    if ((hr = self->control->AddEngineOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetEngineOptions(PyDebugControlObject *self)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetEngineOptions(&Options)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Options);
}

static PyObject *
dbgcontrol_RemoveEngineOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:RemoveEngineOptions", &Options))
        return NULL;
    if ((hr = self->control->RemoveEngineOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_SetEngineOptions(PyDebugControlObject *self, PyObject *args)
{
    ULONG Options;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetEngineOptions", &Options))
        return NULL;
    if ((hr = self->control->SetEngineOptions(Options)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetSystemErrorControl(PyDebugControlObject *self)
{
    HRESULT hr;
    ULONG OutputLevel;
    ULONG BreakLevel;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetSystemErrorControl(&OutputLevel, 
                    &BreakLevel)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("II", OutputLevel, BreakLevel);
}

static PyObject *
dbgcontrol_SetSystemErrorControl(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG OutputLevel;
    ULONG BreakLevel;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "II:SetSystemErrorControl", 
                &OutputLevel, &BreakLevel))
        return NULL;
    if ((hr = self->control->SetSystemErrorControl(OutputLevel, 
                    BreakLevel)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetNotifyEventHandle(PyDebugControlObject *self)
{
    ULONG64 Handle;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetNotifyEventHandle(&Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}

static PyObject *
dbgcontrol_SetNotifyEventHandle(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 Handle;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "K:SetNotifyEventHandle", &Handle))
        return NULL;
    if ((hr = self->control->SetNotifyEventHandle(Handle)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetNumberEvents(PyDebugControlObject *self)
{
    ULONG Events;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    hr = self->control->GetNumberEvents(&Events);
    if (hr == S_OK || hr == S_FALSE)
        return Py_BuildValue("I", Events);
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgcontrol_GetCurrentEventIndex(PyDebugControlObject *self)
{
    ULONG Index;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetCurrentEventIndex(&Index)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Index);
}

static PyObject *
dbgcontrol_SetNextEventIndex(PyDebugControlObject *self, PyObject *args)
{
    ULONG Relation;
    ULONG Value;
    ULONG NextIndex;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "II:SetNextEventIndex", &Relation, &Value))
        return NULL;
    if ((hr = self->control->SetNextEventIndex(Relation, Value, &NextIndex)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", NextIndex);
}

static PyObject *
dbgcontrol_GetEventIndexDescription(PyDebugControlObject *self, PyObject *args)
{
    ULONG Index;
    ULONG Which;
    PSTR Buffer = NULL;;
    ULONG DescSize = 0;
    PyObject *ret = NULL;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "II:GetEventIndexDescription", &Index, &Which))
        return NULL;
    hr = self->control->GetEventIndexDescription(Index, Which, 
                    NULL, 0, &DescSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);

    Buffer = (PSTR)PyMem_Malloc(DescSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    if ((hr = self->control->GetEventIndexDescription(Index, Which, 
                    Buffer, DescSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto error;
    }
    ret = Py_BuildValue("s", Buffer);
error:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgcontrol_GetExpressionSyntax(PyDebugControlObject *self)
{
    HRESULT hr;
    ULONG Flags;
    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetExpressionSyntax(&Flags)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Flags);
}

static PyObject *
dbgcontrol_SetExpressionSyntax(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Flags;
    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetExpressionSyntax", &Flags))
        return NULL;
    if ((hr = self->control->SetExpressionSyntax(Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetExpressionSyntaxNames(PyDebugControlObject *self, PyObject *args)
{
    return err_notimpl();
}

static PyObject *
dbgcontrol_SetExpressionSyntaxByName(PyDebugControlObject *self, PyObject *args)
{
    return err_notimpl();
}

static PyObject *
dbgcontrol_GetNumberExpressionSyntaxes(PyDebugControlObject *self)
{
    return err_notimpl();
}

static PyObject *
dbgcontrol_AddExtension(PyDebugControlObject *self, PyObject *args)
{
    PSTR Path;
    ULONG64 Handle;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s:AddExtension", &Path))
        return NULL;
    if ((hr = self->control->AddExtension(Path, 0, &Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}

static PyObject *
dbgcontrol_RemoveExtension(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 Handle;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "K:RemoveExtension", &Handle))
        return NULL;
    if ((hr = self->control->RemoveExtension(Handle)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_CallExtension(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 Handle;
    PSTR Function;
    PSTR Arguments;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "Ks|s:CallExtension", &Handle, &Function, &Arguments))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->CallExtension(Handle, Function, Arguments); 
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetExtensionByPath(PyDebugControlObject *self, PyObject *args)
{
    PSTR Path;
    ULONG64 Handle;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s:GetExtensionByPath", &Path))
        return NULL;
    if ((hr = self->control->GetExtensionByPath(Path, &Handle)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Handle);
}

static PyObject *
dbgcontrol_GetExtensionFunction(PyDebugControlObject *self, PyObject *args)
{
    return err_notimpl();
}

static PyObject *
dbgcontrol_GetWindbgExtensionApis64(PyDebugControlObject *self)
{
    return err_notimpl();
}

static PyObject *
dbgcontrol_WaitForEvent(PyDebugControlObject *self, PyObject *args)
{
    ULONG Timeout = INFINITE;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "|I:WaitForEvent", &Timeout))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->WaitForEvent(0, Timeout);
    Py_END_ALLOW_THREADS
    switch (hr) 
    {
    case S_OK:
        Py_RETURN_TRUE;
    case S_FALSE:
        return exc_timeout("the time-out expired");
    case E_PENDING:
        return err_pending("An exit interrupt was issued. "
            "The target is not available.");
    case E_UNEXPECTED:
        return err_unexpected("Either there is an outstanding request for input, "
            "or non of the targets could generate events.");
    case E_FAIL:
        return err_fail("The engine is already waiting for an event");
    default:
        return err_dbgeng(hr);
    }
}

static PyObject *
dbgcontrol_GetNearInstruction(PyDebugControlObject *self, PyObject *args)
{
    ULONG64 Offset;
    LONG Delta;
    ULONG64 NearOffset;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "KI:GetNearInstruction", &Offset, &Delta))
        return NULL;
    if ((hr = self->control->GetNearInstruction(Offset, Delta, &NearOffset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", NearOffset);
}

static PyObject *
dbgcontrol_GetInterrupt(PyDebugControlObject *self)
{
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->GetInterrupt();
    Py_END_ALLOW_THREADS
    if (hr == S_OK)
        Py_RETURN_TRUE;
    else if (hr == S_FALSE)
        Py_RETURN_FALSE;
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgcontrol_SetInterrupt(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Flags = DEBUG_INTERRUPT_ACTIVE;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "|I:SetInterrupt", &Flags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = self->control->SetInterrupt(Flags); 
    Py_END_ALLOW_THREADS
    if (hr != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetInterruptTimeout(PyDebugControlObject *self)
{
    HRESULT hr;
    ULONG Seconds;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetInterruptTimeout(&Seconds)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Seconds);
}

static PyObject *
dbgcontrol_SetInterruptTimeout(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Seconds;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetInterruptTimeout", &Seconds))
        return NULL;
    if ((hr = self->control->SetInterruptTimeout(Seconds)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_CloseLogFile(PyDebugControlObject *self)
{
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->CloseLogFile()) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetLogFile(PyDebugControlObject *self)
{
    HRESULT hr;
    PSTR Buffer = NULL;
    ULONG FileSize;
    BOOL Append;
    PyObject *ret = NULL;

    if (self->control == NULL)
        return err_nocontrol();

    hr = self->control->GetLogFile(NULL, 0, &FileSize, &Append);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    Buffer = (PSTR)PyMem_Malloc(FileSize + 2);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->control->GetLogFile(Buffer, FileSize, NULL, &Append)) != S_OK) {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("(sO)", Buffer, Append ? Py_True : Py_False);
done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgcontrol_OpenLogFile(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    PSTR File;
    BOOL Append = FALSE;
    PyObject *obj = NULL;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "s|O:OpenLogFile", &File, &obj))
        return NULL;
    if (obj != NULL) {
        Append = PyObject_IsTrue(obj) ? TRUE : FALSE;
    }
    if ((hr = self->control->OpenLogFile(File, Append)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetLogMask(PyDebugControlObject *self)
{
    ULONG Mask;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetLogMask(&Mask)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Mask);
}

static PyObject *
dbgcontrol_SetLogMask(PyDebugControlObject *self, PyObject *args)
{
    ULONG Mask;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetLogMask", &Mask))
        return NULL;
    if ((hr = self->control->SetLogMask(Mask)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetReturnOffset(PyDebugControlObject *self)
{
    ULONG64 Offset;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetReturnOffset(&Offset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Offset);
}

static PyObject *
dbgcontrol_GetPageSize(PyDebugControlObject *self)
{
    ULONG Size;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetPageSize(&Size)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Size);
}

static PyObject *
dbgcontrol_IsPointer64Bit(PyDebugControlObject *self)
{
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    hr = self->control->IsPointer64Bit(); 
    if (hr == S_OK)
        Py_RETURN_TRUE;
    else if (hr == S_FALSE)
        Py_RETURN_FALSE;
    else
        return err_dbgeng(hr);
}

static PyObject *
dbgcontrol_GetActualProcessorType(PyDebugControlObject *self)
{
    ULONG Type;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetActualProcessorType(&Type)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Type);
}

static PyObject *
dbgcontrol_GetEffectiveProcessorType(PyDebugControlObject *self)
{
    ULONG Type;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetEffectiveProcessorType(&Type)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Type);
}

static PyObject *
dbgcontrol_SetEffectiveProcessorType(PyDebugControlObject *self, PyObject *args)
{
    ULONG Type;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetEffectiveProcessorType", &Type))
        return NULL;
    if ((hr = self->control->SetEffectiveProcessorType(Type)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetExecutingProcessorType(PyDebugControlObject *self)
{
    ULONG Type;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetExecutingProcessorType(&Type)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Type);
}

static PyObject *
dbgcontrol_GetNumberProcessors(PyDebugControlObject *self)
{
    ULONG Number;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetNumberProcessors(&Number)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Number);
}

static PyObject *
dbgcontrol_GetRadix(PyDebugControlObject *self)
{
    ULONG Radix;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetRadix(&Radix)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Radix);
}

static PyObject *
dbgcontrol_SetRadix(PyDebugControlObject *self, PyObject *args)
{
    ULONG Radix;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetRadix", &Radix))
        return NULL;
    if ((hr = self->control->SetRadix(Radix)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetStackTrace(PyDebugControlObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 FrameOffset = 0;
    ULONG64 StackOffset = 0;
    ULONG64 InstructionOffset = 0;
    PDEBUG_STACK_FRAME Frames = NULL;
    ULONG FrameSize = 50;
    ULONG FramesFilled;
    PyObject *ret = NULL;
    ULONG i;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "|KKK:GetStackTrace", 
                &FrameOffset, &StackOffset, &InstructionOffset))
        return NULL;

    Frames = (PDEBUG_STACK_FRAME)PyMem_Malloc(FrameSize * sizeof(DEBUG_STACK_FRAME));
    if (Frames == NULL) {
        PyErr_NoMemory();
        goto error;
    }
    memset(Frames, 0, FrameSize * sizeof(DEBUG_STACK_FRAME));

    hr = self->control->GetStackTrace(FrameOffset, StackOffset,
            InstructionOffset, Frames, FrameSize, &FramesFilled);
    if (hr == E_FAIL)
        FramesFilled = 0;
    else if (hr != S_OK) {
        err_dbgeng(hr);
        goto error;
    }

    if ((ret = PyList_New(FramesFilled)) == NULL)
        goto error;

    for (i = 0; i < FramesFilled; i++) {
        PyDebugStackFrameObject *obj;
        obj = (PyDebugStackFrameObject *)PyObject_CallObject(
                (PyObject *)&PyDebugStackFrameType, NULL);
        if (obj == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            goto error;
        }
        obj->frame = Frames[i];
        PyList_SET_ITEM(ret, i, (PyObject *)obj);
    }

error:
    if (Frames) PyMem_Free(Frames);
    return ret;
}

static PyObject *
dbgcontrol_OutputStackTrace(PyDebugControlObject *self, PyObject *args)
{
    ULONG OutputControl;
    ULONG FramesSize;
    ULONG Flags;
    HRESULT hr;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "II:OutputStackTrace", &FramesSize, &Flags))
        return NULL;
    if ((hr = self->control->OutputStackTrace(OutputControl, NULL, 
                    FramesSize, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_OutputCurrentState(PyDebugControlObject *self, PyObject *args)
{
    ULONG OutputControl;
    ULONG Flags = DEBUG_CURRENT_DEFAULT;
    HRESULT hr;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "|I:OutputCurrentState", &Flags))
        return NULL;
    if ((hr = self->control->OutputCurrentState(OutputControl, Flags)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyObject *
dbgcontrol_GetExecutionStatus(PyDebugControlObject *self)
{
    ULONG Status;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->GetExecutionStatus(&Status)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Status);
}

static PyObject *
dbgcontrol_SetExecutionStatus(PyDebugControlObject *self, PyObject *args)
{
    ULONG Status;
    HRESULT hr;

    if (self->control == NULL)
        return err_nocontrol();
    if (!PyArg_ParseTuple(args, "I:SetExecutionStatus", &Status))
        return NULL;
    hr = self->control->SetExecutionStatus(Status);
    switch (hr)
    {
    case S_OK:
        Py_RETURN_NONE;
    case E_UNEXPECTED:
        return err_unexpected("Something prevented the execution of this "
            "method. Possible causes include: there is no current target, "
            "there is an outstanding request for input, or execution is "
            "not supported in the current target.");
    case E_ACCESSDENIED:
        return err_fail("The target is already executing.");
    case E_NOINTERFACE:
        return err_nointerface("target cannot generate any more events.");
    default:
        return err_dbgeng(hr);
    }
}

static PyObject *
dbgcontrol_OutputVersionInformation(PyDebugControlObject *self)
{
    ULONG OutputControl;
    HRESULT hr;

    OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;

    if (self->control == NULL)
        return err_nocontrol();
    if ((hr = self->control->OutputVersionInformation(OutputControl)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

static PyMethodDef PyDebugControl_methods[] = {
    {"Assemble", 
     (PyCFunction)dbgcontrol_Assemble, METH_VARARGS | METH_KEYWORDS, 
     "Assemble(offset, instr) -> end offset\n"
     "  Assemble a single processor instruction in target memory."
    },
    {"AddAssemblyOptions", 
     (PyCFunction)dbgcontrol_AddAssemblyOptions, METH_VARARGS, 
     "AddAssemblyOptions(options)\n"
     "  Turns on some of the assembly and disassembly options.\n"
     "  Options:\n"
     "    DEBUG_ASMOPT_VERBOSE\n"
     "    DEBUG_ASMOPT_NO_CODE_BYTES\n"
     "    DEBUG_ASMOPT_IGNORE_OUTPUT_WIDTH\n"
     "    DEBUG_ASMOPT_SOURCE_LINE_NUMBER"
    },
    {"GetAssemblyOptions", 
     (PyCFunction)dbgcontrol_GetAssemblyOptions, METH_NOARGS, 
     "GetAssemblyOptions() -> options\n"
     "  Returns the assembly and disassembly options."
    },
    {"RemoveAssemblyOptions", 
     (PyCFunction)dbgcontrol_RemoveAssemblyOptions, METH_VARARGS, 
     "RemoveAssemblyOptions(options)\n"
     "  Turns off some of the assembly and disassembly options."
    },
    {"SetAssemblyOptions", 
     (PyCFunction)dbgcontrol_SetAssemblyOptions, METH_VARARGS, 
     "SetAssemblyOptions(options)\n"
     "  Sets the assembly and disassembly options.\n"
     "  Options:\n"
     "    DEBUG_ASMOPT_VERBOSE\n"
     "    DEBUG_ASMOPT_NO_CODE_BYTES\n"
     "    DEBUG_ASMOPT_IGNORE_OUTPUT_WIDTH\n"
     "    DEBUG_ASMOPT_SOURCE_LINE_NUMBER"
    },
    {"Disassemble", 
     (PyCFunction)dbgcontrol_Disassemble, METH_VARARGS, 
     "Disassemble(offset,flags=DEBUG_DISASM_EFFECTIVE_ADDRESS) -> (disasm, end offset)\n"
     "  Disassemble a processor instruction in the target's memory."
    },
    {"Evaluate", 
     (PyCFunction)dbgcontrol_Evaluate, METH_VARARGS, 
     "Evaluate(expr) -> value\n"
     "  Evaluate an expression."
    },
    {"Execute", 
     (PyCFunction)dbgcontrol_Execute, METH_VARARGS, 
     "Execute(cmd)\n"
     "  Executes the specified debugger commands."
    },
    {"ExecuteCommandFile", 
     (PyCFunction)dbgcontrol_ExecuteCommandFile, METH_VARARGS, 
     "ExecuteCommandFile(cmdfile)\n"
     "  Open the specified file and execute the debugger commands within."
    },
    {"AddBreakpoint", 
     (PyCFunction)dbgcontrol_AddBreakpoint, METH_VARARGS, 
     "AddBreakpoint(type=DEBUG_BREAKPOINT_CODE,DesiredId=DEBUG_ANY_ID) -> breakpoint\n"
     "  Create a new breakpoint for the current target.\n"
     "  Type:\n"
     "    DEBUG_BREAKPOINT_CODE\n"
     "    DEBUG_BREAKPOINT_DATA"
    },
    {"RemoveBreakpoint", 
     (PyCFunction)dbgcontrol_RemoveBreakpoint, METH_VARARGS, 
     "RemoveBreakpoint(BP_obj)\n"
     "  Remove a breakpoint"
    },
    {"GetBreakpointById", 
     (PyCFunction)dbgcontrol_GetBreakpointById, METH_VARARGS, 
     "GetBreakpointById(id) -> breakpoint\n"
     "  Return the breakpoint with the specified breakpoint ID."
    },
    {"GetBreakpointByIndex", 
     (PyCFunction)dbgcontrol_GetBreakpointByIndex, METH_VARARGS, 
     "GetBreakpointByIndex(index) -> breakpoint\n"
     "  Return the breakpoint located at the specified index."
    },
    {"GetNumberBreakpoints", 
     (PyCFunction)dbgcontrol_GetNumberBreakpoints, METH_NOARGS, 
     "GetNumberBreakpoints() -> number\n"
     "  Returns the number of breakpoints for the current process."
    },
    {"GetBreakpointParameters", 
     (PyCFunction)dbgcontrol_GetBreakpointParameters, METH_VARARGS, 
     "GetBreakpointParameters(index) -> DEBUG_BREAKPOINT_PARAMETERS\n"
     "  Returns the parameters for the breakpoint at the specified index."
    },
    {"ReadBugCheckData", 
     (PyCFunction)dbgcontrol_ReadBugCheckData, METH_NOARGS, 
     "ReadBugCheckData() -> (code, arg1, arg2, arg3, arg4)\n"
     "  Reads the kernel bug check code and related parameters."
    },
    {"GetCodeLevel", 
     (PyCFunction)dbgcontrol_GetCodeLevel, METH_NOARGS, 
     "GetCodeLevel() -> level\n"
     "  Returns the current code level."
    },
    {"SetCodeLevel", 
     (PyCFunction)dbgcontrol_SetCodeLevel, METH_VARARGS, 
     "SetCodeLevel(level)\n"
     "  Sets the current code level.\n"
     "  Level:\n"
     "    DEBUG_LEVEL_SOURCE\n"
     "    DEBUG_LEVEL_ASSEMBLY"
    },
    {"GetDebuggeeType", 
     (PyCFunction)dbgcontrol_GetDebuggeeType, METH_NOARGS, 
     "GetDebuggeeType() -> (class, qualifier)\n"
     "  Describes the nature of the current target.\n"
     "  Class:\n"
     "    DEBUG_CLASS_UNINITIALIZED\n"
     "    DEBUG_CLASS_KERNEL\n"
     "    DEBUG_CLASS_USER_WINDOWS\n"
     "  Qualifier:\n"
     "    DEBUG_KERNEL_CONNECTION\n"
     "    DEBUG_KERNEL_LOCAL\n"
     "    DEBUG_KERNEL_EXDI_DRIVER\n"
     "    DEBUG_KERNEL_SMALL_DUMP\n"
     "    DEBUG_KERNEL_DUMP\n"
     "    DEBUG_KERNEL_FULL_DUMP\n"
     "    DEBUG_USER_WINDOWS_PROCESS\n"
     "    DEBUG_USER_WINDOWS_PROCESS_SERVER\n"
     "    DEBUG_USER_WINDOWS_SMALL_DUMP\n"
     "    DEBUG_USER_WINDOWS_DUMP"
    },
    {"GetDisassembleEffectiveOffset", 
     (PyCFunction)dbgcontrol_GetDisassembleEffectiveOffset, METH_NOARGS, 
     "GetDisassembleEffectiveOffset() -> offset\n"
     "  Returns the address of the last instruction disassembled using Disassemble"
    },
    {"OutputDisassembly", 
     (PyCFunction)dbgcontrol_OutputDisassembly, METH_VARARGS, 
     "OutputDisassembly(offset,flags=[all]) -> end offset\n"
     "  Disassembles a processor instruction and sends the disassembly to output\n"
     "  Flags:\n"
     "    DEBUG_DISASM_EFFECTIVE_ADDRESS\n"
     "    DEBUG_DISASM_MATCHING_SYMBOLS\n"
     "    DEBUG_DISASM_SOURCE_LINE_NUMBER\n"
     "    DEBUG_DISASM_SOURCE_FILE_NAME"
    },
    {"OutputDisassemblyLines", 
     (PyCFunction)dbgcontrol_OutputDisassemblyLines, METH_VARARGS, 
     "OutputDisassemblyLines(PrevLines, TotalLines, Offset, Flags=[all]) -> " 
        " (OffsetLine, StartOffset, EndOffset)\n"
     "  Disassembles several process instructions to output\n" 
     "  Flags:\n"
     "    DEBUG_DISASM_EFFECTIVE_ADDRESS\n"
     "    DEBUG_DISASM_MATCHING_SYMBOLS\n"
     "    DEBUG_DISASM_SOURCE_LINE_NUMBER\n"
     "    DEBUG_DISASM_SOURCE_FILE_NAME"
    },
    {"GetDumpFormatFlags", 
     (PyCFunction)dbgcontrol_GetDumpFormatFlags, METH_NOARGS, 
     "GetDumpFormatFlags() -> flags\n"
     "  Returns the flags that describe what information is available in a\n"
     "  dump file target."
    },
    {"AddEngineOptions", 
     (PyCFunction)dbgcontrol_AddEngineOptions, METH_VARARGS, 
     "AddEngineOptions(Options)\n"
     "  Turns on some of the debugger engine's options.\n"
     "  Options:\n"
     "    DEBUG_ENGOPT_IGNORE_DBGHELP_VERSION\n"
     "    DEBUG_ENGOPT_IGNORE_EXTENSION_VERSIONS\n"
     "    DEBUG_ENGOPT_ALLOW_NETWORK_PATHS\n"
     "    DEBUG_ENGOPT_DISALLOW_NETWORK_PATHS\n"
     "    DEBUG_ENGOPT_IGNORE_LOADER_EXCEPTIONS\n"
     "    DEBUG_ENGOPT_INITIAL_BREAK\n"
     "    DEBUG_ENGOPT_INITIAL_MODULE_BREAK\n"
     "    DEBUG_ENGOPT_FINAL_BREAK\n"
     "    DEBUG_ENGOPT_NO_EXECUTE_REPEAT\n"
     "    DEBUG_ENGOPT_FAIL_INCOMPLETE_INFORMATION\n"
     "    DEBUG_ENGOPT_ALLOW_READ_ONLY_BREAKPOINTS\n"
     "    DEBUG_ENGOPT_SYNCHRONIZE_BREAKPOINTS\n"
     "    DEBUG_ENGOPT_DISALLOW_SHELL_COMMANDS\n"
     "    DEBUG_ENGOPT_KD_QUIET_MODE\n"
     "    DEBUG_ENGOPT_DISABLE_MANAGED_SUPPORT\n"
     "    DEBUG_ENGOPT_DISABLE_MODULE_SYMBOL_LOAD\n"
     "    DEBUG_ENGOPT_DISABLE_EXECUTION_COMMANDS"
    },
    {"GetEngineOptions", 
     (PyCFunction)dbgcontrol_GetEngineOptions, METH_NOARGS, 
     "GetEngineOptions() -> options\n"
     "  Returns the engine's options."
    },
    {"RemoveEngineOptions", 
     (PyCFunction)dbgcontrol_RemoveEngineOptions, METH_VARARGS, 
     "RemoveEngineOptions(options)\n"
     "  Turns off some of the engine's options."
    },
    {"SetEngineOptions", 
     (PyCFunction)dbgcontrol_SetEngineOptions, METH_VARARGS, 
     "SetEngineOptions(options)\n"
     "  Changes the engine's options."
    },
    {"GetSystemErrorControl", 
     (PyCFunction)dbgcontrol_GetSystemErrorControl, METH_NOARGS, 
     "GetSystemErrorControl() -> (Outputlevel, Breaklevel)\n"
     "  Returns the control values for handling system errors."
    },
    {"SetSystemErrorControl", 
     (PyCFunction)dbgcontrol_SetSystemErrorControl, METH_VARARGS, 
     "SetSystemErrorControl(OutputLevel, BreakLevel)\n"
     "  Sets the control values for handling system errors."
    },
    {"GetNotifyEventHandle", 
     (PyCFunction)dbgcontrol_GetNotifyEventHandle, METH_NOARGS, 
     "GetNotifyEventHandle() -> handle\n"
     "  Returns the handle of the event that will be signaled after the next\n"
     "  exception in a target."
    },
    {"SetNotifyEventHandle", 
     (PyCFunction)dbgcontrol_SetNotifyEventHandle, METH_VARARGS, 
     "SetNotifyEventHandle(handle)\n"
     "  Sets the event that will be signaled after the next exception in a target."
    },
    {"GetNumberEvents", 
     (PyCFunction)dbgcontrol_GetNumberEvents, METH_NOARGS, 
     "GetNumberEvents() -> number\n"
     "  Returns the number of events for the current target."
    },
    {"GetCurrentEventIndex", 
     (PyCFunction)dbgcontrol_GetCurrentEventIndex, METH_NOARGS, 
     "GetCurrentEventIndex() -> index\n"
     "  Returns the index of the current event within the current list of events\n"
     "  for the current target, if such a list exists."
    },
    {"SetNextEventIndex", 
     (PyCFunction)dbgcontrol_SetNextEventIndex, METH_VARARGS, 
     "SetNextEventIndex(relation, value) -> next index\n"
     "  Sets the next event for the current target by selecting the event from the\n"
     "  static list of events for the target, if such a list exists.\n"
     "  Relation:\n"
     "    DEBUG_EINDEX_FROM_START\n"
     "    DEBUG_EINDEX_FROM_END\n"
     "    DEBUG_EINDEX_FROM_CURRENT"
    },
    {"GetEventIndexDescription", 
     (PyCFunction)dbgcontrol_GetEventIndexDescription, METH_VARARGS, 
     "GetEventIndexDescription(index, which) -> description\n"
     "  Describes the specified event in a list of events for the current target."
    },
    {"GetExpressionSyntax", 
     (PyCFunction)dbgcontrol_GetExpressionSyntax, METH_NOARGS, 
     "GetExpressionSyntax() -> flags\n"
     "  Returns the current syntax that the engine is using for evaluting expressions.\n"
     "  Flags:\n"
     "    DEBUG_EXPR_MASM\n"
     "    DEBUG_EXPR_CPLUSPLUS"
    },
    {"SetExpressionSyntax", 
     (PyCFunction)dbgcontrol_SetExpressionSyntax, METH_VARARGS, 
     "SetExpressionSyntax(flags)\n"
     "  Sets the syntax that the engine will use to evaluate expressions.\n"
     "  Flags:\n"
     "    DEBUG_EXPR_MASM\n"
     "    DEBUG_EXPR_CPLUSPLUS"
    },
    {"GetExpressionSyntaxNames", 
     (PyCFunction)dbgcontrol_GetExpressionSyntaxNames, METH_VARARGS, 
     "GetExpressionSyntaxNames() -> NOT IMPLEMENTED"
    },
    {"SetExpressionSyntaxByName", 
     (PyCFunction)dbgcontrol_SetExpressionSyntaxByName, METH_VARARGS, 
     "SetExpressionSyntaxByName() -> NOT IMPLEMENTED"
    },
    {"GetNumberExpressionSyntaxes", 
     (PyCFunction)dbgcontrol_GetNumberExpressionSyntaxes, METH_NOARGS, 
     "GetNumberExpressionSyntaxes() -> NOT IMPLEMENTED"
    },
    {"AddExtension", 
     (PyCFunction)dbgcontrol_AddExtension, METH_VARARGS, 
     "AddExtension(path) -> handle\n"
     "  Loads an extension library into the debugger engine."
    },
    {"RemoveExtension", 
     (PyCFunction)dbgcontrol_RemoveExtension, METH_VARARGS, 
     "RemoveExtension(handle)\n"
     "  Unloads an extension library."
    },
    {"CallExtension", 
     (PyCFunction)dbgcontrol_CallExtension, METH_VARARGS, 
     "CallExtension(Handle, Function, Args='')\n"
     "  Call a debugger extension."
    },
    {"GetExtensionByPath", 
     (PyCFunction)dbgcontrol_GetExtensionByPath, METH_VARARGS, 
     "GetExtensionByPath(path) -> handle\n"
     "  Return the handle for an already loaded extension library."
    },
    {"GetExtensionFunction", 
     (PyCFunction)dbgcontrol_GetExtensionFunction, METH_VARARGS, 
     "GetExtensionFunction() -> NOT IMPLEMENTED"
    },
    {"GetWindbgExtensionApis64", 
     (PyCFunction)dbgcontrol_GetWindbgExtensionApis64, METH_NOARGS, 
     "GetWindbgExtensionApis64() -> NOT IMPLEMENTED"
    },
    {"WaitForEvent", 
     (PyCFunction)dbgcontrol_WaitForEvent, METH_VARARGS, 
     "WaitForEvent(timeout=INFINITE)\n"
     "  Waits for an event that breaks into the debugger engine application."
    },
// XXX - GetEventFilterCommand
// XXX - SetEventFilterCommand
// XXX - GetNumberEventFilters
// XXX - GetEventFilterText
// XXX - GetLastEventInformation
// XXX - GetStoredEventInformation
// XXX - GetExceptionFilterParameters
// XXX - SetExceptionFilterParameters
// XXX - GetExceptionFilterSecondCommand
// XXX - SetExceptionFilterSecondCommand
// XXX - GetSpecificFilterArgument
// XXX - SetSpecificFilterArgument
// XXX - GetSpecificFilterParameters
// XXX - SetSpecificFilterParameters
// XXX - Input
// XXX - ReturnInput
    {"GetNearInstruction", 
     (PyCFunction)dbgcontrol_GetNearInstruction, METH_VARARGS, 
     "GetNearInstruction(offset, delta) -> near offset\n"
     "  Returns the location of a processor instruction relative to a given location."
    },
    {"GetInterrupt", 
     (PyCFunction)dbgcontrol_GetInterrupt, METH_NOARGS, 
     "GetInterrupt()\n"
     "  Checks whether a user interrupt was issued."
    },
    {"SetInterrupt", 
     (PyCFunction)dbgcontrol_SetInterrupt, METH_VARARGS, 
     "SetInterrupt(flags)\n"
     "  Registers a user interrupt or breaks into the debugger.\n"
     "  Flags:\n"
     "    DEBUG_INTERRUPT_ACTIVE\n"
     "    DEBUG_INTERRUPT_PASSIVE\n"
     "    DEBUG_INTERRUPT_EXIT"
    },
    {"GetInterruptTimeout", 
     (PyCFunction)dbgcontrol_GetInterruptTimeout, METH_NOARGS, 
     "GetInterruptTimeout() -> seconds\n"
     "  Returns the number of seconds that the engine will wait when requesting a\n"
     "  break into the debugger."
    },
    {"SetInterruptTimeout", 
     (PyCFunction)dbgcontrol_SetInterruptTimeout, METH_VARARGS, 
     "SetInterruptTimeout(seconds)\n"
     "  Sets the number of seconds that the debugger engine should wait when\n"
     "  requesting a break into the debugger."
    },
    {"CloseLogFile", 
     (PyCFunction)dbgcontrol_CloseLogFile, METH_NOARGS, 
     "CloseLogFile()\n"
     "  Closes the currently open log file."
    },
    {"GetLogFile", 
     (PyCFunction)dbgcontrol_GetLogFile, METH_NOARGS, 
     "GetLogFile() -> log file name\n"
     "  Return the name of the currently open log file."
    },
// XXX - GetLogFile2
    {"OpenLogFile", 
     (PyCFunction)dbgcontrol_OpenLogFile, METH_VARARGS, 
     "OpenLogFile(file, append)\n"
     "  Open a log file that will receive output from the client objects."
    },
// XXX - OpenLogFile2 
    {"GetLogMask", 
     (PyCFunction)dbgcontrol_GetLogMask, METH_NOARGS, 
     "GetLogMask() -> mask\n"
     "  Returns the output mask for the currently open log file."
    },
    {"SetLogMask", 
     (PyCFunction)dbgcontrol_SetLogMask, METH_VARARGS, 
     "SetLogMask(mask)\n"
     "  Sets the output mask for the currently open log file.\n"
     "  Mask:\n"
     "    DEBUG_OUTPUT_NORMAL\n"
     "    DEBUG_OUTPUT_ERROR\n"
     "    DEBUG_OUTPUT_WARNING\n"
     "    DEBUG_OUTPUT_VERBOSE\n"
     "    DEBUG_OUTPUT_PROMPT\n"
     "    DEBUG_OUTPUT_PROMPT_REGISTERS\n"
     "    DEBUG_OUTPUT_EXTENSION_WARNING\n"
     "    DEBUG_OUTPUT_DEBUGGEE\n"
     "    DEBUG_OUTPUT_DEBUGGEE_PROMPT\n"
     "    DEBUG_OUTPUT_SYMBOLS"
    },
    {"GetReturnOffset", 
     (PyCFunction)dbgcontrol_GetReturnOffset, METH_NOARGS, 
     "GetReturnOffset() -> offset\n"
     "  Returns the return address for the current function."
    },
// XXX - Output
// XXX - ControlledOutput
// XXX - OutputVaList
// XXX - ControlledOutputVaList
    {"GetPageSize", 
     (PyCFunction)dbgcontrol_GetPageSize, METH_NOARGS, 
     "GetPageSize() -> size\n"
     "  Returns the page size for the effective processor mode."
    },
    {"IsPointer64Bit", 
     (PyCFunction)dbgcontrol_IsPointer64Bit, METH_NOARGS, 
     "IsPointer64Bit()\n"
     "  Determines if the effective processor uses 64-bit pointers."
    },
    {"GetActualProcessorType", 
     (PyCFunction)dbgcontrol_GetActualProcessorType, METH_NOARGS, 
     "GetActualProcessorType() -> type\n"
     "  Returns the processor type of the physical processor of the computer that\n"
     "  is running the target.\n"
     "  Type:\n"
     "    IMAGE_FILE_MACHINE_I386\n"
     "    IMAGE_FILE_MACHINE_ARM\n"
     "    IMAGE_FILE_MACHINE_IA64\n"
     "    IMAGE_FILE_MACHINE_AMD64\n"
     "    IMAGE_FILE_MACHINE_EBC"
    },
    {"GetEffectiveProcessorType", 
     (PyCFunction)dbgcontrol_GetEffectiveProcessorType, METH_NOARGS, 
     "GetEffectiveProcessorType() -> type\n"
     "  Returns the effective processor type of the processor of the computer that is\n"
     "  running the target.\n"
     "  Type:\n"
     "    IMAGE_FILE_MACHINE_I386\n"
     "    IMAGE_FILE_MACHINE_ARM\n"
     "    IMAGE_FILE_MACHINE_IA64\n"
     "    IMAGE_FILE_MACHINE_AMD64\n"
     "    IMAGE_FILE_MACHINE_EBC"
    },
    {"SetEffectiveProcessorType", 
     (PyCFunction)dbgcontrol_SetEffectiveProcessorType, METH_VARARGS, 
     "SetEffectiveProcessorType(type)\n"
     "  Sets the effective processor type of the processor of the computer that is\n"
     "  running the target.\n"
     "  Type:\n"
     "    IMAGE_FILE_MACHINE_I386\n"
     "    IMAGE_FILE_MACHINE_ARM\n"
     "    IMAGE_FILE_MACHINE_IA64\n"
     "    IMAGE_FILE_MACHINE_AMD64\n"
     "    IMAGE_FILE_MACHINE_EBC"
    },
    {"GetExecutingProcessorType", 
     (PyCFunction)dbgcontrol_GetExecutingProcessorType, METH_NOARGS, 
     "GetExecutingProcessorType() -> type\n"
     "  Returns the executing processor type for the processor for which the last\n"
     "  event occurred.\n"
     "  Type:\n"
     "    IMAGE_FILE_MACHINE_I386\n"
     "    IMAGE_FILE_MACHINE_ARM\n"
     "    IMAGE_FILE_MACHINE_IA64\n"
     "    IMAGE_FILE_MACHINE_AMD64\n"
     "    IMAGE_FILE_MACHINE_EBC"
    },
    {"GetNumberProcessors", 
     (PyCFunction)dbgcontrol_GetNumberProcessors, METH_NOARGS, 
     "GetNumberProcessors() -> number\n"
     "  Returns the number of processors on the computer running the current target."
    },
// XXX - GetPossibleExecutingProcessorTypes
// XXX - GetNumberPossibleExecutingProcessorTypes
// XXX - GetSupportedProcessorTypes
// XXX - GetNumberSuportedProcessorTypes
// XXX - GetProcessorTypeNames
// XXX - OutputPrompt
// XXX - OutputPromptVaList
// XXX - GetPromptText
    {"GetRadix", 
     (PyCFunction)dbgcontrol_GetRadix, METH_NOARGS, 
     "GetRadix() -> radix\n"
     "  Returns the default radix used by the debugger engine when it evaluates\n"
     "  and displays MASM expressions and when it displays symbol information."
    },
    {"SetRadix", 
     (PyCFunction)dbgcontrol_SetRadix, METH_VARARGS, 
     "SetRadix(radix)\n"
     "  Sets the default radix used by the debugger engine when it evaluates and\n"
     "  displays MASM expressions and when it displays symbol information.\n"
     "  Radix: 8, 10, or 16"
    },
    {"GetStackTrace", 
     (PyCFunction)dbgcontrol_GetStackTrace, METH_VARARGS, 
     "GetStackTrace(FrameOffset=0, StatckOffset=0, InstrOffset=0) -> [frame list]\n"
     "  Returns the frames at the top of the specified call stack."
    },
    {"OutputStackTrace", 
     (PyCFunction)dbgcontrol_OutputStackTrace, METH_VARARGS, 
     "OutputStackTrace(FrameSize, Flags)\n"
     "  Outputs either the supplied stack frame or the current stack frames\n"
     "  Flags:\n"
     "    DEBUG_STACK_ARGUMENTS\n"
     "    DEBUG_STACK_FUNCTION_INFO\n"
     "    DEBUG_STACK_SOURCE_LINE\n"
     "    DEBUG_STACK_FRAME_ADDRESSES\n"
     "    DEBUG_STACK_COLUMN_NAMES\n"
     "    DEBUG_STACK_NONVOLATILE_REGISTERS\n"
     "    DEBUG_STACK_FRAME_NUMBERS\n"
     "    DEBUG_STACK_PARAMETERS\n"
     "    DEBUG_STACK_FRAME_ADDRESSES_RA_ONLY\n"
     "    DEBUG_STACK_FRAME_MEMORY_USAGE\n"
     "    DEBUG_STACK_PARAMETERS_NEWLINE"
    },
// XXX - GetContextStackTrace
// XXX - OutputContextStackTrace
    {"OutputCurrentState", 
     (PyCFunction)dbgcontrol_OutputCurrentState, METH_VARARGS, 
     "OutputCurrentState(flags=DEBUG_CURRENT_DEFAULT)\n"
     "  Prints the current state of the current target to the debugger console.\n"
     "  Flags:\n"
     "    DEBUG_CURRENT_SYMBOL\n"
     "    DEBUG_CURRENT_DISASM\n"
     "    DEBUG_CURRENT_REGISTERS\n"
     "    DEBUG_CURRENT_SOURCE_LINE"
    },
    {"GetExecutionStatus", 
     (PyCFunction)dbgcontrol_GetExecutionStatus, METH_NOARGS, 
     "GetExecutionStatus() -> status\n"
     "  Returns information about the execution status of the debugger engine.\n"
     "  Status:\n"
     "    DEBUG_STATUS_NO_DEBUGGEE\n"
     "    DEBUG_STATUS_STEP_OVER\n"
     "    DEBUG_STATUS_STEP_INTO\n"
     "    DEBUG_STATUS_STEP_BRANCH\n"
     "    DEBUG_STATUS_GO\n"
     "    DEBUG_STATUS_BREAK"
    },
    {"SetExecutionStatus", 
     (PyCFunction)dbgcontrol_SetExecutionStatus, METH_VARARGS, 
     "SetExecutionStatus(status)\n"
     "  Requests that the debugger engine enter an executable state.  Actual\n"
     "  execution will no occur until the next time WaitForEvent is called.\n"
     "  Status:\n"
     "    DEBUG_STATUS_STEP_INTO\n"
     "    DEBUG_STATUS_STEP_BRANCH\n"
     "    DEBUG_STATUS_STEP_OVER\n"
     "    DEBUG_STATUS_GO_NOT_HANDLED\n"
     "    DEBUG_STATUS_GO_HANDLED\n"
     "    DEBUG_STATUS_GO"
    },
// XXX - GetSystemVersion
// XXX - GetSystemVersionString
// XXX - GetSystemVersionValues
// XXX - GetTextMacro
// XXX - SetTextMacro
// XXX - GetTextReplacement
// XXX - SetTextReplacement
// XXX - OutputTextReplacements
// XXX - RemoveTextReplacements
// XXX - GetNumberTextReplacements
// XXX - GetCurrentTimeDate
// XXX - GetCurrentSystemUpTime
// XXX - CoerceValue
// XXX - CoerceValues
    {"OutputVersionInformation", 
     (PyCFunction)dbgcontrol_OutputVersionInformation, METH_NOARGS, 
     "OutputVersionInformation()\n"
     "  Prints version information about the debugger engine to the console."
    },
    {NULL, NULL}
};


PyTypeObject PyDebugControlType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugControl",   /*tp_name*/
    sizeof(PyDebugControlObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgcontrol_dealloc, /*tp_dealloc*/
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
    "DebugControl objects",     /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugControlObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugControl_methods,     /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgcontrol_init,            /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgcontrol_new,             /* tp_new */
    PyObject_Del,              /* tp_free */
};

