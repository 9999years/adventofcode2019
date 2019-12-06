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

    Program = seq[int]

    ArgIdx = tuple
        idx: int
        mode: AddrMode

    AddrMode = enum
        immediateMode,
        positionMode,

    ArgType = enum
        argAny,
        argImmediate,

    Instruction = object
        argModes: seq[AddrMode]
        op: OpType
        idx: int

    VirtualMachine = ref object
        prog: Program
        input: seq[int]
        output: seq[int]

    OpInfo = object
        ty: OpType
        code: int
        width: int
        args: seq[ArgType]

const
    # Tab /[,:]\zs/l1r0
    opInfo = {
        opAdd:      OpInfo(ty: opAdd,      code: 1,  width: 4, args: @[argAny, argAny, argImmediate]),
        opMul:      OpInfo(ty: opMul,      code: 2,  width: 4, args: @[argAny, argAny, argImmediate]),
        opInput:    OpInfo(ty: opInput,    code: 3,  width: 2, args: @[argImmediate]),
        opOutput:   OpInfo(ty: opOutput,   code: 4,  width: 2, args: @[argAny]),
        opJNZ:      OpInfo(ty: opJNZ,      code: 5,  width: 3, args: @[argAny, argAny]),
        opJZ:       OpInfo(ty: opJZ,       code: 6,  width: 3, args: @[argAny, argAny]),
        opLessThan: OpInfo(ty: opLessThan, code: 7,  width: 4, args: @[argAny, argAny, argImmediate]),
        opEquals:   OpInfo(ty: opEquals,   code: 8,  width: 4, args: @[argAny, argAny, argImmediate]),
        opHalt:     OpInfo(ty: opHalt,     code: 99, width: 1, args: @[]),
    }.toTable

    LHS_IDX = 0
    RHS_IDX = 1
    RET_IDX = 2

    INPUT_RET = 0
    OUTPUT_VAL = 0

    JMP_CMP = 0
    JMP_TO = 1

    opCodes = {
        opInfo[opAdd].code: opAdd,
        opInfo[opMul].code: opMul,
        opInfo[opInput].code: opInput,
        opInfo[opOutput].code: opOutput,
        opInfo[opJNZ].code: opJNZ,
        opInfo[opJZ].code: opJZ,
        opInfo[opLessThan].code: opLessThan,
        opInfo[opEquals].code: opEquals,
        opInfo[opHalt].code: opHalt,
    }.toTable

    addrModes = {
        0: positionMode,
        1: immediateMode,
    }.toTable

    instructionOpCodeSize = 100

proc args(ty: OpType): int = opInfo[ty].width - 1

proc parseArgModes(ty: OpType, ins: int): seq[AddrMode] =
    let mandatedArgModes = opInfo[ty].args
    var args = ins div instructionOpCodeSize
    for i in 0 ..< ty.args():
        case mandatedArgModes[i]
        of argAny:
            result &= addrModes[args mod 10]
        of argImmediate:
            result &= immediateMode
        args = args div 10
    return result

proc parseOpCode(ins: int): OpType =
    return opCodes[ins mod instructionOpCodeSize]

iterator args(ins: Instruction): ArgIdx =
    for i, m in ins.argModes:
        yield (idx: ins.idx + i + 1, mode: m)

proc parse(prog: Program, idx: int): Instruction =
    var
        ins = prog[idx]
        ty = parseOpCode(ins)
        argModes = ty.parseArgModes(ins)
    return Instruction(argModes: argModes, op: ty, idx: idx)

proc argValue(prog: Program, idx: ArgIdx): int =
    var immediate = prog[idx.idx]
    case idx.mode
    of immediateMode:
        return immediate
    of positionMode:
        return prog[immediate]

proc args(prog: Program, ins: Instruction): seq[int] =
    for i in ins.args():
        result &= prog.argValue(i)

proc evalInstruction(vm: var VirtualMachine, ins: Instruction): (bool, int) =
    result[1] = opInfo[ins.op].width
    case ins.op
    of opAdd:
        let args = vm.prog.args(ins)
        vm.prog[args[RET_IDX]] = args[LHS_IDX] + args[RHS_IDX]
    of opMul:
        let args = vm.prog.args(ins)
        vm.prog[args[RET_IDX]] = args[LHS_IDX] * args[RHS_IDX]
    of opInput:
        let args = vm.prog.args(ins)
        vm.prog[args[INPUT_RET]] = vm.input.pop()
    of opOutput:
        let args = vm.prog.args(ins)
        vm.output &= args[OUTPUT_VAL]
    of opJNZ, opJZ:
        let args = vm.prog.args(ins)
        if (ins.op == opJNZ and args[JMP_CMP] != 0) or
                (ins.op == opJZ and args[JMP_CMP] == 0):
            result[1] = args[JMP_TO] - ins.idx
    of opLessThan:
        let args = vm.prog.args(ins)
        vm.prog[args[RET_IDX]] =
            if args[LHS_IDX] < args[RHS_IDX]: 1
            else: 0
    of opEquals:
        let args = vm.prog.args(ins)
        vm.prog[args[RET_IDX]] =
            if args[LHS_IDX] == args[RHS_IDX]: 1
            else: 0
    of opHalt:
        result[0] = true

proc evalInstruction(vm: var VirtualMachine, idx: int): (bool, int) =
    var ins = vm.prog.parse(idx)
    return evalInstruction(vm, ins)

proc eval(vm: var VirtualMachine): seq[int] =
    var
        halted = false
        idx = 0
        advance = 0
    while not halted:
        (halted, advance) = vm.evalInstruction(idx)
        idx += advance
    return vm.output

proc evalIntcode(prog: Program): seq[int] =
    var vm = VirtualMachine(prog: prog, input: @[], output: @[])
    discard vm.eval()
    return vm.prog

proc intcodeOutput(prog: Program, input: int = 0): seq[int] =
    var vm = VirtualMachine(prog: prog, input: @[input], output: @[])
    return vm.eval()

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
        check(intcodeOutput(TESTProgram(), 1) ==
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 14_155_342])

    test "Equal to 8 (position-mode)":
        let equalTo8 = @[3, 9, 8, 9, 10, 9, 4, 9, 99, -1, 8]
        check(intcodeOutput(equalTo8, 8) == [1])
        check(intcodeOutput(equalTo8, 7) == [0])
        check(intcodeOutput(equalTo8, 9) == [0])
        check(intcodeOutput(equalTo8, -3) == [0])

    test "Less than 8 (position-mode)":
        let lessThan8 = @[3, 9, 7, 9, 10, 9, 4, 9, 99, -1, 8]
        check(intcodeOutput(lessThan8, 8) == [0])
        check(intcodeOutput(lessThan8, 7) == [1])
        check(intcodeOutput(lessThan8, 9) == [0])
        check(intcodeOutput(lessThan8, 10) == [0])
        check(intcodeOutput(lessThan8, -3) == [1])

    test "Equal to 8 (immediate-mode)":
        let equalTo8Immediate = @[3, 3, 1108, -1, 8, 3, 4, 3, 99]
        check(intcodeOutput(equalTo8Immediate, 8) == [1])
        check(intcodeOutput(equalTo8Immediate, 7) == [0])
        check(intcodeOutput(equalTo8Immediate, 9) == [0])
        check(intcodeOutput(equalTo8Immediate, -3) == [0])

    test "Less than 8 (immediate-mode)":
        let lessThan8Immediate = @[3, 3, 1107, -1, 8, 3, 4, 3, 99]
        check(intcodeOutput(lessThan8Immediate, 8) == [0])
        check(intcodeOutput(lessThan8Immediate, 7) == [1])
        check(intcodeOutput(lessThan8Immediate, 9) == [0])
        check(intcodeOutput(lessThan8Immediate, 10) == [0])
        check(intcodeOutput(lessThan8Immediate, -3) == [1])

    test "Jumps (position-mode)":
        let prog = @[3, 12, 6, 12, 15, 1, 13, 14, 13, 4, 13, 99, -1, 0, 1, 9]
        # 0 if input is 0, 1 if non-zero
        check(intcodeOutput(prog, 0) == [0])
        check(intcodeOutput(prog, 1) == [1])
        check(intcodeOutput(prog, 993802) == [1])
        check(intcodeOutput(prog, -392) == [1])

    test "Jumps (immediate-mode)":
        let prog = @[3, 3, 1105, -1, 9, 1101, 0, 0, 12, 4, 12, 99, 1]
        # 0 if input is 0, 1 if non-zero
        check(intcodeOutput(prog, 0) == [0])
        check(intcodeOutput(prog, 1) == [1])
        check(intcodeOutput(prog, 993802) == [1])
        check(intcodeOutput(prog, -392) == [1])

    test "Large example":
        let prog = @[
            3, 21, 1008, 21, 8, 20, 1005, 20, 22, 107, 8, 21, 20, 1006, 20, 31,
            1106, 0, 36, 98, 0, 0, 1002, 21, 125, 20, 4, 20, 1105, 1, 46, 104,
            999, 1105, 1, 46, 1101, 1000, 1, 20, 4, 20, 1105, 1, 46, 98, 99
        ]
        check(intcodeOutput(prog, -3) == [999])
        check(intcodeOutput(prog, 0) == [999])
        check(intcodeOutput(prog, 1) == [999])
        check(intcodeOutput(prog, 7) == [999])
        check(intcodeOutput(prog, 8) == [1000])
        check(intcodeOutput(prog, 9) == [1001])
        check(intcodeOutput(prog, 10) == [1001])
        check(intcodeOutput(prog, 1039) == [1001])
        check(intcodeOutput(prog, 4895096) == [1001])

    test "Day 5, Part 2 (final)":
        check(intcodeOutput(TESTProgram(), 5) == [8684145])
