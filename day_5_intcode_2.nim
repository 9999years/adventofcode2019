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

const
    opInfo = [
        opAdd:    OpInfo(ty: opAdd,    code: 1,  width: 4),
        opMul:    OpInfo(ty: opMul,    code: 2,  width: 4),
        opInput:  OpInfo(ty: opInput,  code: 3,  width: 2),
        opOutput: OpInfo(ty: opOutput, code: 4,  width: 2),
        opHalt:   OpInfo(ty: opOutput, code: 99, width: 1),
    ]

    opCodes: array[int, OpType] = [
        opInfo[opAdd].code: opAdd,
        opInfo[opMul].code: opMul,
        opInfo[opInput].code: opInput,
        opInfo[opOutput].code: opOutput,
        opInfo[opHalt].code: opHalt,
    ]

