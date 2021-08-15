from .pydbg import DebuggerBase, DbgEng

class UserDbg(DebuggerBase):
    def __init__(self):
        super().__init__(client=None, standalone=True)

    def proc_list(self):
        procs = []
        for id in self._client.GetRunningProcessSystemIds():
            try:
                name,desc = self._client.GetRunningProcessDescription(id)
            except WindowsError:
                name,desc = '<UKNOWN>',''
            proc = (id, name, desc)
            procs.append(proc)
        return procs

    def ps(self):
        """ps() -> Print a list of running processes"""
        for id,name,desc in self.proc_list():
            print(" %6d %4x\n      %s\n      %s" % (id, id, name, desc))

    def pids_by_name(self, proc_name):
        """pids_by_name(name) -> return a list of pids matching name"""
        procs = []
        for id,name,desc in self.proc_list():
            if name.lower().endswith(proc_name.lower()):
                procs.append((id,name,desc))
        return procs

    def create(self, path, initial_break=True):
        """create(path, initial_break=True) -> Create a new process to debug"""
        self._client.CreateProcess(path, DbgEng.DEBUG_PROCESS)
        if initial_break:
            self._control.AddEngineOptions(DbgEng.DEBUG_ENGINITIAL_BREAK)
        self.wait()

    def attach(self, pid, initial_break=True):
        """attach(pid, flags=DEBUG_ATTACH_DEFAULT) -> attach to pid"""
        self._client.AttachProcess(pid, DbgEng.DEBUG_ATTACH_DEFAULT)
        if initial_break:
            self._control.AddEngineOptions(DbgEng.DEBUG_ENGINITIAL_BREAK)
        self.wait()

    def reattach(self, pid):
        """reattach(pid) -> reattach to pid"""
        #self._control.RemoveEngineOptions(pydbgeng.DEBUG_ENGOPT_INITIAL_BREAK)
        self.attach(pid, DbgEng.DEBUG_ATTACH_EXISTING)
        self.wait()

    def detach(self):
        """detach() -> detach from current process"""
        self._client.DetachCurrentProcess()

    def abandon(self):
        """abandon() -> abandon the current process"""
        #Not working
        #self._client.AbandonCurrentProcess()
        pass

    def terminate(self):
        """terminate() -> terminate the current process"""
        self._client.TerminateCurrentProcess()
        self.wait()

    @property
    def pid(self):
        return self._systems.GetCurrentProcessSystemId()

    def handoff(self):
        # XXX - Fixme
        # -y symbolpath
        # -srcpath sourcepath
        #prog = r'c:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64\windbg.exe'
        #pid = self.pid
        #print "PID ", pid
        #self.abandon()
        #subprocess.Popen([prog, '-c', '~*m;g', '-pe', '-p', '%d' % pid])
        #time.sleep(2)
        #sys.exit(0)
        pass
