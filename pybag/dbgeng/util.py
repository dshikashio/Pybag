import binascii
import functools
import os
import string

import capstone

"""
Utilities that don't depend directly on DbgEng
"""

def logger(f):
    """logger - method decorator for dbg"""
    @functools.wraps(f)
    def wrapper(*args, **kwargs):
        print(f.__name__, args, kwargs)
        return f(*args, **kwargs)
    return wrapper

def module_locator():
    return os.path.dirname(os.path.realpath(__file__))

def hexdump(data, vaddr=0, width=16):
    def printable(c):
        if c in string.whitespace and c != ' ':
            c = '.'
        elif c not in string.printable:
            c = '.'
        return c
    i = 0
    while i < len(data):
        line = data[i:i+width]
        h = ["%02x" % x for x in line]
        a = ["%c" % printable(chr(x)) for x in line]
        pad = "   "*((width+1) - len(line))
        print("%07x: %s%s%s" % (i+vaddr, ' '.join(h), pad, ''.join(a)))
        i += width
    print("")

def str_memory_state(val):
    s = []
    if val & 0x1000:
        s.append('MEM_COMMIT')
    if val & 0x2000:
        s.append('MEM_RESERVE')
    if val & 0x10000:
        s.append('MEM_FREE')
    return ' | '.join(s)

def str_memory_protect(val):
    prot_tab = { 0x10: 'PAGE_EXECUTE',
                 0x20: 'PAGE_EXECUTE_READ',
                 0x40: 'PAGE_EXECUTE_READWRITE',
                 0x80: 'PAGE_EXECUTE_WRITECOPY',
                 0x01: 'PAGE_NOACCESS',
                 0x02: 'PAGE_READONLY',
                 0x04: 'PAGE_READWRITE',
                 0x08: 'PAGE_WRITECOPY',
                 0x100: 'PAGE_GUARD',
                 0x200: 'PAGE_NOCACHE',
                 0x400: 'PAGE_WRITECOMBINE',
               }
    return ' | '.join(desc
                for mask, desc in prot_tab.items()
                if val & mask)

def str_memory_type(val):
    s = []
    if val & 0x20000:
        s.append('MEM_PRIVATE')
    if val & 0x40000:
        s.append('MEM_MAPPED')
    if val & 0x1000000:
        s.append('MEM_IMAGE')
    return ' | '.join(s)

def str_memory_info(info):
    fmt  = "%015x - %015x : %08x"
    fmt2 = "\t%s %s %s"
    return '\n'.join([fmt  % (info.BaseAddress, info.BaseAddress + info.RegionSize, info.RegionSize),
                      fmt2 % (str_memory_state(info.State), 
                              str_memory_protect(info.Protect), 
                              str_memory_type(info.Type))])

def str_instruction(ins, bitness):
    if bitness == '64':
        fmt = "{:015x} {:<16} {:<5}  {}"
    else:
        fmt = "{:07x} {:<16} {:<5}  {}"
    return fmt.format(ins.address, binascii.hexlify(ins.bytes).decode(), ins.mnemonic, ins.op_str)

def disassemble_instruction(bitness, address, data):
    if bitness == '64':
        mode = capstone.CS_MODE_64
    else:
        mode = capstone.CS_MODE_32

    # By only doing one instr this is kinda gross
    # - But do we care? This keeps utils 'cleaner'
    cs  = capstone.Cs(capstone.CS_ARCH_X86, mode)
    try:
        ins = next(cs.disasm(data, address))
    except RuntimeError:
        ins = None
    return ins

def disassemble_string(bitness, address, data, verbose=False):
    ins = disassemble_instruction(bitness, address, data)

    if ins and ins.size > 0:
        disasm = '{} {}'.format(ins.mnemonic, ins.op_str)
        return (disasm, ins.size)
    else:
        return None

def bp_wrap(cls, fn):
    @functools.wraps(fn)
    def bp_handler(bp):
        return fn(bp, cls)
    return bp_handler