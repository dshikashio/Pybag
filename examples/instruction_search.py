import random
import sys
import pydbgeng
import pywindbg
import beadisasm


def check_jmpcall(instr):
    if instr.Instruction.BranchType not in (beadisasm.JmpType, beadisasm.CallType):
        return False

    if instr.Instruction.Mnemonic.strip() in ('jmp far', 'call far'):
        return False

    if instr.Argument2.ArgType & beadisasm.MEMORY_TYPE:
        if (instr.Argument2.Memory.BaseRegister != 0 or 
                instr.Argument2.Memory.IndexRegister != 0):
            return True

    elif instr.Argument2.ArgType & beadisasm.REGISTER_TYPE:
        if (instr.Argument1.ArgType == 0 and
                instr.Argument3.ArgType == 0):
            return True

    return False

def check_ret(instr):
    if instr.Instruction.BranchType != beadisasm.RetType:
        return False

    if instr.Instruction.Opcode in (0xc2, 0xc3):
        return True

    return False

def find_branch(w, addr, len):
    res = []
    end = addr + len
    while addr < end:
        instr = w.instruction_at(addr)
        if instr:
            if check_ret(instr[0]) or check_jmpcall(instr[0]):
                #print "%016x : " % addr, instr[0].CompleteInstr
                res.append(addr)
        addr += 1
    return res

# esp == REG4

# pop esp
# mov esp, any
# xchg esp, any
# xchg any, esp

# popad
# sub esp, any
# add esp, any
# leave

def check_mov(instr):
    reg = beadisasm.REG4
    m = instr.Instruction.Mnemonic.strip()
    mask = beadisasm.REGISTER_TYPE | reg | beadisasm.GENERAL_REG
    if m in ('mov', 'xchg', 'pop'):
        if m == 'mov' and not instr.Argument2.ArgType & beadisasm.REGISTER_TYPE:
            return False
        if (instr.Argument1.ArgType & mask) == mask:
            return True

    if m == 'xchg':
        if (instr.Argument2.ArgType & mask) == mask:
            return True

    return False


def find_mov(w, addr, len):
    res = []
    end = addr + len
    while addr < end:
        instr = w.instruction_at(addr)
        if instr and check_mov(instr[0]):
            #print "%016x : " % addr, instr[0].CompleteInstr
            res.append(addr)
        addr += 1
    return res

def iter_mod(w, name, searchfn):
    res = []
    mod = w.mod[name]
    for section in mod.section_list():
        if section.IMAGE_SCN_MEM_EXECUTE:
            print "Checking %s %s" % (name, section.Name.strip('\x00'))
            res += searchfn(w, 
                            mod.addr + section.VirtualAddress,
                            section.Misc_VirtualSize)
    return res

w = pywindbg.Userdbg()
w.quiet()
w.create(r'c:\windows\system32\notepad.exe')
w.go(3)

#for mod in w.module_list():
#    branches += iter_mod(w, mod[0][1], find_branch) 
branches = iter_mod(w, 'winspool', find_branch)

for addr in branches:
    for x in w._disasm_back(addr, 16):
        print "---"
        print x


