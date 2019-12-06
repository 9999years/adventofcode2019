import unittest
import tables
import strutils
import sequtils

type
    OpType = enum
        opAdd,
        opMul,
        opInput,
        opOutput,
        opJNZ, # Jump if non-zero
        opJZ, # Jump if zero
        opLessThan,
        opEquals,
        opHalt,

    OpInfo = object
        ty: OpType
        code: int
        width: int

    Program = seq[int]

    ArgIdx = tuple
        idx: int
        mode: AddrMode

    AddrMode = enum
        immediateMode,
        positionMode,

    Instruction = object
        argModes: seq[AddrMode]
        op: OpType
        idx: int

    VirtualMachine = object
        prog: Program
        input: seq[int]
        output: seq[int]

const
    # Tab /[,:]\zs/l1r0
    opInfo = {
        opAdd:      OpInfo(ty: opAdd,      code: 1,  width: 4),
        opMul:      OpInfo(ty: opMul,      code: 2,  width: 4),
        opInput:    OpInfo(ty: opInput,    code: 3,  width: 2),
        opOutput:   OpInfo(ty: opOutput,   code: 4,  width: 2),
        opJNZ:      OpInfo(ty: opJNZ,      code: 5,  width: 3),
        opJZ:       OpInfo(ty: opJZ,       code: 6,  width: 3),
        opLessThan: OpInfo(ty: opLessThan, code: 7,  width: 4),
        opEquals:   OpInfo(ty: opEquals,   code: 8,  width: 4),
        opHalt:     OpInfo(ty: opHalt,     code: 99, width: 1),
    }.toTable

    opCodes = {
        opInfo[opAdd].code: opAdd,
        opInfo[opMul].code: opMul,
        opInfo[opInput].code: opInput,
        opInfo[opOutput].code: opOutput,
        opInfo[opHalt].code: opHalt,
    }.toTable

    addrModes = {
        0: positionMode,
        1: immediateMode,
    }.toTable

    instructionOpCodeSize = 100

proc args(ty: OpType): int = opInfo[ty].width - 1

proc parseArgModes(ty: OpType, ins: int): seq[AddrMode] =
    var
        args = ins div instructionOpCodeSize
    for _ in 1 .. ty.args():
        result &= addrModes[args mod 10]
        args = args div 10
    return result

proc parseOpCode(ins: int): OpType =
    return opCodes[ins mod instructionOpCodeSize]

iterator args(ins: Instruction): ArgIdx =
    for i, m in ins.argModes:
        yield (idx: ins.idx + i + 1, mode: m)

proc outputArg(prog: Program, ins: Instruction): ArgIdx =
    return (idx: ins.idx + high(ins.argModes) + 1, mode: immediateMode)

proc parse(prog: Program, idx: int): Instruction =
    var
        ins = prog[idx]
        ty = parseOpCode(ins)
        argModes = ty.parseArgModes(ins)
    return Instruction(argModes: argModes, op: ty, idx: idx)

proc argValue(prog: Program, idx: ArgIdx): int =
    var
        immediate = prog[idx.idx]
    case idx.mode
    of immediateMode:
        return immediate
    of positionMode:
        return prog[immediate]

proc args(prog: Program, ins: Instruction): (seq[int], int) =
    for i in ins.args():
        result[0] &= prog.argValue(i)
    result[1] = prog.argValue(prog.outputArg(ins))
    return

proc evalInstruction(vm: var VirtualMachine, ins: Instruction): bool =
    case ins.op
    of opAdd:
        let (args, retAddr) = vm.prog.args(ins)
        vm.prog[retAddr] = args[0] + args[1]
    of opMul:
        let (args, retAddr) = vm.prog.args(ins)
        vm.prog[retAddr] = args[0] * args[1]
    of opInput:
        let (_, retAddr) = vm.prog.args(ins)
        vm.prog[retAddr] = vm.input.pop()
    of opOutput:
        let (args, _) = vm.prog.args(ins)
        vm.output &= args[0]
    of opHalt:
        return true
    return false

proc evalInstruction(vm: var VirtualMachine, idx: int): (bool, int) =
    var
        ins = vm.prog.parse(idx)
    return (evalInstruction(vm, ins), opInfo[ins.op].width)

proc eval(vm: var VirtualMachine): seq[int] =
    var
        halted = false
        idx = 0
        advance = 0
    while not halted:
        (halted, advance) = vm.evalInstruction(idx)
        idx += advance
    return vm.output

proc evalIntcode(s: seq[int]): seq[int] =
    var
        vm = VirtualMachine(prog: s, input: @[], output: @[])
    vm.eval()
    return vm.prog

proc readAndSplitToInt(path: string): seq[int] =
    let
        data = readFile(path).strip()
        numStrs = data.split({','})
    return numStrs.map(parseInt)

proc gravityAssistProgram(noun, verb: int): Program =
    result = readAndSplitToInt("data/day_2_gravity_assist_input.txt")
    result[1] = noun
    result[2] = verb
    return result

proc TESTProgram(): Program =
    return readAndSplitToInt("data/day_5_TEST_input.txt")

proc asVM(prog: Program, input: int): VirtualMachine =
    return VirtualMachine(prog: prog, input: @[input], output: @[])

suite "Advent of Code, Day 5: Sunny with a Chance of Asteroids":
    test "Simple intcode tests":
        check(evalIntcode(@[2, 3, 0, 3, 99]) == [2, 3, 0, 6, 99])
        check(evalIntcode(@[2, 4, 4, 5, 99, 0]) == [2, 4, 4, 5, 99, 9_801])
        check(evalIntcode(@[1, 1, 1, 4, 99, 5, 6, 0, 99]) ==
            [30, 1, 1, 4, 2, 5, 6, 0, 99])

    test "Day 2, Part 1":
        check(evalIntcode(gravityAssistProgram(12, 2))[0] == 6_327_510)

    test "Day 5, Part 1":
        check(TESTProgram().asVM(1).eval() ==
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 14_155_342])

    test "Equal to 8 (position-mode)":
        let equalTo8 = [3, 9, 8, 9, 10, 9, 4, 9, 99, -1, 8]
        check(equalTo8.asVM(8).eval() == [1])
        check(equalTo8.asVM(7).eval() == [0])
        check(equalTo8.asVM(9).eval() == [0])
        check(equalTo8.asVM(-3).eval() == [0])

    test "Less than 8 (position-mode)":
        let lessThan8 = [3, 9, 7, 9, 10, 9, 4, 9, 99, -1, 8]
        check(lessThan8.asVM(8).eval() == [0])
        check(lessThan8.asVM(7).eval() == [1])
        check(lessThan8.asVM(9).eval() == [0])
        check(lessThan8.asVM(10).eval() == [0])
        check(lessThan8.asVM(-3).eval() == [1])

    test "Equal to 8 (immediate-mode)":
        let equal8Immediate = [3, 3, 1108, -1, 8, 3, 4, 3, 99]
        check(equalTo8Immediate.asVM(8).eval() == [1])
        check(equalTo8Immediate.asVM(7).eval() == [0])
        check(equalTo8Immediate.asVM(9).eval() == [0])
        check(equalTo8Immediate.asVM(-3).eval() == [0])

    test "Less than 8 (immediate-mode)":
        let lessThan8Immediate = [3, 3, 1107, -1, 8, 3, 4, 3, 99]
        check(lessThan8Immediate.asVM(8).eval() == [0])
        check(lessThan8Immediate.asVM(7).eval() == [1])
        check(lessThan8Immediate.asVM(9).eval() == [0])
        check(lessThan8Immediate.asVM(10).eval() == [0])
        check(lessThan8Immediate.asVM(-3).eval() == [1])

    test "Jumps (position-mode)":
        let prog = [3, 12, 6, 12, 15, 1, 13, 14, 13, 4, 13, 99, -1, 0, 1, 9]
        # 0 if input is 0, 1 if non-zero
        check(prog.asVM(0).eval() == [0])
        check(prog.asVM(1).eval() == [1])
        check(prog.asVM(993802).eval() == [1])
        check(prog.asVM(-392).eval() == [1])

    test "Jumps (immediate-mode)":
        let prog = [3, 3, 1105, -1, 9, 1101, 0, 0, 12, 4, 12, 99, 1]
        # 0 if input is 0, 1 if non-zero
        check(prog.asVM(0).eval() == [0])
        check(prog.asVM(1).eval() == [1])
        check(prog.asVM(993802).eval() == [1])
        check(prog.asVM(-392).eval() == [1])

    test "Large example":
        let prog = [
            3, 21, 1008, 21, 8, 20, 1005, 20, 22, 107, 8, 21, 20, 1006, 20, 31,
            1106, 0, 36, 98, 0, 0, 1002, 21, 125, 20, 4, 20, 1105, 1, 46, 104,
            999, 1105, 1, 46, 1101, 1000, 1, 20, 4, 20, 1105, 1, 46, 98, 99
        ]

    test "Day 5, Part 2 (final)":
        check(TESTProgram().asVM(5).eval() ==
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0])
