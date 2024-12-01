from collections.abc import Mapping, Callable

from pybag.dbgeng import core as DbgEng
from pybag.dbgeng import exception
from pybag.dbgeng.idebugbreakpoint import DebugBreakpoint


class Breakpoints(Mapping, Callable):
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
        bpid = rawbp.GetId()

        try:
            (count,bpfn) = self.__getitem__(bpid)
        except KeyError:
            count = 0
            bpfn = None
        if bpfn:
            ret = bpfn(rawbp)
        else:
            ret = DbgEng.DEBUG_STATUS_NO_CHANGE

        if rawbp.GetFlags() & DbgEng.DEBUG_BREAKPOINT_ONE_SHOT:
            self._remove_stale(bpid)

        count -= 1
        if count == 0:
            self._remove(rawbp, bpid)

        return ret


    def set(self, expr, handler=None, type=None, windbgcmd=None, oneshot=False,
                  passcount=None, threadid=None, size=None, access=None,
                  count=0xffffffff):
        existingid = self.find(expr)
        if existingid != -1:
            # Just enable
            self.enable(existingid)
            return
        else:
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
            if type == DbgEng.DEBUG_BREAKPOINT_DATA:
                bp.SetDataParameters(size, access)
        if passcount is not None:
            bp.SetPassCount(passcount)
        if threadid:
            bp.SetMatchThreadId(threadid)
        if oneshot:
            bp.AddFlags(DbgEng.DEBUG_BREAKPOINT_ONE_SHOT)
        self._bp[id] = (count,handler)
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

    def find_id(self, fid):
        for bpid in self:
            if bpid == fid:
                return fid
        return -1

    def find_expr(self, expr):
        ids = [bpid for bpid in self]
        for bpid in ids:
            try:
                bp = self._control.GetBreakpointById(bpid)
            except exception.E_NOINTERFACE_Error:
                self._remove_stale(bpid)
                continue

            bpexpr = bp.GetOffsetExpression()
            print(bpexpr)
            #expr = self.get_name_by_offset(bp.GetOffset())
            if bpexpr == expr:
                return bpid
        return -1

    def find_offset(self, offset):
        ids = [bpid for bpid in self]
        for bpid in ids:
            try:
                bp = self._control.GetBreakpointById(bpid)
            except exception.E_NOINTERFACE_Error:
                self._remove_stale(bpid)
                continue

            bpoff = bp.GetOffset()
            if bpoff == offset:
                return bpid
        return -1


    def find(self, expr):
        bpid = self.find_expr(expr)
        if bpid == -1:
            bpid = self.find_offset(expr)
        return bpid
