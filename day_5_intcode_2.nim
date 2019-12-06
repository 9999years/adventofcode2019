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
    opInfo = {
        opAdd:    OpInfo(ty: opAdd,    code: 1,  width: 4),
        opMul:    OpInfo(ty: opMul,    code: 2,  width: 4),
        opInput:  OpInfo(ty: opInput,  code: 3,  width: 2),
        opOutput: OpInfo(ty: opOutput, code: 4,  width: 2),
        opHalt:   OpInfo(ty: opHalt,   code: 99, width: 1),
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

proc eval(vm: var VirtualMachine) =
    var
        halted = false
        idx = 0
        advance = 0
    while not halted:
        (halted, advance) = vm.evalInstruction(idx)
        idx += advance

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

suite "Advent of Code, Day 5: Sunny with a Chance of Asteroids":
    test "Simple intcode tests":
        check(evalIntcode(@[2, 3, 0, 3, 99]) == [2, 3, 0, 6, 99])
        check(evalIntcode(@[2, 4, 4, 5, 99, 0]) == [2, 4, 4, 5, 99, 9_801])
        check(evalIntcode(@[1, 1, 1, 4, 99, 5, 6, 0, 99]) ==
            [30, 1, 1, 4, 2, 5, 6, 0, 99])

    test "Day 2, Part 1":
        check(evalIntcode(gravityAssistProgram(12, 2))[0] == 6_327_510)

    test "Day 5, Part 1":
        let code = TESTProgram()
        var vm = VirtualMachine(
            prog: code,
            input: @[1],
            output: @[],
        )
        vm.eval()
        check(vm.output == [0, 0, 0, 0, 0, 0, 0, 0, 0, 14_155_342])
