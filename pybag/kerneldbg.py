from .pydbg import DebuggerBase, DbgEng

class KernelDbg(DebuggerBase):
    def __init__(self):
        super().__init__(client=None, standalone=True)

    def attach(self, connect_string, initial_break=False):
        if initial_break:
            self._control.SetEngineOptions(DbgEng.DEBUG_ENGINITIAL_BREAK)

        # net:port=55555,key=d.b.g.it
        # com:pipe,port=\\.\pipe\dbgtarget,reconnect,resets=0,baud=115200

        self._worker_wait('AttachKernel', DbgEng.WAIT_INFINITE, (connect_string,))
        self.wait()

    def detach(self):
        self._client.EndSession(DbgEng.DEBUG_END_ACTIVE_DETACH)

# ReadProcessorSystemData
# VirtualToPhysical
# ReadHandleData
# ReadUnicodeStringVirtual
# 