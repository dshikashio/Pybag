import socket
import struct
import sys

NOID = 0xffffffffffffffff
procs = {}
proc_count = 0

def dump(buf):
    for i,b in enumerate(buf):
        if i % 16 == 0:
            print ""
        print "%02x" % ord(b),
    print ""

class IOException(Exception):
    pass

class Msg(object):
    def __init__(self):
        self.op = None
        self.addr = None
        self.bp = None
        self.cnt = None
        self.cid = None
        self.flags = None
        self.fd = None
        self.reg = Reg()
        self.bytes = ""
        self.argv = []
        self.err = ""

class Reg(object):
    def __init__(self):
        self.eflags = 0
        self.rbx = 0
        self.rcx = 0
        self.rdx = 0
        self.rsi = 0
        self.rdi = 0
        self.rbp = 0
        self.rax = 0
        self.rip = 0
        self.rsp = 0
        self.r8 = 0
        self.r9 = 0
        self.r10 = 0
        self.r11 = 0
        self.r12 = 0
        self.r13 = 0
        self.r14 = 0
        self.r15 = 0

    def unpack(self, data, offset):
        (self.eflags, self.rbx, self.rcx, self.rdx,
        self.rsi, self.rdi, self.rbp, self.rax,
        self.rip, self.rsp, self.r8, self.r9,
        self.r10, self.r11, self.r12, self.r13,
        self.r14, self.r15) = struct.unpack_from("<" + "Q"*18, data, offset)

    def pack(self):
        return struct.pack("<" + "Q"*18,
            self.eflags, self.rbx, self.rcx, self.rdx,
            self.rsi, self.rdi, self.rbp, self.rax,
            self.rip, self.rsp, self.r8, self.r9,
            self.r10, self.r11, self.r12, self.r13,
            self.r14, self.r15)

MSG_OP = (
        "Rerror",
        "Tlaunch",
        "Rlaunch",
        "Tcont",
        "Rcont",

        "Tstop",
        "Rstop",
        "Tkill",
        "Rkill",
        "Tsetbrk",

        "Rsetbrk",
        "Tclrbrk",
        "Rclrbrk",
        "Tread",
        "Rread",

        "Abrk",
        "Aobit",
        "Aack",
        "Tcleave",
        "Rcleave",

        "Twrite",
        "Rwrite",
        "Tsetregs",
        "Rsetregs",
        "Tsetsnap",

        "Rsetsnap",
        "Asnap",
        "Tattach",
        "Rattach",
        "Ttest",

        "Rtest",
        "Tstep",
        "Rstep",
        "Aclone",
        "Tfread",

        "Rfread",
        "Tfwrite",
        "Rfwrite")

def xwrite(fd, buf):
    fd.sendall(buf)

def writexmsg(fd, buf):
    print "writexmsg len : %d" % len(buf)
    xwrite(fd, struct.pack("<Q", len(buf)))
    xwrite(fd, buf)

def writemsg(fd, m):
    print "Write msg OP : %s" % m.op
    op = MSG_OP.index(m.op)
    if m.op == 'Rerror':
        buf = struct.pack("<BQs", op, m.id, m.err)
    elif m.op == 'Tlaunch':
        buf = struct.pack("<BQ", op, len(m.argv))
        for i in range(len(m.argv)):
            buf += struct.pack("%ds" % (len(m.argv[i])+1), m.argv[i])
    elif m.op in ('Rlaunch', 'Rstop'):
        buf = struct.pack("<BQ", op, m.id)
        buf += m.reg.pack()
    elif m.op in ('Tcont', 'Tstep', 'Rwrite', 'Rattach',
                  'Rcont', 'Rclrbrk', 'Rkill', 'Rsetregs',
                  'Rstep', 'Aobit', 'Aack'):
        buf = struct.pack("<BQ", op, m.id)
    elif m.op == 'Tsetbrk':
        buf = struct.pack("<BQQ", op, m.id, m.addr)
    elif m.op in ('Tclrbrk', 'Rsetbrk', 'Rsetsnap'):
        buf = struct.pack("<BQQ", op, m.id, m.bp)
    elif m.op == 'Abrk':
        buf = struct.pack("<BQQ", op, m.id, m.bp)
        buf += struct.reg.pack()
    elif m.op == 'Rread':
        buf = struct.pack("<BQ", op, m.id)
        buf += m.bytes[:m.cnt]
    elif m.op == 'Tfread':
        buf = struct.pack("<BQQQ", op, m.id, m.fd, m.cnt)
    elif m.op == 'Tfwrite':
        buf = struct.pack("<BQQQ", op, m.id, m.fd, m.cnt)
        buf += m.bytes[:m.cnt]
    elif m.op == 'Rfread':
        buf = struct.pack("<BQQ", op, m.id, m.cnt)
        buf += m.bytes[:m.cnt]
    elif m.op == 'Rfwrite':
        buf = struct.pack("<BQQ", op, m.id, m.cnt)
    elif m.op == 'Asnap':
        buf = struct.pack("<BQQQ", op, m.id, m.bp, m.cid)
    elif m.op == 'Aclone':
        buf = struct.pack("<BQQ", op, m.id, m.cid)
    elif m.op == 'Rcleave':
        buf = struct.pack("<BQQ", op, m.id, m.cid)
    elif m.op == 'Ttest':
        buf = struct.pack("<BQ", op, m.cnt)
        buf += m.bytes[:m.cnt]
    elif m.op == 'Rtest':
        buf = struct.pack("<BQ", op, m.cnt)
    else:
        raise IOException('writemsg: Unimplemented op code %s' % m.op)
    writexmsg(fd, buf)

def eproto(fd):
    m = Msg()
    m.op = 'Rerror'
    m.id = 0xffffffffffffffff
    m.err = "bad protocol"
    writemsg(fd, m)

def xread(fd, size):
    buf = []
    nrecv = 0
    while nrecv < size:
        data = fd.recv(size - nrecv)
        buf.append(data)
        nrecv += len(data)
    return ''.join(buf)


def readxmsg(fd):
    msglen = fd.recv(8)
    if len(msglen) != 8:
        raise IOException('readxmsg got bad length')
    xmlen = struct.unpack("<Q", msglen)[0]
    print "readxmsg: Len : %d" % xmlen
    return xread(fd, xmlen)

def readmsg(fd):
    msg = Msg()
    xm = readxmsg(fd)
    dump(xm)
    msg.op = ord(xm[0])
    offset = 1

    try:
        msg.op = MSG_OP[msg.op]
    except IndexError:
        print "Invalid msg op %d" % msg.op
        return None

    print "MSG OP : %s" % msg.op

    if msg.op == 'Tlaunch':
        narg = struct.unpack_from("<Q", xm, offset)[0]
        offset += struct.calcsize("Q")
        print narg
        for i in range(narg):
            arg = struct.unpack_from("s", xm, offset)[0]
            offset += len(arg) + 1
            msg.argv.append(arg)
    elif msg.op == 'Rlaunch':
        msg.id = struct.unpack_from("<Q", xm, offset)[0]
        offset += struct.calcsize("Q")
        msg.reg.unpack(xm, offset)
    elif msg.op in ('Tattach', 'Tcont', 'Tstop', 'Tkill',
                    'Tcleave', 'Tstep', 'Rattach', 'Rcont',
                    'Rclrbrk', 'Rstep', 'Aack', 'Aobit'):
        msg.id = struct.unpack_from("<Q", xm, offset)[0]
    elif msg.op in ('Tsetbrk', 'Tsetsnap'):
        msg.id, msg.addr = struct.unpack_from("<QQ", xm, offset)
    elif msg.op in ('Rsetbrk', 'Rsetsnap'):
        msg.id, msg.bp = struct.unpack_from("<QQ", xm, offset)
    elif msg.op == 'Tclrbrk':
        msg.id, msg.bp = struct.unpack_from("<QQ", xm, offset)
    #elif msg.op == 'Abrk':
        #msg.id, msg.bp, msg.reg = ""
    elif msg.op == 'Tread':
        msg.id, msg.addr, msg.cnt = struct.unpack_from("<QQQ", xm, offset)
    elif msg.op == 'Twrite':
        msg.id, msg.addr, msg.cnt = struct.unpack_from("<QQQ", xm, offset)
        offset += struct.calcsize("QQQ")
        msg.bytes = struct.unpack_from("%ds" % msg.cnt, xm, offset)
    elif msg.op == 'Tfread':
        msg.id, msg.fd,  msg.cnt = struct.unpack_from("<QQQ", xm, offset)
    elif msg.op == 'Tfwrite':
        msg.id, msg.fd, msg.cnt = struct.unpack_from("<QQQ", xm, offset)
        offset += struct.calcsize("QQQ")
        msg.bytes = struct.unpack_from("%ds" % msg.cnt, xm, offset)
    elif msg.op == 'Rfread':
        msg.id, msg.cnt = struct.unpack_from("<QQ", xm, offset)
        offset += struct.calcsize("QQ")
        msg.bytes = struct.unpack_from("%ds" % msg.cnt, xm, offset)
    elif msg.op == 'Rfwrite':
        msg.id, msg.cnt = struct.unpack_from("<QQ", xm, offset)
    #elif msg.op == 'Tsetregs':
    elif msg.op == 'Ttest':
        msg.cnt = struct.unpack_from("<Q", xm, offset)
        offset += struct.calcsize("Q")
        msg.bytes = struct.unpack_from("%ds" % msg.cnt, xm, offset)
    elif msg.op == 'Rtest':
        msg.cnt = struct.unpack_from("<Q", xm, offset)
    elif msg.op == 'Rerror':
        msg.id = struct.unpack_from("<Q", xm, offset)[0]
        msg.err = xm[offset:]
    else:
        return None
        
    return msg

PROC_STATES = ('Noprocess', 'Running', 'Stopped',
               'Atbreak', 'Dead')

class Proc(object):
    def __init__(self):
        self.id = -1
        self.pid = -1
        self.state = None
        self.bp = []
        self.regs = Regs()
        self.curbp = None
        # this should have it's own dbgclient instance

class Breakpoint(object):
    def __init__(self):
        self.addr = 0
        self.text = 0
        self.issnap = False

def newproc():
    global procs
    global proc_count
    p = Proc()
    proc_count += 1
    procs[proc_count] = p
    p.id = proc_count
    return p

def editbrk(proc, bp, set):
    idx = bp->addr % 4

    # text = read mem from proc.pid, bp.addr & (~3), 

    if set:
        bp.text = text[idx]
        text[idx] = "\xcc"
    else:
        text[idx] = bp->text

    # write mem = text @ bp.addr & (~3)

    # return False if any failures

    return True

def step(proc):
    # ask dbg to single step with proc.pid
    # return false if fails

    proc.state = 'Stopped'
    return True

def clrbrk(proc, bp):
    return editbrk(proc, bp, False)

def setbrk(proc, bp):
    return editbrk(proc, bp, True)

def stepbrk(proc):
    if not clrbrk(proc, proc.curbp):
        return False
    if not step(proc):
        return False
    if not setbrk(proc, proc.curbp)
        return False
    return True

def domsg(fd, msg):
    global procs
    rep = Msg()
    if msg.op == 'Tlaunch':
        print "Launching new process"

        # start new process with msg.argv
        pid = 0
        # set up some local bookkeeping
        proc = newproc()
        proc.pid = pid
        proc.state = 'Stopped'
        # Read regs into proc.regs
        # copy proc.regs into rep.regs

        # if any errors
        rep.op = 'Rerror'
        rep.id = NOID
        rep.err = "Bad process"
        writemsg(fd, rep)
        return

        rep.op = 'Rlaunch'
        rep.id = proc.id
        writemsg(fd, rep)
    elif msg.op == 'Tattach':
        print "Attaching to : %d" % msg.id
        pid = msg.id
        # attach to process with pid
        
        proc = newproc()
        proc.pid = pid
        proc.state = 'Stopped'

        rep.op = 'Rattach'
        rep.id = p.id
        writemsg(fd, rep)
    elif msg.op == 'Tcont':
        if msg.id not in procs:
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return

        proc = procs[msg.id]
        if proc.state == 'Noprocess':
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return
        elif proc.state == 'Stopped':
            # tell process to continue
            proc.state = 'Running'
        elif proc.state == 'Atbreak':
            if not stepbrk(proc):
                rep.op = 'Rerror'
                rep.id = msg.id
                rep.err = 'stepbrk failed'
                writemsg(fd, rep)
                return
            # tell process to continue
            proc.state = 'Running'

        rep.op = 'Rcont'
        rep.id = msg.id
        writemsg(fd, rep)
    elif msg.op == 'Tstop':
        if msg.id not in procs:
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return

        proc = procs[msg.id]
        if proc.state == 'Noprocess':
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return
        if proc.state == 'Running':
            xstop(proc)
        # copy proc.regs to rep.reg
        rep.op = 'Rstop'
        rep.id = msg.id
        writemsg(fd, rep)
    elif msg.op == 'Tkill':
        if msg.id not in procs:
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return

        proc = procs[msg.id]
        if proc.state == 'Noprocess':
            rep.op = 'Rerror'
            rep.id = msg.id
            rep.err = 'bad id'
            writemsg(fd, rep)
            return
        elif proc.state == 'Running':
            # kill (proc.pid)
        elif proc.state in ('Atbreak', 'Stopped'):
            # ask debugger to kill proc.pid

        proc.state = 'Noprocess'
        rep.op = 'Rkill'
        rep.id = msg.id
        writemsg(fd, rep)
    elif msg.op == 'Tcleave':
        # Not supported
        eproto(fd)
    elif msg.op in ('Tsetbrk', 'Tsetsnap'):

    else:
        eproto(fd)
        return


def prctl(conn):
    while True:
        msg = readmsg(conn)
        if msg:
            domsg(conn, msg)
        else:
            print "msg is None"
    

def parse_args():
    from optparse import OptionParser
    DEFAULT_PORT = 30000

    parser = OptionParser(usage="usage: %prog [options]")
    parser.add_option("-p", "--port", dest="port",
            default=DEFAULT_PORT, help="Port to listen on")

    (options, args) = parser.parse_args()
    return options.port

def get_client(port):
    s = socket.socket()
    print "Binding port %d" % int(port)
    #s.bind(('127.0.0.1', int(port)))
    s.bind(('0.0.0.0', int(port)))

    print "Listening..."
    s.listen(1)
    cli,addr = s.accept()

    print "Connection from %s:%d" % addr
    return cli


if __name__ == '__main__':
    port = parse_args()

    print "Waiting for client"
    conn = get_client(port)
    prctl(conn)



