#ifndef _WINSTRUCTS_H
#define _WINSTRUCTS_H

// XXX - MEMORY_BASIC_INFORMATION64 - QueryVirtual

#include <windows.h>
#include <dbgeng.h>
#include <Python.h>
#include <structmember.h>

#if defined(_M_IX86)

/*
typedef struct _M128A {
	UINT64 Low;
	INT64  High;
} M128A;

*/

typedef struct XMM_SAVE_AREA32 {
	UINT16       ControlWord;                                             
	UINT16       StatusWord;                                              
	UINT8        TagWord;                                                 
	UINT8        Reserved1;                                               
	UINT16       ErrorOpcode;                                             
	ULONG32      ErrorOffset;                                             
	UINT16       ErrorSelector;                                           
	UINT16       Reserved2;                                               
	ULONG32      DataOffset;                                              
	UINT16       DataSelector;                                            
	UINT16       Reserved3;                                               
	ULONG32      MxCsr;                                                   
	ULONG32      MxCsr_Mask;                                              
	struct _M128A FloatRegisters[8];                                      
	struct _M128A XmmRegisters[16];                                       
	UINT8        Reserved4[96];                                           
}XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;
#endif

typedef struct DECLSPEC_ALIGN(16) _CONTEXT64 {
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;
    DWORD ContextFlags;
    DWORD MxCsr;
    WORD   SegCs;
    WORD   SegDs;
    WORD   SegEs;
    WORD   SegFs;
    WORD   SegGs;
    WORD   SegSs;
    DWORD EFlags;
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;
    DWORD64 Rip;
    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        };
    };
    M128A VectorRegister[26];
    DWORD64 VectorControl;
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} CONTEXT64, *PCONTEXT64;

#define MAXIMUM_SUPPORTED_EXTENSION     512
#define SIZE_OF_80387_REGISTERS         80

typedef struct _CONTEXT32 {
    DWORD ContextFlags;
    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;
    struct {
        DWORD   ControlWord;
        DWORD   StatusWord;
        DWORD   TagWord;
        DWORD   ErrorOffset;
        DWORD   ErrorSelector;
        DWORD   DataOffset;
        DWORD   DataSelector;
        BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
        DWORD   Cr0NpxState;
    } FloatSave;
    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;
    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;
    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;
    DWORD   EFlags;
    DWORD   Esp;
    DWORD   SegSs;
    BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXT32, *PCONTEXT32;


typedef struct {
    PyObject_HEAD
    DEBUG_SYMBOL_ENTRY sym;
} PyDebugSymbolEntryObject;

typedef struct {
    PyObject_HEAD
    DEBUG_REGISTER_DESCRIPTION reg;
} PyDebugRegisterDescriptionObject;

typedef struct {
    PyObject_HEAD
    DEBUG_STACK_FRAME frame;
} PyDebugStackFrameObject;

typedef struct {
    PyObject_HEAD
    DEBUG_MODULE_PARAMETERS dmp;
} PyDebugModuleParametersObject;

typedef struct {
    PyObject_HEAD
    DEBUG_BREAKPOINT_PARAMETERS bpp;
} PyDebugBreakPointParametersObject;

typedef struct {
    PyObject_HEAD
    ULONG64 ModuleBase;
    ULONG64 Id;
} PyDebugModuleAndIdObject;

typedef struct {
    PyObject_HEAD
    EXCEPTION_RECORD64 exr; 
} PyExceptionRecordObject;

typedef struct {
    PyObject_HEAD
    CONTEXT64 ctx;
} PyContext64Object;

typedef struct {
    PyObject_HEAD
    CONTEXT32 ctx;
} PyContext32Object;

typedef struct {
    PyObject_HEAD
    DEBUG_CREATE_PROCESS_OPTIONS cpo;
} PyCreateProcessOptionsObject;

typedef struct {
    PyObject_HEAD
    DEBUG_EXCEPTION_FILTER_PARAMETERS efp;
} PyExceptionFilterParametersObject;

typedef struct {
    PyObject_HEAD
    DEBUG_HANDLE_DATA_BASIC hdb;
} PyHandleDataBasicObject;

typedef struct {
    PyObject_HEAD
    DEBUG_SPECIFIC_FILTER_PARAMETERS sfp;
} PySpecificFilterParametersObject;

typedef struct {
    PyObject_HEAD
    DEBUG_SYMBOL_PARAMETERS sp;
} PySymbolParametersObject;

typedef struct {
    PyObject_HEAD
    DEBUG_SYMBOL_SOURCE_ENTRY sse;
} PySymbolSourceEntryObject;

typedef struct {
    PyObject_HEAD
    DEBUG_THREAD_BASIC_INFORMATION tbi;
} PyThreadBasicInformationObject;

// WbgExts.h
//typedef struct {
//    PyObject_HEAD
//    DEBUG_TYPED_DATA td;
//} PyTypedDataObject;

extern PyTypeObject PyDebugSymbolEntryType;
extern PyTypeObject PyDebugRegisterDescriptionType;
extern PyTypeObject PyDebugStackFrameType;
extern PyTypeObject PyDebugModuleParametersType;
extern PyTypeObject PyDebugBreakPointParametersType;
extern PyTypeObject PyDebugModuleAndIdType;
extern PyTypeObject PyExceptionRecordType;
extern PyTypeObject PyContext64Type;
extern PyTypeObject PyContext32Type;
extern PyTypeObject PyCreateProcessOptionsType;
extern PyTypeObject PyExceptionFilterParametersType;
extern PyTypeObject PyHandleDataBasicType;
extern PyTypeObject PySpecificFilterParametersType;
extern PyTypeObject PySymbolParametersType;
extern PyTypeObject PySymbolSourceEntryType;
extern PyTypeObject PyThreadBasicInformationType;
//extern PyTypeObject PyTypedDataType;

#endif

