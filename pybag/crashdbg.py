from .pydbg import DebuggerBase


class CrashDbg(DebuggerBase):
    def __init__(self):
        super().__init__(client=None, standalone=True)

    def load_dump(self, name):
        self._client.OpenDumpFile(name)
        self.go()

