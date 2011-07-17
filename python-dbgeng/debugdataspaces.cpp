#include "pydbgeng.h"

// Usually from ddk
#define STATUS_NO_MORE_ENTRIES 0x8000001aL
#define STATUS_NO_PAGEFILE     0xC0000147L


static PyObject *
dbgdata_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebugDataSpacesObject *self;

    self = (PyDebugDataSpacesObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->data = NULL;
        self->weakreflist = NULL;
    }
    return (PyObject *)self;
}

static void
dbgdata_dealloc(PyDebugDataSpacesObject *self)
{
    if (self->weakreflist != NULL)
        PyObject_ClearWeakRefs((PyObject *)self);

    if (self->data != NULL) {
        self->data->Release();
        self->data = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int 
dbgdata_init(PyObject *oself, PyObject *args, PyObject *kwds)
{
    HRESULT hr;
    PyDebugDataSpacesObject *self = (PyDebugDataSpacesObject *)oself;
    PyDebugClientObject *client = NULL;
    IDebugDataSpaces4 *debugDataSpaces = NULL;
    static char *kwlist[] = {"client", NULL};

    if (self->data) {
        self->data->Release();
        self->data = NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:DebugDataSpaces",
                kwlist, &PyDebugClientType, &client))
        return -1;
    if (client->client == NULL) {
        err_noclient();
        return -1;
    }
    if ((hr = client->client->QueryInterface(__uuidof(IDebugDataSpaces4),
                    (void **)&debugDataSpaces)) != S_OK)
    {
        err_dbgeng(hr);
        return -1;
    }

    self->data = debugDataSpaces;
    return 0;
}

static PyObject *
dbgdata_ReadBusData(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
    ULONG Offset;
    PVOID Buffer = NULL;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret = NULL;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IIIII:ReadBusData", &BusDataType, &BusNumber,
                &SlotNumber, &Offset, &BufferSize))
        return NULL;

    Buffer = (PVOID)PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadBusData(BusDataType, BusNumber, SlotNumber, Offset,
            Buffer, BufferSize, &BytesRead)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_WriteBusData(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
    ULONG Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IIIIIz#:WriteBusData", &BusDataType, &BusNumber,
                &SlotNumber, &Offset, &Buffer, &BufferSize))
        return NULL;

    if ((hr = self->data->WriteBusData(BusDataType, BusNumber, SlotNumber, Offset,
            Buffer, BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_ReadControl(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Processor;
    ULONG64 Offset;
    PVOID Buffer = NULL;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret = NULL;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IKI:ReadControl", 
                &Processor, &Offset, &BufferSize))
        return NULL;

    Buffer = (PVOID)PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadControl(Processor, Offset,
            Buffer, BufferSize, &BytesRead)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_WriteControl(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Processor;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IKz#:WriteControl", &Processor,
                &Offset, &Buffer, &BufferSize))
        return NULL;

    if ((hr = self->data->WriteControl(Processor, Offset,
            Buffer, BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_ReadDebuggerData(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Index;
    ULONG64 Buffer;
    ULONG DataSize;

    // XXX - does the size conversion need to be explicit?
    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "I:ReadDebuggerData", &Index))
        return NULL;
    if ((hr = self->data->ReadDebuggerData(Index, &Buffer, 
                    sizeof(Buffer), &DataSize)) != S_OK)
        return err_dbgeng(hr);
    // XXX - Return value size changes based off Index
    return Py_BuildValue("K", Buffer);
}

static PyObject *
dbgdata_ReadHandleData(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Handle;
    ULONG DataType;
    PVOID Buffer = NULL;
    ULONG DataSize;
    PyObject *ret = NULL;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KI:ReadHandleData", &Handle, &DataType))
        return NULL;

    if (DataType == DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE ||
        DataType == DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE)
    {
        return PyErr_Format(PyExc_ValueError, "Invalid DataType");
    }

    hr = self->data->ReadHandleData(Handle, DataType, NULL, 0, &DataSize);
    if (hr != S_OK && hr != S_FALSE)
        return err_dbgeng(hr);
    Buffer = PyMem_Malloc(DataSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadHandleData(Handle, DataType, 
                    Buffer, DataSize, NULL)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    switch (DataType)
    {
    /* Causes segfault...wtf
    case DEBUG_HANDLE_DATA_TYPE_BASIC:
        {
            PDEBUG_HANDLE_DATA_BASIC phdb;
            PyHandleDataBasicObject *hdb;

            phdb = (PDEBUG_HANDLE_DATA_BASIC)Buffer;
            hdb = (PyHandleDataBasicObject *)PyObject_CallObject(
                    (PyObject *)&PyHandleDataBasicType, NULL);
            if (hdb != NULL) {
                hdb->hdb = *phdb;
                return Py_BuildValue("N", hdb);
                // Contains reference to buffer so don't want to free
            }
            else {
                ret = NULL;
            }
        }
        break;
    */
    case DEBUG_HANDLE_DATA_TYPE_TYPE_NAME:
    case DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME:
        ret = Py_BuildValue("s", Buffer);
        break;
    case DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT:
        ret = Py_BuildValue("I", *((ULONG *)Buffer));
        break;
    default:
        PyErr_Format(PyExc_ValueError, "Invalid DataType");
        ret = NULL;
        break;
    }

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_ReadIo(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG64 Offset;
    PVOID Buffer = NULL;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret = NULL;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IIIKI:ReadIo", &InterfaceType, &BusNumber,
                &AddressSpace, &Offset, &BufferSize))
        return NULL;

    Buffer = (PVOID)PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadIo(InterfaceType, BusNumber, AddressSpace, Offset,
            Buffer, BufferSize, &BytesRead)) != S_OK)
    {
        err_dbgeng(hr);
        goto done;
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);

done:
    if (Buffer) PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_WriteIo(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IIIKz#:WriteIo", &InterfaceType, &BusNumber,
                &AddressSpace, &Offset, &Buffer, &BufferSize))
        return NULL;

    if ((hr = self->data->WriteIo(InterfaceType, BusNumber, AddressSpace, Offset,
            Buffer, BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);

    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_ReadMSR(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG MSR;
    ULONG64 Value;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "I:ReadMsr", &MSR))
        return NULL;
    if ((hr = self->data->ReadMsr(MSR, &Value)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", Value);
}

static PyObject *
dbgdata_WriteMSR(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG MSR;
    ULONG64 Value;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IK:WriteMsr", &MSR, &Value))
        return NULL;
    if ((hr = self->data->WriteMsr(MSR, Value)) != S_OK)
        return err_dbgeng(hr);
    Py_RETURN_NONE;
}

/*
static PyObject *
dbgdata_GetOffsetInformation(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG Space;
    ULONG Which;
    ULONG64 Offset;
    

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "IIK:GetOffsetInformation", 
                &Space, &Which, &Offset))
        return NULL;
#if 0
    if ((hr = self->data->GetOffsetInformation(Space, Which, Offset,
                    Buffer, )) != S_OK)
        return err_dbgeng(hr);
#endif
    Py_RETURN_NONE;
}
*/

static PyObject *
dbgdata_FillPhysical(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Start;
    ULONG Size;
    PVOID Pattern;
    ULONG PatternSize;
    ULONG Filled;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KIs#:FillPhysical", &Start, &Size, 
                &Pattern, &PatternSize))
        return NULL;
    if ((hr = self->data->FillPhysical(Start, Size, Pattern,
                    PatternSize, &Filled)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Filled);
}

static PyObject *
dbgdata_ReadPhysical(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KI:ReadPhysical", &Offset, &BufferSize))
        return NULL;

    Buffer = PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadPhysical(Offset, Buffer, 
                    BufferSize, &BytesRead)) != S_OK)
    {
        PyMem_Free(Buffer);
        return err_dbgeng(hr);
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);
    PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_ReadPhysical2(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Flags;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KII:ReadPhysical2", &Offset, &Flags, &BufferSize))
        return NULL;

    Buffer = PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadPhysical2(Offset, Flags, Buffer, 
                    BufferSize, &BytesRead)) != S_OK)
    {
        PyMem_Free(Buffer);
        return err_dbgeng(hr);
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);
    PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_WritePhysical(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "Ks#:WritePhysical", &Offset,
                &Buffer, &BufferSize))
        return NULL;
    if ((hr = self->data->WritePhysical(Offset, Buffer, 
                    BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_WritePhysical2(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG Flags;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KIs#:WritePhysical2", &Offset, &Flags,
                &Buffer, &BufferSize))
        return NULL;
    if ((hr = self->data->WritePhysical2(Offset, Flags, Buffer, 
                    BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_FillVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Start;
    ULONG Size;
    PVOID Pattern;
    ULONG PatternSize;
    ULONG Filled;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KIs#:FillVirtual", &Start, &Size, 
                &Pattern, &PatternSize))
        return NULL;
    if ((hr = self->data->FillVirtual(Start, Size, Pattern,
                    PatternSize, &Filled)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", Filled);
}

static PyObject *
dbgdata_QueryVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    MEMORY_BASIC_INFORMATION64 Info;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "K:QueryVirtual", &Offset))
        return NULL;
    if ((hr = self->data->QueryVirtual(Offset, &Info)) != S_OK)
        return err_dbgeng(hr);
    // XXX - should this return a struct?
    return Py_BuildValue("KKIKIII", Info.BaseAddress, Info.AllocationBase,
            Info.AllocationProtect, Info.RegionSize, Info.State,
            Info.Protect, Info.Type);
}

static PyObject *
dbgdata_ReadVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KI:ReadVirtual", &Offset, &BufferSize))
        return NULL;

    Buffer = PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadVirtual(Offset, Buffer, 
                    BufferSize, &BytesRead)) != S_OK)
    {
        PyMem_Free(Buffer);
        return err_dbgeng(hr);
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);
    PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_SearchVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG64 Length;
    PVOID Pattern;
    ULONG PatternSize;
    ULONG PatternGranularity = 1;
    ULONG64 MatchOffset;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KKs#|I:SearchVirtual", 
                &Offset, &Length, &Pattern, &PatternSize, &PatternGranularity))
        return NULL;
    if ((hr = self->data->SearchVirtual(Offset, Length, Pattern,
        PatternSize, PatternGranularity, &MatchOffset)) != S_OK)
    {
        if (hr == HRESULT_FROM_NT(STATUS_NO_MORE_ENTRIES))
            Py_RETURN_NONE;
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("K", MatchOffset);
}

static PyObject *
dbgdata_WriteVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "Is#:WriteVirtual", &Offset,
                &Buffer, &BufferSize))
        return NULL;
    if ((hr = self->data->WriteVirtual(Offset, Buffer, 
                    BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_ReadVirtualUncached(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesRead;
    PyObject *ret;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KI:ReadVirtualUncached", &Offset, &BufferSize))
        return NULL;

    Buffer = PyMem_Malloc(BufferSize);
    if (Buffer == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    if ((hr = self->data->ReadVirtualUncached(Offset, Buffer, 
                    BufferSize, &BytesRead)) != S_OK)
    {
        PyMem_Free(Buffer);
        return err_dbgeng(hr);
    }

    ret = Py_BuildValue("s#", Buffer, BytesRead);
    PyMem_Free(Buffer);
    return ret;
}

static PyObject *
dbgdata_WriteVirtualUncached(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG BytesWritten;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "Is#:WriteVirtualUncached", &Offset,
                &Buffer, &BufferSize))
        return NULL;
    if ((hr = self->data->WriteVirtualUncached(Offset, Buffer, 
                    BufferSize, &BytesWritten)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("I", BytesWritten);
}

static PyObject *
dbgdata_VirtualToPhysical(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Virtual;
    ULONG64 Physical;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "K:VirtualToPhysical", &Virtual))
        return NULL;
    if ((hr = self->data->VirtualToPhysical(Virtual, &Physical)) != S_OK) {
        if (hr == HRESULT_FROM_NT(STATUS_NO_PAGEFILE))
            return err_fail("No physical page containing the specified "
                "address could be found.");
        else
            return err_dbgeng(hr);
    }
    return Py_BuildValue("K", Physical);
}

static PyObject *
dbgdata_GetValidRegionVirtual(PyDebugDataSpacesObject *self, PyObject *args)
{
    HRESULT hr;
    ULONG64 Base;
    ULONG Size;
    ULONG64 ValidBase;
    ULONG ValidSize;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, "KI:GetValidRegionVirtual", 
                &Base, &Size))
        return NULL;
    if ((hr = self->data->GetValidRegionVirtual(Base, Size, &ValidBase, 
                    &ValidSize)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("KI", ValidBase, ValidSize);
}

static PyObject *
dbgdata_GetNextDifferentlyValidOffsetVirtual(PyDebugDataSpacesObject *self, 
        PyObject *args)
{
    HRESULT hr;
    ULONG64 Offset;
    ULONG64 NextOffset;

    if (self->data == NULL)
        return err_nodata();
    if (!PyArg_ParseTuple(args, 
                "K:GetNextDifferentlyValidOffsetVirtual", &Offset))
        return NULL;
    if ((hr = self->data->GetNextDifferentlyValidOffsetVirtual(Offset, 
                    &NextOffset)) != S_OK)
        return err_dbgeng(hr);
    return Py_BuildValue("K", NextOffset);
}

static PyMethodDef PyDebugDataSpaces_methods[] = {
    {"ReadBusData", 
     (PyCFunction)dbgdata_ReadBusData, METH_VARARGS, 
     "ReadBusData(BusDataType, BusNumber, SlotNumber, Offset, Size) -> data\n"
     "  Reads data from a system bus."
    },
    {"WriteBusData", 
     (PyCFunction)dbgdata_WriteBusData, METH_VARARGS, 
     "WriteBusData(BusDataType, BusNumber, SlotNumber, Offset, data) -> bytes written\n"
     "  Writes data to a system bus."
    },
    {"ReadControl", 
     (PyCFunction)dbgdata_ReadControl, METH_VARARGS, 
     "ReadControl(Processor, Offset, Size) -> data\n"
     "  Reads implementation specific system data."
    },
    {"WriteControl", 
     (PyCFunction)dbgdata_WriteControl, METH_VARARGS, 
     "WriteControl(Processor, Offset, data) -> bytes written\n"
     "  Writes implementation specific system data."
    },
    {"ReadDebuggerData", 
     (PyCFunction)dbgdata_ReadDebuggerData, METH_VARARGS, 
     "ReadDebuggerData(index) -> data\n"
     "  Returns information about the target that the debugger engine has queried\n"
     "  or determined during the current session.  The available information includes\n"
     "  the locations of certain key target kernel locations, specific status values,\n"
     "  and a number of other things.\n"
     "  Index:\n"
     "    DEBUG_DATA_KernBase\n"
     "    DEBUG_DATA_BreakpointWithStatusAddr\n"
     "    DEBUG_DATA_SavedContextAddr\n"
     "    DEBUG_DATA_KiCallUserModeAddr\n"
     "    DEBUG_DATA_KeUserCallbackDispatcherAddr\n"
     "    DEBUG_DATA_PsLoadedModuleListAddr\n"
     "    DEBUG_DATA_PsActiveProcessHeadAddr\n"
     "    DEBUG_DATA_PspCidTableAddr\n"
     "    DEBUG_DATA_ExpSystemResourcesListAddr\n"
     "    DEBUG_DATA_ExpPagedPoolDescriptorAddr\n"
     "    DEBUG_DATA_ExpNumberOfPagedPoolsAddr\n"
     "    DEBUG_DATA_KeTimeIncrementAddr\n"
     "    DEBUG_DATA_KeBugCheckCallbackListHeadAddr\n"
     "    DEBUG_DATA_KiBugcheckDataAddr\n"
     "    DEBUG_DATA_IopErrorLogListHeadAddr\n"
     "    DEBUG_DATA_ObpRootDirectoryObjectAddr\n"
     "    DEBUG_DATA_ObpTypeObjectTypeAddr\n"
     "    DEBUG_DATA_MmSystemCacheStartAddr\n"
     "    DEBUG_DATA_MmSystemCacheEndAddr\n"
     "    DEBUG_DATA_MmSystemCacheWsAddr\n"
     "    DEBUG_DATA_MmPfnDatabaseAddr\n"
     "    DEBUG_DATA_MmSystemPtesStartAddr\n"
     "    DEBUG_DATA_MmSystemPtesEndAddr\n"
     "    DEBUG_DATA_MmSubsectionBaseAddr\n"
     "    DEBUG_DATA_MmNumberOfPagingFilesAddr\n"
     "    DEBUG_DATA_MmLowestPhysicalPageAddr\n"
     "    DEBUG_DATA_MmHighestPhysicalPageAddr\n"
     "    DEBUG_DATA_MmNumberOfPhysicalPagesAddr\n"
     "    DEBUG_DATA_MmMaximumNonPagedPoolInBytesAddr\n"
     "    DEBUG_DATA_MmNonPagedSystemStartAddr\n"
     "    DEBUG_DATA_MmNonPagedPoolStartAddr\n"
     "    DEBUG_DATA_MmNonPagedPoolEndAddr\n"
     "    DEBUG_DATA_MmPagedPoolStartAddr\n"
     "    DEBUG_DATA_MmPagedPoolEndAddr\n"
     "    DEBUG_DATA_MmPagedPoolInformationAddr\n"
     "    DEBUG_DATA_MmPageSize\n"
     "    DEBUG_DATA_MmSizeOfPagedPoolInBytesAddr\n"
     "    DEBUG_DATA_MmTotalCommitLimitAddr\n"
     "    DEBUG_DATA_MmTotalCommittedPagesAddr\n"
     "    DEBUG_DATA_MmSharedCommitAddr\n"
     "    DEBUG_DATA_MmDriverCommitAddr\n"
     "    DEBUG_DATA_MmProcessCommitAddr\n"
     "    DEBUG_DATA_MmPagedPoolCommitAddr\n"
     "    DEBUG_DATA_MmExtendedCommitAddr\n"
     "    DEBUG_DATA_MmZeroedPageListHeadAddr\n"
     "    DEBUG_DATA_MmFreePageListHeadAddr\n"
     "    DEBUG_DATA_MmStandbyPageListHeadAddr\n"
     "    DEBUG_DATA_MmModifiedPageListHeadAddr\n"
     "    DEBUG_DATA_MmModifiedNoWritePageListHeadAddr\n"
     "    DEBUG_DATA_MmAvailablePagesAddr\n"
     "    DEBUG_DATA_MmResidentAvailablePagesAddr\n"
     "    DEBUG_DATA_PoolTrackTableAddr\n"
     "    DEBUG_DATA_NonPagedPoolDescriptorAddr\n"
     "    DEBUG_DATA_MmHighestUserAddressAddr\n"
     "    DEBUG_DATA_MmSystemRangeStartAddr\n"
     "    DEBUG_DATA_MmUserProbeAddressAddr\n"
     "    DEBUG_DATA_KdPrintCircularBufferAddr\n"
     "    DEBUG_DATA_KdPrintCircularBufferEndAddr\n"
     "    DEBUG_DATA_KdPrintWritePointerAddr\n"
     "    DEBUG_DATA_KdPrintRolloverCountAddr\n"
     "    DEBUG_DATA_MmLoadedUserImageListAddr\n"
     "    DEBUG_DATA_PaeEnabled\n"
     "    DEBUG_DATA_SharedUserData\n"
     "    DEBUG_DATA_ProductType\n"
     "    DEBUG_DATA_SuiteMask\n"
     "    DEBUG_DATA_DumpWriterStatus\n"
     "    DEBUG_DATA_NtBuildLabAddr\n"
     "    DEBUG_DATA_KiNormalSystemCall\n"
     "    DEBUG_DATA_KiProcessorBlockAddr\n"
     "    DEBUG_DATA_MmUnloadedDriversAddr\n"
     "    DEBUG_DATA_MmLastUnloadedDriverAddr\n"
     "    DEBUG_DATA_MmTriageActionTakenAddr\n"
     "    DEBUG_DATA_MmSpecialPoolTagAddr\n"
     "    DEBUG_DATA_KernelVerifierAddr\n"
     "    DEBUG_DATA_MmVerifierDataAddr\n"
     "    DEBUG_DATA_MmAllocatedNonPagedPoolAddr\n"
     "    DEBUG_DATA_MmPeakCommitmentAddr\n"
     "    DEBUG_DATA_MmTotalCommitLimitMaximumAddr\n"
     "    DEBUG_DATA_CmNtCSDVersionAddr\n"
     "    DEBUG_DATA_MmPhysicalMemoryBlockAddr\n"
     "    DEBUG_DATA_MmSessionBase\n"
     "    DEBUG_DATA_MmSessionSize\n"
     "    DEBUG_DATA_MmSystemParentTablePage\n"
     "    DEBUG_DATA_MmVirtualTranslationBase\n"
     "    DEBUG_DATA_OffsetKThreadNextProcessor\n"
     "    DEBUG_DATA_OffsetKThreadTeb\n"
     "    DEBUG_DATA_OffsetKThreadKernelStack\n"
     "    DEBUG_DATA_OffsetKThreadInitialStack\n"
     "    DEBUG_DATA_OffsetKThreadApcProcess\n"
     "    DEBUG_DATA_OffsetKThreadState\n"
     "    DEBUG_DATA_OffsetKThreadBStore\n"
     "    DEBUG_DATA_OffsetKThreadBStoreLimit\n"
     "    DEBUG_DATA_SizeEProcess\n"
     "    DEBUG_DATA_OffsetEprocessPeb\n"
     "    DEBUG_DATA_OffsetEprocessParentCID\n"
     "    DEBUG_DATA_OffsetEprocessDirectoryTableBase\n"
     "    DEBUG_DATA_SizePrcb\n"
     "    DEBUG_DATA_OffsetPrcbDpcRoutine\n"
     "    DEBUG_DATA_OffsetPrcbCurrentThread\n"
     "    DEBUG_DATA_OffsetPrcbMhz\n"
     "    DEBUG_DATA_OffsetPrcbCpuType\n"
     "    DEBUG_DATA_OffsetPrcbVendorString\n"
     "    DEBUG_DATA_OffsetPrcbProcessorState\n"
     "    DEBUG_DATA_OffsetPrcbNumber\n"
     "    DEBUG_DATA_SizeEThread\n"
     "    DEBUG_DATA_KdPrintCircularBufferPtrAddr\n"
     "    DEBUG_DATA_KdPrintBufferSizeAddr"
    },
    {"ReadHandleData", 
     (PyCFunction)dbgdata_ReadHandleData, METH_VARARGS, 
     "ReadHandleData(handle, DataType) -> handle information\n"
     "  Retrieves information about a system object specified by a system handle.\n"
     "  DataType:\n"
     "    DEBUG_HANDLE_DATA_TYPE_TYPE_NAME\n"
     "    DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME\n"
     "    DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT"
    },
// ReadImageNtHeaders
    {"ReadIo", 
     (PyCFunction)dbgdata_ReadIo, METH_VARARGS, 
     "ReadIo(InterfaceType, BusNumber, AddressSpace, Offset, Size) -> data\n"
     "  Reads from the system and bus I/O memory."
    },
    {"WriteIo", 
     (PyCFunction)dbgdata_WriteIo, METH_VARARGS, 
     "WriteIo(InterfaceType, BusNumber, AddressSpace, Offset, data) -> bytes written\n"
     "  Writes to the system and bus I/O memory."
    },
// CheckLowMemory
    {"ReadMsr", 
     (PyCFunction)dbgdata_ReadMSR, METH_VARARGS, 
     "ReadMsr(msr) -> value\n"
     "  Reads a specified model-specific register (MSR)."
    },
    {"WriteMsr", 
     (PyCFunction)dbgdata_WriteMSR, METH_VARARGS, 
     "WriteMsr(msr, value)\n"
     "  Writes a value to the specified MSR."
    },
    /*
    {"GetOffsetInformation", (PyCFunction)dbgdata_GetOffsetInformation,
        METH_VARARGS, ""},
    */
    {"FillPhysical", 
     (PyCFunction)dbgdata_FillPhysical, METH_VARARGS, 
     "FillPhysical(Start, Size, Pattern) -> filled\n"
     "  Writes a pattern of bytes to the target's physical memory.  The pattern is\n"
     "  written repeatedly until the specified memory range is filled."
    },
    {"ReadPhysical", 
     (PyCFunction)dbgdata_ReadPhysical, METH_VARARGS, 
     "ReadPhysical(Offset, Size) -> data\n"
     "  Reads the target's memory from the specified physical address."
    },
    {"ReadPhysical2", 
     (PyCFunction)dbgdata_ReadPhysical2, METH_VARARGS, 
     "ReadPhysical2(Offset, Flags, Size) -> data\n"
     "  Reads the target's memory from the specified physical address.\n"
     "  Flags:\n"
     "    DEBUG_PHYSICAL_DEFAULT\n"
     "    DEBUG_PHYSICAL_CACHED\n"
     "    DEBUG_PHYSICAL_UNCACHED\n"
     "    DEBUG_PHYSICAL_WRITE_COMBINED"
    },
    {"WritePhysical", 
     (PyCFunction)dbgdata_WritePhysical, METH_VARARGS, 
     "WritePhysical(Offset, data) -> bytes written\n"
     "  Writes data to the specified physical address in the target's memory."
    },
    {"WritePhysical2", 
     (PyCFunction)dbgdata_WritePhysical2, METH_VARARGS, 
     "WritePhysical2(Offset, Flags, data) -> bytes written\n"
     "  Writes data to the specified physical address in the target's memory.\n"
     "  Flags:\n"
     "    DEBUG_PHYSICAL_DEFAULT\n"
     "    DEBUG_PHYSICAL_CACHED\n"
     "    DEBUG_PHYSICAL_UNCACHED\n"
     "    DEBUG_PHYSICAL_WRITE_COMBINED"
    },
// ReadProcessorSystemData
// ReadTagged
// EndEnumTagged
// StartEnumTagged
// GetNextTagged
    {"FillVirtual", 
     (PyCFunction)dbgdata_FillVirtual, METH_VARARGS, 
     "FillVirtual(Start, Size, Pattern) -> filled\n"
     "  Writes a pattern of bytes to the target's virtual memory.  The pattern is\n"
     "  written repeatedly until the specified memory range is filled."
    },
    {"QueryVirtual", 
     (PyCFunction)dbgdata_QueryVirtual, METH_VARARGS, 
     "QueryVirtual(Offset) -> MEMORY_BASIC_INFORMATION\n"
     "  Provides information about the specified pages in the target's virtual\n"
     "  address space."
    },
    {"ReadVirtual", 
     (PyCFunction)dbgdata_ReadVirtual, METH_VARARGS, 
     "ReadVirtual(Offset, Size) -> data\n"
     "  Reads memory from the target's virtual address space."
    },
    {"SearchVirtual", 
     (PyCFunction)dbgdata_SearchVirtual, METH_VARARGS, 
     "SearchVirtual(Offset, Length, Pattern, PatternGranularity=1) -> offset\n"
     "  Searches the target's virtual memory for a specified pattern of bytes."
    },
// SearchVirtual2
    {"WriteVirtual", 
     (PyCFunction)dbgdata_WriteVirtual, METH_VARARGS, 
     "WriteVirtual(Offset, Data) -> bytes written\n"
     "  Writes data to the target's virtual address space."
    },
    {"ReadVirtualUncached", 
     (PyCFunction)dbgdata_ReadVirtualUncached, METH_VARARGS, 
     "ReadVirtualUncached(Offset, Size) -> data\n"
     "  Reads memory from the target's virtual address space."
    },
    {"WriteVirtualUncached", 
     (PyCFunction)dbgdata_WriteVirtualUncached, METH_VARARGS, 
     "WriteVirtualUncached(Offset, data) -> bytes written\n"
     "  Writes data to the target's virtual address space."
    },
// ReadPointersVirtual
// WritePointersVirtual
// ReadMultiByteStringVirtual
// ReadMultibyteStringVirtualWide
// ReadUnicodeStringVirtual
// ReadUnicodeStringVirtualWide
    {"VirtualToPhysical", 
     (PyCFunction)dbgdata_VirtualToPhysical, METH_VARARGS, 
     "VirtualToPhysical(Virtual) -> physical\n"
     "  Translates a location in the target's virtual address space into a\n"
     "  physical memory address."
    },
// GetVirtualTranslationPhysicalOffsets
    {"GetNextDifferentlyValidOffsetVirtual", 
     (PyCFunction)dbgdata_GetNextDifferentlyValidOffsetVirtual, METH_VARARGS, 
     "GetNextDifferentlyValidOffsetVirtual(Offset) -> next offset\n"
     "  Returns the offset to the next address whose validity might be different\n"
     "  from the validity of the specified address."
    },
    {"GetValidRegionVirtual", 
     (PyCFunction)dbgdata_GetValidRegionVirtual, METH_VARARGS, 
     "GetValidRegionVirtual(Base, Size) -> (Valid base, Valid Size)\n"
     "  Locates the first valid region of memory in a specified memory range."
    },
    {NULL, NULL}
};


PyTypeObject PyDebugDataSpacesType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydbgeng.DebugDataSpaces", /*tp_name*/
    sizeof(PyDebugDataSpacesObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dbgdata_dealloc, /*tp_dealloc*/
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
    "DebugDataSpaces objects", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    offsetof(PyDebugDataSpacesObject, weakreflist), /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyDebugDataSpaces_methods, /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    dbgdata_init,              /* tp_init */
    PyType_GenericAlloc,       /* tp_alloc */
    dbgdata_new,               /* tp_new */
    PyObject_Del,              /* tp_free */
};

