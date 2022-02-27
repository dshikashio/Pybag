import collections

from . import core as DbgEng
from . import exception
from .idebugbreakpoint import DebugBreakpoint


class Breakpoints(collections.Mapping, collections.Callable):
    def __init__(self, DebugControlObj):
        self._control = DebugControlObj
        self._bp = {}

    def __len__(self):
        return len(self._bp)

    def __iter__(self):
        return iter(self._bp.keys())

    def __contains__(self, id):
        return id in self._bp

    def __getitem__(self, id):
        return self._bp[id]

    def __call__(self, *args):
        rawbp = DebugBreakpoint(args[0])
        try:
            bpfn = self.__getitem__(rawbp.GetId())
        except KeyError:
            bpfn = None
        if bpfn:
            ret = bpfn(rawbp)
        else:
            ret = DbgEng.DEBUG_STATUS_NO_CHANGE

        if rawbp.GetFlags() & DbgEng.DEBUG_BREAKPOINT_ONE_SHOT:
            self._bp.pop(rawbp.GetId(), None)
            self._control.RemoveBreakpoint(rawbp)

        return ret

    def set(self, expr, handler=None, type=None, windbgcmd=None, oneshot=False,
                  passcount=None, threadid=None, size=None, access=None):
        # XXX - would be nice to check if bp with expr already exists
        # if so, then enable existing, else add
        if type is None:
            type = DbgEng.DEBUG_BREAKPOINT_CODE
        if isinstance(expr, int):
            expr = b"0x%x" % expr
        elif isinstance(expr, str):
            expr = expr.encode()
        bp = self._control.AddBreakpoint(type)
        id = bp.GetId()
        bp.SetOffsetExpression(expr)
        if windbgcmd:
            pass
        if passcount is not None:
            bp.SetPassCount(passcount)
        if threadid:
            bp.SetMatchThreadId(threadid)
        if type == DbgEng.DEBUG_BREAKPOINT_DATA:
            bp.SetDataParameters(size, access)
        if oneshot:
            bp.AddFlags(DbgEng.DEBUG_BREAKPOINT_ONE_SHOT)
        self._bp[id] = handler
        if not bp.GetFlags() & DbgEng.DEBUG_BREAKPOINT_DEFERRED:
            self.enable(id)
        return id

    def enable(self, id):
        bp = self._control.GetBreakpointById(id)
        bp.AddFlags(DbgEng.DEBUG_BREAKPOINT_ENABLED)

    def disable(self, id):
        bp = self._control.GetBreakpointById(id)
        bp.RemoveFlags(DbgEng.DEBUG_BREAKPOINT_ENABLED)

    def _remove(self, bp, id):
        self._control.RemoveBreakpoint(bp)
        self._bp.pop(id, None)

    def _remove_stale(self, id):
        self._bp.pop(id, None)

    def remove(self, id):
        bp = self._control.GetBreakpointById(id)
        self._remove(bp, id)

    def enable_all(self):
        for bpid in self:
            self.enable(bpid)

    def disable_all(self):
        for bpid in self:
            self.disable(bpid)

    def remove_all(self):
        for bpid in self:
            self.remove(bpid)

