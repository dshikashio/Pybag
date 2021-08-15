from ctypes             import *
from comtypes.hresult   import S_OK, S_FALSE
import functools

from . import core as DbgEng
from . import exception
from .idebugbreakpoint import DebugBreakpoint
from .util import logger

class DebugControl(object):
    def __init__(self, controlobj):
        self._ctrl = controlobj
        exception.wrap_comclass(self._ctrl)

    # IDebugControl

    def GetInterrupt(self):
        hr = self._ctrl.GetInterrupt()
        if hr == S_OK:
            return True
        elif hr == S_FALSE:
            return False
        else:
            exception.check_err(hr)

    def SetInterrupt(self, flags=DbgEng.DEBUG_INTERRUPT_ACTIVE):
        hr = self._ctrl.SetInterrupt(flags)
        exception.check_err(hr)

    def GetIntteruptTimeout(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetIntteruptTimeout()
        #exception.check_err(hr)
        #return seconds

    def SetInterruptTimeout(self, timeout):
        hr = self._ctrl.SetInterruptTimeout(timeout)
        exception.check_err(hr)

    def GetLogFile(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetLogFile()
        #exception.check_err(hr)
        #return logfile

    def OpenLogFile(self, filename, append=False):   
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.OpenLogFile()
        #exception.check_err(hr)

    def CloseLogFile(self):   
        hr = self._ctrl.CloseLogFile()
        exception.check_err(hr)

    def GetLogMask(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetLogMask()
        #exception.check_err(hr)
        #return mask

    def SetLogMask(self, mask):
        hr = self._ctrl.SetLogMask()
        exception.check_err(hr)

    def Input(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.Input()
        #exception.check_err(hr)
        #return input

    def ReturnInput(self, input):
        hr = self._ctrl.ReturnInput(input)
        exception.check_hr(hr)

    def Output(self, mask, output):
        hr = self._ctrl.Output(mask, output)
        exception.check_hr(hr)

    def OutputVaList(self):
        raise exception.E_NOTIMPL_Error

    def ControlledOutput(self):
        raise exception.E_NOTIMPL_Error

    def ControlledOutputVaList(self):
        raise exception.E_NOTIMPL_Error

    def OutputPrompt(self):
        raise exception.E_NOTIMPL_Error

    def OutputPromptVaList(self):
        raise exception.E_NOTIMPL_Error

    def GetPromptText(self):
        raise exception.E_NOTIMPL_Error

    def OutputCurrentState(self):
        outcontrol = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        flags = DbgEng.DEBUG_CURRENT_DEFAULT
        hr = self._ctrl.OutputCurrentState(outcontrol, flags)
        exception.check_hr(hr)

    def OutputVersionInformation(self):
        hr = self._ctrl.OutputVersionInformation(DbgEng.DEBUG_OUTCTL_ALL_CLIENTS)
        exception.check_hr(hr)

    def GetNotifyEventHandle(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNotifyEventHandle()
        #exception.check_err(hr)
        #return handle

    def SetNotifyEventHandle(self, handle):
        hr = self._ctrl.SetNotifyEventHandle(handle)
        exception.check_err(hr)

    def Asssemble(self, offset, instr):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.Assemble(offset, instr, ...)
        #exception.check_err(hr)
        #return endoffset

    def Disassemble(self, offset):
        raise exception.E_NOTIMPL_Error
        #flags = DEBUG_DISASM_EFFECTIVE_ADDRESS
        #hr = self._ctrl.Disassemble()
        #exception.check_err(hr)
        #return (instr, endoffset)

    def GetDisassembleEffectiveOffset(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetDisassembleEffectiveOffset()
        #exception.check_err(hr)
        #return offset

    def OutputDisassembly(self, offset):
        raise exception.E_NOTIMPL_Error
        #outcontrol = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        #flags = DbgEng.DEBUG_DISASM_EFFECTIVE_ADDRESS | DbgEng.DEBUG_DISASM_MATCHING_SYMBOLS
        #hr = self._ctrl.OutputDisassembly()
        #exception.check_err(hr)
        #return endoffset

    def OutputDisassemblyLines(self):
        raise exception.E_NOTIMPL_Error

    def GetNearInstruction(self):
        raise exception.E_NOTIMPL_Error

    def GetStackTrace(self):
        frames = (DbgEng._DEBUG_STACK_FRAME * 50)()
        count = c_ulong()
        hr = self._ctrl.GetStackTrace(0, 0, 0, frames, 50, byref(count))
        exception.check_err(hr)
        return list(frames)[:count.value]

    def GetReturnOffset(self):
        offset = c_ulonglong()
        hr = self._ctrl.GetReturnOffset(byref(offset))
        exception.check_err(hr)
        return offset.value

    def OutputStackTrace(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.OutputStackTrace()
        #exception.check_err(hr)

    def GetDebuggeeType(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetDebuggeeType()
        #exception.check_err(hr)
        #return (class, qualifier)

    def GetActualProcessorType(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetActualProcessorType()
        #exception.check_err(hr)
        #return type

    def GetExecutingProcessorType(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetExecutingProcessorType()
        #exception.check_err(hr)
        #return type

    def GetNumberPossibleExecutingProcessorTypes(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNumberPossibleExecutingProcessorTypes()
        #exception.check_err(hr)
        #return number

    def GetPossibleExecutingProcessorTypes(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetPossibleExecutingProcessorTypes()
        #exception.check_err(hr)
        #return types

    def GetNumberProcessors(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNumberProcessors()
        #exception.check_err(hr)
        #return number

    def GetSystemVersion(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetSystemVersion()
        #exception.check_err(hr)
        #return (platformid, major, minor, servicepack, build)

    def GetPageSize(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetPageSize()
        #exception.check_err(hr)
        #return size

    def IsPointer64Bit(self):
        hr = self._ctrl.IsPointer64Bit()
        if hr == S_OK:
            return True
        elif hr == S_FALSE:
            return False
        exception.check_err(hr)

    def ReadBugCheckData(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.ReadBugCheckData()
        #exception.check_err(hr)
        #return (code, arg1, arg2, arg3, arg4)

    def GetNumberSupportedProcessorTypes(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNumberSupportedProcessorTypes()
        #exception.check_err(hr)
        #return number

    def GetSupportedProcessorTypes(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetSupportedProcessorTypes()
        #exception.check_err(hr)
        #return types

    def GetProcessorTypeNames(self, type):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetProcessorTypeNames()
        #exception.check_err(hr)
        #return (fullname, abbrevname)

    def GetEffectiveProcessorType(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetEffectiveProcessorType()
        #exception.check_err(hr)
        #return type

    def SetEffectiveProcessorType(self, type):
        hr = self._ctrl.SetEffectiveProcessorType(type)
        exception.check_err(hr)

    @logger
    def GetExecutionStatus(self):
        status = c_ulong()
        hr = self._ctrl.GetExecutionStatus(byref(status))
        exception.check_err(hr)
        return status.value

    @logger
    def SetExecutionStatus(self, status):
        hr = self._ctrl.SetExecutionStatus(status)
        exception.check_err(hr)

    def GetCodeLevel(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetCodeLevel()
        #exception.check_err(hr)
        #return level

    def SetCodeLevel(self, level):
        hr = self._ctrl.SetCodeLevel(level)
        exception.check_err(hr)

    def GetEngineOptions(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetEngineOptions()
        #exception.check_err(hr)
        #return options

    def AddEngineOptions(self, options):
        hr = self._ctrl.AddEngineOptions(options)
        exception.check_err(hr)

    def RemoveEngineOptions(self, options):
        hr = self._ctrl.RemoveEngineOptions(options)
        exception.check_err(hr)

    def SetEngineOptions(self, options):
        hr = self._ctrl.SetEngineOptions(options)
        exception.check_err(hr)

    def GetSystemErrorControl(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetSystemErrorControl()
        #exception.check_err(hr)
        #return (outputlevel, breaklevel)

    def SetSystemErrorControl(self, outputlevel, breaklevel):
        hr = self._ctrl.SetSystemErrorControl(outputlevel, breaklevel)
        exception.check_err(hr)

    def GetTextMacro(self):
        raise exception.E_NOTIMPL_Error

    def SetTextMacro(self):
        raise exception.E_NOTIMPL_Error

    def GetRadix(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetRadix()
        #exception.check_err(hr)
        #return radix

    def SetRadix(self, radix):
        hr = self._ctrl.SetRadix(radix)
        exception.check_err(hr)

    def Evaluate(self):
        raise exception.E_NOTIMPL_Error

    def CoerceValue(self):
        raise exception.E_NOTIMPL_Error

    def CoerceValues(self):
        raise exception.E_NOTIMPL_Error

    def Execute(self, cmd):
        if isinstance(cmd, str):
            cmd = cmd.encode()
        outctrl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        hr = self._ctrl.Execute(outctrl, cmd, 0)
        exception.check_err(hr)

    def ExecuteCommandFile(self, file):
        outctrl = DbgEng.DEBUG_OUTCTL_ALL_CLIENTS
        hr = self._ctrl.ExecuteCommandFile(outctrl, file, 0)
        exception.check_err(hr)

    def GetNumberBreakpoints(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNumberBreakpoints()
        #exception.check_err(hr)
        #return number

    def GetBreakpointByIndex(self, index):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetBreakpointByIndex()
        #exception.check_err(hr)
        #return bp

    def GetBreakpointById(self, id):
        bp      = POINTER(DbgEng.IDebugBreakpoint)()
        bpptr   = POINTER(POINTER(DbgEng.IDebugBreakpoint))(bp)
        hr = self._ctrl.GetBreakpointById(id, bpptr)
        exception.check_err(hr)
        return DebugBreakpoint(bp)

    def GetBreakpointParameters(self, count):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetBreakpointParameters()
        #exception.check_err(hr)
        #return params

    def AddBreakpoint(self, type=DbgEng.DEBUG_BREAKPOINT_CODE):
        id      = DbgEng.DEBUG_ANY_ID
        bp      = POINTER(DbgEng.IDebugBreakpoint)()
        bpptr   = POINTER(POINTER(DbgEng.IDebugBreakpoint))(bp)
        hr = self._ctrl.AddBreakpoint(type, id, bpptr)
        exception.check_err(hr)
        return DebugBreakpoint(bp)

    def RemoveBreakpoint(self, bp):
        hr = self._ctrl.RemoveBreakpoint(bp._bp)
        exception.check_err(hr)

    def AddExtension(self):
        raise exception.E_NOTIMPL_Error

    def RemoveExtension(self):
        raise exception.E_NOTIMPL_Error

    def GetExtensionByPath(self):
        raise exception.E_NOTIMPL_Error

    def CallExtension(self):
        raise exception.E_NOTIMPL_Error

    def GetExtensionFunction(self):
        raise exception.E_NOTIMPL_Error

    def GetWindbgExtensionApis32(self):
        raise exception.E_NOTIMPL_Error

    def GetWindbgExtensionApis64(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberEventFilters(self):
        raise exception.E_NOTIMPL_Error

    def GetEventFilterText(self):
        raise exception.E_NOTIMPL_Error

    def GetEventFilterCommand(self):
        raise exception.E_NOTIMPL_Error

    def SetEventFilterCommand(self):
        raise exception.E_NOTIMPL_Error

    def GetSpecificFilterParameters(self):
        raise exception.E_NOTIMPL_Error

    def SetSpecificFilterParameters(self):
        raise exception.E_NOTIMPL_Error

    def GetSpecificFilterArgument(self):
        raise exception.E_NOTIMPL_Error

    def SetSpecificFilterArgument(self):
        raise exception.E_NOTIMPL_Error

    def GetExceptionFilterParameters(self):
        raise exception.E_NOTIMPL_Error

    def SetExceptionFilterParameters(self):
        raise exception.E_NOTIMPL_Error

    def GetExceptionFilterSecondCommand(self):
        raise exception.E_NOTIMPL_Error

    def SetExceptionFilterSecondCommand(self):
        raise exception.E_NOTIMPL_Error

    def WaitForEvent(self, timeout=DbgEng.WAIT_INFINITE):
        hr = self._ctrl.WaitForEvent(0, timeout)
        if hr == S_FALSE:
            raise exception.DbgEngTimeout("WaitForEvent timeout: {}".format(timeout))
        exception.check_err(hr)

    def GetLastEventInformation(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetLastEventInformation()
        #exception.check_err(hr)
        #return (type, pid, tid, extra, desc)

    # IDebugControl2

    def GetCurrentTimeDate(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetCurrentTimeDate()
        #exception.check_err(hr)
        #return timedate

    def GetCurrentSystemUpTime(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetCurrentSystemUpTime()
        #exception.check_err(hr)
        #return uptime

    def GetDumpFormatFlags(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetDumpFormatFlags()
        #exception.check_err(hr)
        #return flags

    def GetNumberTextReplacements(self):
        raise exception.E_NOTIMPL_Error

    def GetTextReplacement(self):
        raise exception.E_NOTIMPL_Error

    def SetTextReplacement(self):
        raise exception.E_NOTIMPL_Error

    def RemoveTextReplacements(self):
        raise exception.E_NOTIMPL_Error

    def OutputTextReplacements(self):
        raise exception.E_NOTIMPL_Error

    # IDebugControl3

    def GetAssemblyOptions(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetAssemblyOptions()
        #exception.check_err(hr)
        #return options

    def AddAssemblyOptions(self, options):
        hr = self._ctrl.AddAssemblyOptions(options)
        exception.check_err(hr)

    def RemoveAssemblyOptions(self, options):
        hr = self._ctrl.RemoveAssemblyOptions(options)
        exception.check_err(hr)

    def SetAssemblyOptions(self, options):
        hr = self._ctrl.SetAssemblyOptions(options)
        exception.check_err(hr)

    def GetExpressionSyntax(self):
        raise exception.E_NOTIMPL_Error

    def SetExpressionSyntax(self):
        raise exception.E_NOTIMPL_Error

    def SetExpressionSyntaxByName(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberExpressionSyntaxes(self):
        raise exception.E_NOTIMPL_Error

    def GetExpressionSyntaxNames(self):
        raise exception.E_NOTIMPL_Error

    def GetNumberEvents(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetNumberEvents()
        #exception.check_err(hr)
        #return events

    def GetEventIndexDescription(self):
        raise exception.E_NOTIMPL_Error

    def GetCurrentEventIndex(self):
        raise exception.E_NOTIMPL_Error

    def SetNextEventIndex(self):
        raise exception.E_NOTIMPL_Error

    # IDebugControl4

    def GetLogFileWide(self):
        raise exception.E_NOTIMPL_Error

    def OpenLogFileWide(self):
        raise exception.E_NOTIMPL_Error

    def InputWide(self):
        raise exception.E_NOTIMPL_Error

    def ReturnInputWide(self):
        raise exception.E_NOTIMPL_Error

    def OutputWide(self):
        raise exception.E_NOTIMPL_Error

    def OutputVaListWide(self):
        raise exception.E_NOTIMPL_Error

    def ControlledOutputWide(self):
        raise exception.E_NOTIMPL_Error

    def ControlledOutputVaListWide(self):
        raise exception.E_NOTIMPL_Error

    def OutputPromptWide(self):
        raise exception.E_NOTIMPL_Error

    def OutputPromptVaListWide(self):
        raise exception.E_NOTIMPL_Error

    def GetPromptTextWide(self):
        raise exception.E_NOTIMPL_Error

    def AssembleWide(self):
        raise exception.E_NOTIMPL_Error

    def DisassembleWide(self):
        raise exception.E_NOTIMPL_Error

    def GetProcessorTypeNamesWide(self):
        raise exception.E_NOTIMPL_Error

    def GetTextMacroWide(self):
        raise exception.E_NOTIMPL_Error

    def SetTextMacroWide(self):
        raise exception.E_NOTIMPL_Error

    def EvaluateWide(self):
        raise exception.E_NOTIMPL_Error

    def ExecuteWide(self):
        raise exception.E_NOTIMPL_Error

    def ExecuteCommandFileWide(self):
        raise exception.E_NOTIMPL_Error

    def GetBreakpointByIndex2(self, index):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetBreakpointByIndex2()
        #exception.check_err(hr)
        #return bp

    def GetBreakpointById2(self, id):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetBreakpointById2()
        #exception.check_err(hr)
        #return bp

    def AddBreakpoint2(self, type=DbgEng.DEBUG_BREAKPOINT_CODE):
        raise exception.E_NOTIMPL_Error
        #id = DbgEng.DEBUG_ANY_ID
        #hr = self._ctrl.AddBreakpoint2()
        #exception.check_err(hr)
        #return bp

    def RemoveBreakpoint2(self, bp):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.RemoveBreakpoint2()
        #exception.check_err(hr)

    def AddExtensionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetExtensionByPathWide(self):
        raise exception.E_NOTIMPL_Error

    def CallExtensionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetExtensionFunctionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetEventFilterTextWide(self):
        raise exception.E_NOTIMPL_Error

    def GetEventFilterCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def SetEventFilterCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def GetSpecificFilterArgumentWide(self):
        raise exception.E_NOTIMPL_Error

    def SetSpecificFilterArgumentWide(self):
        raise exception.E_NOTIMPL_Error

    def GetExceptionFilterSecondCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def SetExceptionFilterSecondCommandWide(self):
        raise exception.E_NOTIMPL_Error

    def GetLastEventInformationWide(self):
        raise exception.E_NOTIMPL_Error

    def GetTextReplacementWide(self):
        raise exception.E_NOTIMPL_Error

    def SetTextReplacementWide(self):
        raise exception.E_NOTIMPL_Error

    def SetExpressionSyntaxByNameWide(self):
        raise exception.E_NOTIMPL_Error

    def GetExpressionSyntaxNamesWide(self):
        raise exception.E_NOTIMPL_Error

    def GetEventIndexDescriptionWide(self):
        raise exception.E_NOTIMPL_Error

    def GetLogFile2(self):
        raise exception.E_NOTIMPL_Error

    def OpenLogFile2(self):
        raise exception.E_NOTIMPL_Error

    def GetLogFile2Wide(self):
        raise exception.E_NOTIMPL_Error

    def OpenLogFile2Wide(self):
        raise exception.E_NOTIMPL_Error

    def GetSystemVersionValues(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetSystemVersionValues()
        #exception.check_err(hr)
        #return (platformid, major, minor, kdmajor, kdminor)

    def GetSystemVersionValues(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetSystemVersionValues()
        #exception.check_err(hr)
        #return (platformid, major, minor, kdmajor, kdminor)

    def GetSystemVersionString(self):
        raise exception.E_NOTIMPL_Error
        #which = DbgEng.DEBUG_SYSVERSTR_BUILD
        #hr = self._ctrl.GetSystemVersionString()
        #exception.check_err(hr)
        #return version

    def GetSystemVersionStringWide(self):
        raise exception.E_NOTIMPL_Error

    def GetContextStackTrace(self):
        raise exception.E_NOTIMPL_Error

    def OutputContextStackTrace(self):
        raise exception.E_NOTIMPL_Error

    def GetStoredEventInformation(self):
        raise exception.E_NOTIMPL_Error

    def GetManagedStatus(self):
        raise exception.E_NOTIMPL_Error

    def GetManagedStatusWide(self):
        raise exception.E_NOTIMPL_Error

    def ResetManagedStatus(self):
        raise exception.E_NOTIMPL_Error

    # IDebugControl5

    def GetStackTraceEx(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetStackTraceEx()
        #exception.check_err(hr)
        #return frames

    def OutputStackTraceEx(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.OutputStackTraceEx()
        #exception.check_err(hr)

    def GetContextStackTraceEx(self):
        raise exception.E_NOTIMPL_Error

    def OutputContextStackTraceEx(self):
        raise exception.E_NOTIMPL_Error

    def GetBreakpointByGuid(self):
        raise exception.E_NOTIMPL_Error

    # IDebugControl6

    def GetExecutionStatusEx(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetExecutionStatusEx()
        #exception.check_err(hr)
        #return status

    def GetSynchronizationStatus(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetExecutionStatusEx()
        #exception.check_err(hr)
        #return status

    # IDebugControl7

    def GetDebuggeeType2(self):
        raise exception.E_NOTIMPL_Error
        #hr = self._ctrl.GetExecutionStatusEx()
        #exception.check_err(hr)
        #return status
