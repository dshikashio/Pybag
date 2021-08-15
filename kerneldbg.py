from .pydbg import DebuggerBase, DbgEng

class KernelDbg(DebuggerBase):
    def __init__(self):
        super().__init__(client=None, standalone=True)

    def attach(self, connect_string):
        self.events.engine_state(verbose=True)
        self.events.debuggee_state(verbose=True)
        self.events.session_status(verbose=True)
        self.events.system_error(verbose=True)
        self._control.SetEngineOptions(DbgEng.DEBUG_ENGOPT_INITIAL_BREAK)

        # net:port=55555,key=d.b.g.it
        # com:pipe,port=\\.\pipe\dbgtarget,reconnect,resets=0,baud=115200

        self._client.AttachKernel(
            DbgEng.DEBUG_ATTACH_KERNEL_CONNECTION,
            connect_string)
        self.wait()

    def detach(self):
        pass