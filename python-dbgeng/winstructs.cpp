#include "winstructs.h"

static int 
dbgse_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugSymbolEntryObject *self = (PyDebugSymbolEntryObject *)oself;
    static char *kwlist[] = {
        "ModuleBase",
        "Offset",
        "Id",
        "Arg64",
        "Size",
        "Flags",
        "TypeId",
        "NameSize",
        "Token",
        "Tag",
        "Arg32",
        NULL 
    };
    DEBUG_SYMBOL_ENTRY sym = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KKKKIIIIIII:DebugSymbolEntry", kwlist, 
                &sym.ModuleBase, &sym.Offset, &sym.Id, &sym.Arg64,
                &sym.Size, &sym.Flags, &sym.TypeId, &sym.NameSize,
                &sym.Token, &sym.Tag, &sym.Arg32))
        return -1;

    self->sym = sym;
    return 0;
}

static PyMemberDef PyDebugSymbolEntryObject_members[] = {
    {"ModuleBase", T_ULONGLONG, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, ModuleBase),
        0, "ModuleBase"},
    {"Offset", T_ULONGLONG, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Offset),
        0, "Offset"},
    {"Id", T_ULONGLONG, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Id),
        0, "Id"},
    {"Arg64", T_ULONGLONG, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Arg64),
        0, "Arg64"},
    {"Size", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Size),
        0, "Size"},
    {"Flags", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Flags),
        0, "Flags"},
    {"TypeId", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, TypeId),
        0, "TypeId"},
    {"NameSize", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, NameSize),
        0, "NameSize"},
    {"Token", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Token),
        0, "Token"},
    {"Tag", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Tag),
        0, "Tag"},
    {"Arg32", T_UINT, 
        offsetof(PyDebugSymbolEntryObject, sym) + 
        offsetof(DEBUG_SYMBOL_ENTRY, Arg32),
        0, "Arg32"},
    {NULL}
};

static PyMethodDef PyDebugSymbolEntryObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugSymbolEntryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSymbolEntry", /*tp_name*/
    sizeof(PyDebugSymbolEntryObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugSymbolEntry object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugSymbolEntryObject_methods, /* tp_methods */
    PyDebugSymbolEntryObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgse_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgrd_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugRegisterDescriptionObject *self = 
        (PyDebugRegisterDescriptionObject *)oself;
    static char *kwlist[] = {
        "Type",
        "Flags",
        "SubregMaster",
        "SubregLength",
        "SubregMask",
        "SubregShift",
        NULL 
    };
    DEBUG_REGISTER_DESCRIPTION reg = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIIIKI:DebugRegisterDescription", kwlist, 
                &reg.Type, &reg.Flags, &reg.SubregMaster, &reg.SubregLength,
                &reg.SubregMask, &reg.SubregShift))
        return -1;

    self->reg = reg;
    return 0;
}

static PyMemberDef PyDebugRegisterDescriptionObject_members[] = {
    {"Type", T_UINT, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, Type),
        0, "Type"},
    {"Flags", T_UINT, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, Flags),
        0, "Flags"},
    {"SubregMaster", T_UINT, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, SubregMaster),
        0, "SubregMaster"},
    {"SubregLength", T_UINT, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, SubregLength),
        0, "SubregLength"},
    {"SubregMask", T_ULONGLONG, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, SubregMask),
        0, "SubregMask"},
    {"SubregShift", T_UINT, 
        offsetof(PyDebugRegisterDescriptionObject, reg) + 
        offsetof(DEBUG_REGISTER_DESCRIPTION, SubregShift),
        0, "SubregShift"},
    {NULL}
};

static PyMethodDef PyDebugRegisterDescriptionObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugRegisterDescriptionType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugRegiseterDescription", /*tp_name*/
    sizeof(PyDebugRegisterDescriptionObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugRegisterDescription object",  /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugRegisterDescriptionObject_methods, /* tp_methods */
    PyDebugRegisterDescriptionObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgrd_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgsf_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugStackFrameObject *self = (PyDebugStackFrameObject *)oself;
    static char *kwlist[] = {
        "InstructionOffset",
        "ReturnOffset",
        "FrameOffset",
        "StackOffset",
        "FuncTableEntry",
        "Param1",
        "Param2",
        "Param3",
        "Param4",
        "Virtual",
        "FrameNumber",
        NULL 
    };
    DEBUG_STACK_FRAME frame = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KKKKKKKKKiI:DebugStackFrame", kwlist, 
                &frame.InstructionOffset, &frame.ReturnOffset,
                &frame.FrameOffset, &frame.StackOffset,
                &frame.FuncTableEntry, &frame.Params[0],
                &frame.Params[1], &frame.Params[2], &frame.Params[3],
                &frame.Virtual, &frame.FrameNumber))
        return -1;

    self->frame = frame;
    return 0;
}

static PyMemberDef PyDebugStackFrameObject_members[] = {
    {"InstructionOffset", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, InstructionOffset),
        0, "InstructionOffset"},
    {"ReturnOffset", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, ReturnOffset),
        0, "ReturnOffset"},
    {"FrameOffset", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, FrameOffset),
        0, "FrameOffset"},
    {"StackOffset", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, StackOffset),
        0, "StackOffset"},
    {"FuncTableEntry", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, FuncTableEntry),
        0, "FuncTableEntry"},
    {"Param1", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, Params) + (0 * sizeof(ULONG64)),
        0, "Param1"},
    {"Param2", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, Params) + (1 * sizeof(ULONG64)),
        0, "Param2"},
    {"Param3", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, Params) + (2 * sizeof(ULONG64)),
        0, "Param3"},
    {"Param4", T_ULONGLONG, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, Params) + (3 * sizeof(ULONG64)),
        0, "Param4"},
    {"Virtual", T_INT, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, Virtual),
        0, "Virtual"},
    {"FrameNumber", T_UINT, 
        offsetof(PyDebugStackFrameObject, frame) + 
        offsetof(DEBUG_STACK_FRAME, FrameNumber),
        0, "FrameNumber"},
    {NULL}
};

static PyMethodDef PyDebugStackFrameObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugStackFrameType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugStackFrame", /*tp_name*/
    sizeof(PyDebugStackFrameObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugStackFrame object",  /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugStackFrameObject_methods, /* tp_methods */
    PyDebugStackFrameObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsf_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgdmp_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugModuleParametersObject *self = (PyDebugModuleParametersObject *)oself;
    static char *kwlist[] = {
        "Base",
        "Size",
        "TimeDateStamp",
        "Checksum",
        "Flags",
        "SymbolType",
        "ImageNameSize",
        "ModuleNameSize",
        "LoadedImageNameSize",
        "SymbolFileNameSize",
        "MappedImageNameSize",
        NULL 
    };
    DEBUG_MODULE_PARAMETERS p = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KIIIIIIIIII:DebugModuleParameters", kwlist, 
                &p.Base, &p.Size, &p.TimeDateStamp, &p.Checksum, &p.Flags,
                &p.SymbolType, &p.ImageNameSize, &p.ModuleNameSize, 
                &p.LoadedImageNameSize, &p.SymbolFileNameSize, 
                &p.MappedImageNameSize))
        return -1;

    self->dmp = p;
    return 0;
}

static PyMemberDef PyDebugModuleParametersObject_members[] = {
    {"Base", T_ULONGLONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, Base),
        0, "Base"},
    {"Size", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, Size),
        0, "Size"},
    {"TimeDateStamp", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, TimeDateStamp),
        0, "TimeDateStamp"},
    {"Checksum", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, Checksum),
        0, "Checksum"},
    {"Flags", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, Flags),
        0, "Flags"},
    {"SymbolType", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, SymbolType),
        0, "SymbolType"},
    {"ImageNameSize", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, ImageNameSize),
        0, "ImageNameSize"},
    {"ModuleNameSize", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, ModuleNameSize),
        0, "ModuleNameSize"},
    {"LoadedImageNameSize", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, LoadedImageNameSize),
        0, "LoadedImageNameSize"},
    {"SymbolFileNameSize", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, SymbolFileNameSize),
        0, "SymbolFileNameSize"},
    {"MappedImageNameSize", T_ULONG, 
        offsetof(PyDebugModuleParametersObject, dmp) +
        offsetof(DEBUG_MODULE_PARAMETERS, MappedImageNameSize),
        0, "MappedImageNameSize"},
    {NULL}
};

static PyMethodDef PyDebugModuleParametersObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugModuleParametersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugModuleParameters", /*tp_name*/
    sizeof(PyDebugModuleParametersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugModuleParameters object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugModuleParametersObject_methods, /* tp_methods */
    PyDebugModuleParametersObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgdmp_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgparams_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugBreakPointParametersObject *self = (PyDebugBreakPointParametersObject*)oself;
    static char *kwlist[] = {
        "Offset", 
        "Id", 
        "BreakType", 
        "ProcType",
        "Flags",
        "DataSize",
        "DataAccessType",
        "PassCount",
        "CurrentPassCount",
        "MatchThread",
        "CommandSize",
        "OffsetExpressionSize",
        NULL 
    };
    DEBUG_BREAKPOINT_PARAMETERS p = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KIIIIIIIIIII:BreakPointParameters", kwlist, 
                &p.Offset, &p.Id, &p.BreakType, &p.ProcType, &p.Flags, &p.DataSize,
                &p.DataAccessType, &p.PassCount, &p.CurrentPassCount, &p.MatchThread,
                &p.CommandSize, &p.OffsetExpressionSize))
        return -1;

    self->bpp = p;
    return 0;
}

static PyMemberDef PyDebugBreakPointParametersObject_members[] = {
    {"Offset", T_ULONGLONG, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, Offset),
        0, "Offset"},
    {"Id", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, Id),
        0, "Id"},
    {"BreakType", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, BreakType),
        0, "BreakType"},
    {"ProcType", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, ProcType),
        0, "ProcType"},
    {"Flags", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, Flags),
        0, "Flags"},
    {"DataSize", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, DataSize),
        0, "DataSize"},
    {"DataAccessType", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, DataAccessType),
        0, "DataAccessType"},
    {"PassCount", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, PassCount),
        0, "PassCount"},
    {"CurrentPassCount", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, CurrentPassCount),
        0, "CurrentPassCount"},
    {"MatchThread", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, MatchThread),
        0, "MatchThread"},
    {"CommandSize", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, CommandSize),
        0, "CommandSize"},
    {"OffsetExpressionSize", T_UINT, 
        offsetof(PyDebugBreakPointParametersObject, bpp) + 
        offsetof(DEBUG_BREAKPOINT_PARAMETERS, OffsetExpressionSize),
        0, "OffsetExpressionSize"},
    {NULL}
};

static PyMethodDef PyDebugBreakPointParametersObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugBreakPointParametersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.BreakPointParameters", /*tp_name*/
    sizeof(PyDebugBreakPointParametersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "BreakPointParameters object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugBreakPointParametersObject_methods, /* tp_methods */
    PyDebugBreakPointParametersObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgparams_init,            /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgdmai_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyDebugModuleAndIdObject *self = (PyDebugModuleAndIdObject*)oself;
    static char *kwlist[] = {
        "ModuleBase", 
        "Id", 
        NULL 
    };
    ULONG64 ModuleBase = 0;
    ULONG64 Id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|KK:DebugModuleAndId",
                kwlist, &ModuleBase, &Id))
        return -1;

    self->ModuleBase = ModuleBase;
    self->Id = Id;
    return 0;
}

static PyMemberDef PyDebugModuleAndIdObject_members[] = {
    {"ModuleBase", T_ULONGLONG, offsetof(PyDebugModuleAndIdObject, ModuleBase),
        0, "ModuleBase"},
    {"Id", T_ULONGLONG, offsetof(PyDebugModuleAndIdObject, Id),
        0, "Id"},
    {NULL}
};

static PyMethodDef PyDebugModuleAndIdObject_methods[] = {
    {NULL}
};

PyTypeObject PyDebugModuleAndIdType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugModuleAndId", /*tp_name*/
    sizeof(PyDebugModuleAndIdObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugModuleAndId object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugModuleAndIdObject_methods, /* tp_methods */
    PyDebugModuleAndIdObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgdmai_init,            /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgexr_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyExceptionRecordObject *self = (PyExceptionRecordObject *)oself;
    static char *kwlist[] = {
        "ExceptionCode", 
        "ExceptionFlags", 
        "ExceptionRecord",
        "ExceptionAddress",
        "NumberParameters",
        // XXX -  "ExceptionInformation",
        NULL 
    };
    EXCEPTION_RECORD64 e = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIKKI:EXCEPTION_RECORD", kwlist, 
                &e.ExceptionCode, &e.ExceptionFlags, &e.ExceptionRecord,
                &e.ExceptionAddress, &e.NumberParameters))
        return -1;

    self->exr = e;
    return 0;
}

static PyObject *
dbgexr_str(PyObject *oself)
{
    PyExceptionRecordObject *self = (PyExceptionRecordObject *)oself;
    return PyString_FromFormat("(ExceptionCode=%x | ExceptionFlags=%x |"
                               " ExceptionRecord=%x | ExceptionAddress=%p |"
                               " NumberParameters=%d)",
                               self->exr.ExceptionCode, self->exr.ExceptionFlags,
                               self->exr.ExceptionRecord, self->exr.ExceptionAddress,
                               self->exr.NumberParameters);
}

static PyMemberDef PyExceptionRecordObject_members[] = {
    {"ExceptionCode", T_UINT, 
        offsetof(PyExceptionRecordObject, exr) +
        offsetof(EXCEPTION_RECORD64, ExceptionCode),
        0, "ExceptionCode"},
    {"ExceptionFlags", T_UINT, 
        offsetof(PyExceptionRecordObject, exr) +
        offsetof(EXCEPTION_RECORD64, ExceptionFlags),
        0, "ExceptionFlags"},
    {"ExceptionRecord", T_ULONGLONG, 
        offsetof(PyExceptionRecordObject, exr) +
        offsetof(EXCEPTION_RECORD64, ExceptionRecord),
        0, "ExceptionRecord"},
    {"ExceptionAddress", T_ULONGLONG,
        offsetof(PyExceptionRecordObject, exr) +
        offsetof(EXCEPTION_RECORD64, ExceptionAddress),
        0, "ExceptionAddress"},
    {"NumberParameters", T_UINT, 
        offsetof(PyExceptionRecordObject, exr) +
        offsetof(EXCEPTION_RECORD64, NumberParameters),
        0, "NumberParameters"},
    {NULL}
};

static PyMethodDef PyExceptionRecordObject_methods[] = {
    {NULL}
};

PyTypeObject PyExceptionRecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.ExceptionRecord", /*tp_name*/
    sizeof(PyExceptionRecordObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    dbgexr_str,                /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "ExceptionRecord64 object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyExceptionRecordObject_methods, /* tp_methods */
    PyExceptionRecordObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgexr_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgctx64_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {
        "SegCs", "SegDs", "SegEs", "SegFs", "SegGs", "SegSs",
        "EFlags",
        "Dr0", "Dr1", "Dr2", "Dr3", "Dr6", "Dr7",
        "Rax", "Rcx", "Rdx", "Rbx", "Rsp", "Rbp", "Rsi", "Rdi",
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", 
        "Rip",
        NULL 
    };
    PyContext64Object *self = (PyContext64Object *)oself;
    CONTEXT64 ctx = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|HHHHHHKKKKKKKKKKKKKKKKKKKKKKKK:Context64", kwlist,
                &ctx.SegCs, &ctx.SegDs, &ctx.SegEs, &ctx.SegFs,
                &ctx.SegGs, &ctx.SegSs, &ctx.EFlags, &ctx.Dr0,
                &ctx.Dr1, &ctx.Dr2, &ctx.Dr3, &ctx.Dr6, &ctx.Dr7,
                &ctx.Rax, &ctx.Rcx, &ctx.Rdx, &ctx.Rbx, &ctx.Rsp,
                &ctx.Rbp, &ctx.Rsi, &ctx.Rdi, &ctx.R8, &ctx.R9,
                &ctx.R10, &ctx.R11, &ctx.R12, &ctx.R13, &ctx.R14,
                &ctx.R15, &ctx.Rip))
        return -1;

    self->ctx = ctx;
    return 0;
}

static PyMemberDef PyContext64Object_members[] = {
    {"SegCs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegCs),
        0, "SegCs"},
    {"SegDs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegDs),
        0, "SegDs"},
    {"SegEs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegEs),
        0, "SegEs"},
    {"SegFs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegFs),
        0, "SegFs"},
    {"SegGs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegGs),
        0, "SegGs"},
    {"SegSs", T_SHORT, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, SegSs),
        0, "SegSs"},
    {"EFlags", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, EFlags),
        0, "EFlags"},
    {"Dr0", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr0),
        0, "Dr0"},
    {"Dr1", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr1),
        0, "Dr1"},
    {"Dr2", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr2),
        0, "Rip"},
    {"Dr3", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr3),
        0, "Dr3"},
    {"Dr6", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr6),
        0, "Dr6"},
    {"Dr7", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Dr7),
        0, "Dr7"},
    {"Rax", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rax),
        0, "Rax"},
    {"Rcx", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rcx),
        0, "Rcx"},
    {"Rdx", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rdx),
        0, "Rdx"},
    {"Rbx", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rbx),
        0, "Rbx"},
    {"Rsp", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rsp),
        0, "Rsp"},
    {"Rbp", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rbp),
        0, "Rbp"},
    {"Rsi", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rsi),
        0, "Rsi"},
    {"Rdi", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rdi),
        0, "Rdi"},
    {"R8", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R8),
        0, "R8"},
    {"R9", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R9),
        0, "R9"},
    {"R10", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R10),
        0, "R10"},
    {"R11", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R11),
        0, "R11"},
    {"R12", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R12),
        0, "R12"},
    {"R13", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R13),
        0, "R13"},
    {"R14", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R14),
        0, "R14"},
    {"R15", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, R15),
        0, "R15"},
    {"Rip", T_ULONGLONG, 
        offsetof(PyContext64Object, ctx) +
        offsetof(CONTEXT64, Rip),
        0, "Rip"},
    {NULL}
};

static PyMethodDef PyContext64Object_methods[] = {
    {NULL}
};

PyTypeObject PyContext64Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.Context64",      /*tp_name*/
    sizeof(PyContext64Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Context64 object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyContext64Object_methods, /* tp_methods */
    PyContext64Object_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgctx64_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgctx32_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyContext32Object *self = (PyContext32Object *)oself;
    static char *kwlist[] = {
        "SegCs", "SegDs", "SegEs", "SegFs", "SegGs", "SegSs",
        "EFlags", "Dr0", "Dr1", "Dr2", "Dr3", "Dr6", "Dr7",
        "Edi", "Esi", "Ebx", "Edx", "Ecx", "Eax", "Ebp", "Eip", "Esp",
        NULL 
    };

    CONTEXT32 ctx = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|HHHHHHIIIIIIIIIIIIIIII:Context32", kwlist, 
                &ctx.SegCs, &ctx.SegDs, &ctx.SegEs, &ctx.SegFs, &ctx.SegGs,
                &ctx.SegSs, &ctx.EFlags, &ctx.Dr0, &ctx.Dr1, &ctx.Dr2,
                &ctx.Dr3, &ctx.Dr6, &ctx.Dr7, &ctx.Edi, &ctx.Esi, &ctx.Ebx,
                &ctx.Edx, &ctx.Ecx, &ctx.Eax, &ctx.Ebp, &ctx.Eip, &ctx.Esp))
        return -1;

    self->ctx = ctx;
    return 0;
}

static PyMemberDef PyContext32Object_members[] = {
    {"SegCs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegCs),
        0, "SegCs"},
    {"SegDs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegDs),
        0, "SegDs"},
    {"SegEs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegEs),
        0, "SegEs"},
    {"SegFs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegFs),
        0, "SegFs"},
    {"SegGs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegGs),
        0, "SegGs"},
    {"SegSs", T_SHORT, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, SegSs),
        0, "SegSs"},
    {"EFlags", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, EFlags),
        0, "EFlags"},
    {"Dr0", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr0),
        0, "Dr0"},
    {"Dr1", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr1),
        0, "Dr1"},
    {"Dr2", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr2),
        0, "Rip"},
    {"Dr3", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr3),
        0, "Dr3"},
    {"Dr6", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr6),
        0, "Dr6"},
    {"Dr7", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Dr7),
        0, "Dr7"},
    {"Edi", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Edi),
        0, "Edi"},
    {"Esi", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Esi),
        0, "Esi"},
    {"Ebx", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Ebx),
        0, "Ebx"},
    {"Edx", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Edx),
        0, "Edx"},
    {"Ecx", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Ecx),
        0, "Ecx"},
    {"Eax", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Eax),
        0, "Eax"},
    {"Ebp", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Ebp),
        0, "Ebp"},
    {"Eip", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Eip),
        0, "Eip"},
    {"Esp", T_ULONG, 
        offsetof(PyContext32Object, ctx) +
        offsetof(CONTEXT32, Esp),
        0, "Esp"},
    {NULL}
};

static PyMethodDef PyContext32Object_methods[] = {
    {NULL}
};

PyTypeObject PyContext32Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.Context32",      /*tp_name*/
    sizeof(PyContext32Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Context32 object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyContext32Object_methods, /* tp_methods */
    PyContext32Object_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgctx32_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgcpo_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyCreateProcessOptionsObject *self = (PyCreateProcessOptionsObject *)oself;
    static char *kwlist[] = {
        "CreateFlags",
        "EngCreateFlags",
        "VerifierFlags",
        NULL 
    };

    DEBUG_CREATE_PROCESS_OPTIONS cpo = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|III:DEBUG_CREATE_PROCESS_OPTIONS", kwlist, 
                &cpo.CreateFlags, &cpo.EngCreateFlags, &cpo.VerifierFlags))
        return -1;

    self->cpo = cpo;
    return 0;
}

static PyMemberDef PyCreateProcessOptionsObject_members[] = {
    {"CreateFlags", T_ULONG, 
        offsetof(PyCreateProcessOptionsObject, cpo) +
        offsetof(DEBUG_CREATE_PROCESS_OPTIONS, CreateFlags),
        0, "CreateFlags"},
    {"EngCreateFlags", T_ULONG, 
        offsetof(PyCreateProcessOptionsObject, cpo) +
        offsetof(DEBUG_CREATE_PROCESS_OPTIONS, EngCreateFlags),
        0, "EngCreateFlags"},
    {"VerifierFlags", T_ULONG, 
        offsetof(PyCreateProcessOptionsObject, cpo) +
        offsetof(DEBUG_CREATE_PROCESS_OPTIONS, VerifierFlags),
        0, "VerifierFlags"},
    {NULL}
};

static PyMethodDef PyCreateProcessOptionsObject_methods[] = {
    {NULL}
};

PyTypeObject PyCreateProcessOptionsType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugCreateProcessOptions",      /*tp_name*/
    sizeof(PyCreateProcessOptionsObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugCreateProcessOptions object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyCreateProcessOptionsObject_methods, /* tp_methods */
    PyCreateProcessOptionsObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgcpo_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgefp_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyExceptionFilterParametersObject *self = 
        (PyExceptionFilterParametersObject *)oself;
    static char *kwlist[] = {
        "ExecutionOption",
        "ContinueOption",
        "TextSize",
        "CommandSize",
        "SecondCommandSize",
        "ExceptionCode",
        NULL 
    };

    DEBUG_EXCEPTION_FILTER_PARAMETERS efp = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIIIII:DebugExceptionFilterParameters", kwlist, 
                &efp.ExecutionOption, &efp.ContinueOption,
                &efp.TextSize, &efp.CommandSize,
                &efp.SecondCommandSize, &efp.ExceptionCode))
        return -1;

    self->efp = efp;
    return 0;
}

static PyMemberDef PyExceptionFilterParametersObject_members[] = {
    {"ExecutionOption", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, ExecutionOption),
        0, "ExecutionOption"},
    {"ContinueOption", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, ContinueOption),
        0, "ContinueOption"},
    {"TextSize", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, TextSize),
        0, "TextSize"},
    {"CommandSize", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, CommandSize),
        0, "CommandSize"},
    {"SecondCommandSize", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, SecondCommandSize),
        0, "SecondCommandSize"},
    {"ExceptionCode", T_ULONG, 
        offsetof(PyExceptionFilterParametersObject, efp) +
        offsetof(DEBUG_EXCEPTION_FILTER_PARAMETERS, ExceptionCode),
        0, "ExceptionCode"},
    {NULL}
};

static PyMethodDef PyExceptionFilterParametersObject_methods[] = {
    {NULL}
};

PyTypeObject PyExceptionFilterParametersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugExceptionFilterParameters",      /*tp_name*/
    sizeof(PyExceptionFilterParametersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugExceptionFilterParameters object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyExceptionFilterParametersObject_methods, /* tp_methods */
    PyExceptionFilterParametersObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgefp_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbghdb_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyHandleDataBasicObject *self = (PyHandleDataBasicObject *)oself;
    static char *kwlist[] = {
        "TypeNameSize",
        "ObjectNameSize",
        "Attributes",
        "GrantedAccess",
        "HandleCount",
        "PointerCount",
        NULL 
    };

    DEBUG_HANDLE_DATA_BASIC hdb = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIIIII:DebugHandleDataBasic", kwlist, 
                &hdb.TypeNameSize, &hdb.ObjectNameSize,
                &hdb.Attributes, &hdb.GrantedAccess,
                &hdb.HandleCount, &hdb.PointerCount))
        return -1;

    self->hdb = hdb;
    return 0;
}

static PyMemberDef PyHandleDataBasicObject_members[] = {
    {"TypeNameSize", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, TypeNameSize),
        0, "TypeNameSize"},
    {"ObjectNameSize", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, ObjectNameSize),
        0, "ObjectNameSize"},
    {"Attributes", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, Attributes),
        0, "Attributes"},
    {"GrantedAccess", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, GrantedAccess),
        0, "GrantedAccess"},
    {"HandleCount", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, HandleCount),
        0, "HandleCount"},
    {"PointerCount", T_ULONG, 
        offsetof(PyHandleDataBasicObject, hdb) +
        offsetof(DEBUG_HANDLE_DATA_BASIC, PointerCount),
        0, "PointerCount"},
    {NULL}
};

static PyMethodDef PyHandleDataBasicObject_methods[] = {
    {NULL}
};

PyTypeObject PyHandleDataBasicType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugHandleDataBasic",      /*tp_name*/
    sizeof(PyHandleDataBasicObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugHandleDataBasic object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyHandleDataBasicObject_methods, /* tp_methods */
    PyHandleDataBasicObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbghdb_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgsfp_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PySpecificFilterParametersObject *self = 
        (PySpecificFilterParametersObject *)oself;
    static char *kwlist[] = {
        "ExecutionOption",
        "ContinueOption",
        "TextSize",
        "CommandSize",
        "ArgumentSize",
        NULL 
    };

    DEBUG_SPECIFIC_FILTER_PARAMETERS sfp = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIIII:DebugSpecificFilterparameters", kwlist, 
                &sfp.ExecutionOption, &sfp.ContinueOption,
                &sfp.TextSize, &sfp.CommandSize,
                &sfp.ArgumentSize))
        return -1;

    self->sfp = sfp;
    return 0;
}

static PyMemberDef PySpecificFilterParametersObject_members[] = {
    {"ExecutionOption", T_ULONG, 
        offsetof(PySpecificFilterParametersObject, sfp) +
        offsetof(DEBUG_SPECIFIC_FILTER_PARAMETERS, ExecutionOption),
        0, "ExecutionOption"},
    {"ContinueOption", T_ULONG, 
        offsetof(PySpecificFilterParametersObject, sfp) +
        offsetof(DEBUG_SPECIFIC_FILTER_PARAMETERS, ContinueOption),
        0, "ContinueOption"},
    {"TextSize", T_ULONG, 
        offsetof(PySpecificFilterParametersObject, sfp) +
        offsetof(DEBUG_SPECIFIC_FILTER_PARAMETERS, TextSize),
        0, "TextSize"},
    {"CommandSize", T_ULONG, 
        offsetof(PySpecificFilterParametersObject, sfp) +
        offsetof(DEBUG_SPECIFIC_FILTER_PARAMETERS, CommandSize),
        0, "CommandSize"},
    {"ArgumentSize", T_ULONG, 
        offsetof(PySpecificFilterParametersObject, sfp) +
        offsetof(DEBUG_SPECIFIC_FILTER_PARAMETERS, ArgumentSize),
        0, "ArgumentSize"},
    {NULL}
};

static PyMethodDef PySpecificFilterParametersObject_methods[] = {
    {NULL}
};

PyTypeObject PySpecificFilterParametersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSpecificFilterparameters",      /*tp_name*/
    sizeof(PySpecificFilterParametersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugSpecificFilterparameters object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PySpecificFilterParametersObject_methods, /* tp_methods */
    PySpecificFilterParametersObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsfp_init,               /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgsp_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PySymbolParametersObject *self = (PySymbolParametersObject *)oself;
    static char *kwlist[] = {
        "Module",
        "TypeId",
        "ParentSymbol",
        "SubElements",
        "Flags",
        NULL 
    };

    DEBUG_SYMBOL_PARAMETERS sp = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KIIII:DebugSymbolParameters", kwlist, 
                &sp.Module, &sp.TypeId,
                &sp.ParentSymbol, &sp.SubElements,
                &sp.Flags))
        return -1;

    self->sp = sp;
    return 0;
}

static PyMemberDef PySymbolParametersObject_members[] = {
    {"Module", T_ULONGLONG, 
        offsetof(PySymbolParametersObject, sp) +
        offsetof(DEBUG_SYMBOL_PARAMETERS, Module),
        0, "Module"},
    {"TypeId", T_ULONG,
        offsetof(PySymbolParametersObject, sp) +
        offsetof(DEBUG_SYMBOL_PARAMETERS, TypeId),
        0, "TypeId"},
    {"ParentSymbol", T_ULONG,
        offsetof(PySymbolParametersObject, sp) +
        offsetof(DEBUG_SYMBOL_PARAMETERS, ParentSymbol),
        0, "ParentSymbol"},
    {"SubElements", T_ULONG,
        offsetof(PySymbolParametersObject, sp) +
        offsetof(DEBUG_SYMBOL_PARAMETERS, SubElements),
        0, "SubElements"},
    {"Flags", T_ULONG,
        offsetof(PySymbolParametersObject, sp) +
        offsetof(DEBUG_SYMBOL_PARAMETERS, Flags),
        0, "Flags"},
    {NULL}
};

static PyMethodDef PySymbolParametersObject_methods[] = {
    {NULL}
};

PyTypeObject PySymbolParametersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSymbolParameters",      /*tp_name*/
    sizeof(PySymbolParametersObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugSymbolParameters object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PySymbolParametersObject_methods, /* tp_methods */
    PySymbolParametersObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsp_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgsse_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PySymbolSourceEntryObject *self = (PySymbolSourceEntryObject *)oself;
    static char *kwlist[] = {
        "ModuleBase",
        "Offset",
        "FileNameId",
        "Size",
        "Flags",
        "FileNameSize",
        "StartLine",
        "EndLine",
        "StartColumn",
        "EndColumn",
        NULL 
    };

    DEBUG_SYMBOL_SOURCE_ENTRY sse = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KKKIIIIIII:DebugSymbolSourceEntry", kwlist, 
                &sse.ModuleBase, &sse.Offset, &sse.FileNameId,
                &sse.Size, &sse.Flags, &sse.FileNameSize,
                &sse.StartLine, &sse.EndLine, &sse.StartColumn,
                &sse.EndColumn))
        return -1;

    self->sse = sse;
    return 0;
}

static PyMemberDef PySymbolSourceEntryObject_members[] = {
    {"ModuleBase", T_ULONGLONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, ModuleBase),
        0, "ModuleBase"},
    {"Offset", T_ULONGLONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, Offset),
        0, "Offset"},
    {"FileNameId", T_ULONGLONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, FileNameId),
        0, "FileNameId"},
    {"Size", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, Size),
        0, "Size"},
    {"Flags", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, Flags),
        0, "Flags"},
    {"FileNameSize", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, FileNameSize),
        0, "FileNameSize"},
    {"StartLine", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, StartLine),
        0, "StartLine"},
    {"EndLine", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, EndLine),
        0, "EndLine"},
    {"StartColumn", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, StartColumn),
        0, "StartColumn"},
    {"EndColumn", T_ULONG, 
        offsetof(PySymbolSourceEntryObject, sse) +
        offsetof(DEBUG_SYMBOL_SOURCE_ENTRY, EndColumn),
        0, "EndColumn"},
    {NULL}
};

static PyMethodDef PySymbolSourceEntryObject_methods[] = {
    {NULL}
};

PyTypeObject PySymbolSourceEntryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugSymbolSourceEntry",      /*tp_name*/
    sizeof(PySymbolSourceEntryObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugSymbolSourceEntry object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PySymbolSourceEntryObject_methods, /* tp_methods */
    PySymbolSourceEntryObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgsse_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static int 
dbgtbi_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyThreadBasicInformationObject *self = 
        (PyThreadBasicInformationObject *)oself;
    static char *kwlist[] = {
        "Valid",
        "ExitStatus",
        "PriorityClass",
        "Priority",
        "CreateTime",
        "ExitTime",
        "KernelTime",
        "UserTime",
        "StartOffset",
        "Affinity",
        NULL 
    };

    DEBUG_THREAD_BASIC_INFORMATION tbi = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|IIIIKKKKKK:DebugThreadBasicInformation", kwlist, 
                &tbi.Valid, &tbi.ExitStatus, &tbi.PriorityClass,
                &tbi.Priority, &tbi.CreateTime, &tbi.ExitTime,
                &tbi.KernelTime, &tbi.UserTime, &tbi.StartOffset,
                &tbi.Affinity))
        return -1;

    self->tbi = tbi;
    return 0;
}

static PyMemberDef PyThreadBasicInformationObject_members[] = {
    {"Valid", T_ULONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, Valid),
        0, "Valid"},
    {"ExitStatus", T_ULONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, ExitStatus),
        0, "ExitStatus"},
    {"PriorityClass", T_ULONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, PriorityClass),
        0, "PriorityClass"},
    {"Priority", T_ULONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, Priority),
        0, "Priority"},
    {"CreateTime", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, CreateTime),
        0, "CreateTime"},
    {"ExitTime", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, ExitTime),
        0, "ExitTime"},
    {"KernelTime", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, KernelTime),
        0, "KernelTime"},
    {"UserTime", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, UserTime),
        0, "UserTime"},
    {"StartOffset", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, StartOffset),
        0, "StartOffset"},
    {"Affinity", T_ULONGLONG, 
        offsetof(PyThreadBasicInformationObject, tbi) +
        offsetof(DEBUG_THREAD_BASIC_INFORMATION, Affinity),
        0, "Affinity"},
    {NULL}
};

static PyMethodDef PyThreadBasicInformationObject_methods[] = {
    {NULL}
};

PyTypeObject PyThreadBasicInformationType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugThreadBasicInformation",      /*tp_name*/
    sizeof(PyThreadBasicInformationObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugThreadBasicInformation object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyThreadBasicInformationObject_methods, /* tp_methods */
    PyThreadBasicInformationObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgtbi_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

#if 0
static int 
dbgtd_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    PyTypedDataObject *self = (PyTypedDataObject *)oself;
    static char *kwlist[] = {
        "ModBase",
        "Offset",
        "Data",
        "Size",
        "Flags",
        "TypeId",
        "BaseTypeId",
        "Tag",
        "Register",
        NULL 
    };

    DEBUG_TYPED_DATA td = {0};

    if (!PyArg_ParseTupleAndKeywords(args, 
                kwds, "|KKKIIIIII:DebugTypedData", kwlist, 
                &td.ModBase, &td.Offset, &td.Data,
                &td.Size, &td.Flags, &td.TypeId,
                &td.BaseTypeId, &td.Tag, &td.Register))
        return -1;

    self->td = td;
    return 0;
}

static PyMemberDef PyTypedDataObject_members[] = {
    {"ModBase", T_ULONGLONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, ModBase),
        0, "ModBase"},
    {"Offset", T_ULONGLONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Offset),
        0, "Offset"},
    {"Data", T_ULONGLONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Data),
        0, "Data"},
    {"Size", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Size),
        0, "Size"},
    {"Flags", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Flags),
        0, "Flags"},
    {"TypeId", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, TypeId),
        0, "TypeId"},
    {"BaseTypeId", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, BaseTypeId),
        0, "BaseTypeId"},
    {"Tag", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Tag),
        0, "Tag"},
    {"Register", T_ULONG, 
        offsetof(PyTypedDataObject, td) +
        offsetof(DEBUG_TYPED_DATA, Register),
        0, "Register"},
    {NULL}
};

static PyMethodDef PyTypedDataObject_methods[] = {
    {NULL}
};

PyTypeObject PyTypedDataType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugTypedData", /*tp_name*/
    sizeof(PyTypedDataObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "DebugTypedData object",        /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyTypedDataObject_methods, /* tp_methods */
    PyTypedDataObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgtd_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};
#endif

