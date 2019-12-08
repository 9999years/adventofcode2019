#include <stdio.h>

#define PROG_CAP 2048
#define INPUT_CAP 16
#define ARG_CAP 8
#define ARGS_ZEROED                                                            \
  { 0, 0, 0, 0, 0, 0, 0, 0 }
#define INSTRUCTION_SIZE 100;

#define ADD_LHS 0
#define ADD_RHS 1
#define ADD_RET 2

#define INPUT_RET 0
#define OUTPUT_VAL 0

#define JMP_COMPARE 0
#define JMP_DEST 1

enum bool {
  true = 0,
  false = 1,
};
typedef enum bool bool;

enum Operator {
  OpAdd = 1,
  OpMul = 2,
  OpInput = 3,
  OpOutput = 4,
  OpJNZ = 5,
  OpJZ = 6,
  OpLessThan = 7,
  OpEquals = 8,
  OpHalt = 99,
};

enum Mode {
  PositionMode = 0,
  ImmediateMode = 1,
};

struct Args {
  int args[ARG_CAP];
  size_t len;
};

struct Stack {
  int s[INPUT_CAP];
  size_t top;
};

struct VirtualMachine {
  int program[PROG_CAP];
  // Instruction pointer.
  size_t ip;
  struct Stack *input;
  struct Stack *output;
};

struct Instruction {
  enum Mode modes[ARG_CAP];
  enum Operator op;
  struct Args *args;
  size_t idx;
};

void new_stack(struct Stack *stack) { stack->top = 0; }
bool push(struct Stack *stack, int val) {
  if (stack->top >= INPUT_CAP - 1) {
    return false;
  }
  stack->top++;
  stack->s[stack->top] = val;
  return true;
}
int pop(struct Stack *stack) {
  stack->top--;
  return stack->s[stack->top + 1];
}

int get_arg(struct VirtualMachine *vm, enum Mode mode, size_t idx) {
  int immediate = vm->program[idx];
  switch (mode) {
  case ImmediateMode:
    return immediate;
  case PositionMode:
    return vm->program[idx];
  }
  return 0; // Unreachable.
}

void get_args(struct VirtualMachine *vm, struct Instruction *ins) {
  int i = 0;
  struct Args *args = ins->args;
  switch (ins->op) {
  case OpAdd:
  case OpMul:
  case OpLessThan:
  case OpEquals:
    // any, any, immediate
    args->len = 3;
    args->args[i] = get_arg(vm, ins->modes[i], ins->idx + i + 1);
    i++;
    args->args[i] = get_arg(vm, ins->modes[i], ins->idx + i + 1);
    i++;
    args->args[i] = get_arg(vm, ImmediateMode, ins->idx + i + 1);
    break;
  case OpInput:
    // immediate
    args->len = 1;
    args->args[0] = get_arg(vm, ImmediateMode, ins->idx + 1);
    break;
  case OpOutput:
    // any
    args->len = 1;
    args->args[0] = get_arg(vm, ins->modes[0], ins->idx + 1);
    break;
  case OpJNZ:
  case OpJZ:
    // any, any
    args->len = 2;
    args->args[i] = get_arg(vm, ins->modes[i], ins->idx + i + 1);
    i++;
    args->args[i] = get_arg(vm, ins->modes[i], ins->idx + i + 1);
    break;
  case OpHalt:
    // nothing
    args->len = 0;
    break;
  }
}

// int num_args(enum Operator op) {
// switch (op) {
// case OpHalt:
// return 0;
// case OpInput:
// case OpOutput:
// return 1;
// case OpJNZ:
// case OpJZ:
// return 2;
// case OpAdd:
// case OpMul:
// case OpLessThan:
// case OpEquals:
// return 3;
// }
// }

void get_modes(struct Instruction *ins, int raw_ins) {
  raw_ins /= INSTRUCTION_SIZE;
  for (int i = 0; i < ins->args->len; i++) {
    ins->modes[i] = raw_ins % 10;
    raw_ins /= 10;
  }
}

void get_instruction(struct VirtualMachine *vm, size_t idx,
                     struct Instruction *ins) {
  ins->idx = idx;
  ins->op = vm->program[idx] % INSTRUCTION_SIZE;
  get_modes(ins, vm->program[idx]);
  get_args(vm, ins);
}

bool eval_instruction(struct VirtualMachine *vm, struct Instruction *ins) {
  int *args = ins->args->args;
  size_t new_ip = vm->ip + ins->args->len + 1;
  switch (ins->op) {
  case OpAdd:
    vm->program[args[ADD_RET]] = args[ADD_LHS] + args[ADD_RHS];
    break;
  case OpMul:
    vm->program[args[ADD_RET]] = args[ADD_LHS] * args[ADD_RHS];
    break;
  case OpInput:
    vm->program[args[INPUT_RET]] = pop(vm->input);
    break;
  case OpOutput:
    push(vm->output, args[OUTPUT_VAL]);
    break;
  case OpJNZ:
  case OpJZ:
    if (args[JMP_COMPARE] == 0 || ins->op == OpJNZ) {
      new_ip = args[JMP_DEST];
    }
    break;
  case OpLessThan:
    vm->program[args[ADD_RET]] = args[ADD_LHS] < args[ADD_RHS] ? 1 : 0;
    break;
  case OpEquals:
    vm->program[args[ADD_RET]] = args[ADD_LHS] == args[ADD_RHS] ? 1 : 0;
    break;
  case OpHalt:
    return false;
  }
  vm->ip = new_ip;
  return true;
}

void eval(struct VirtualMachine *vm) {
  vm->ip = 0;
  bool executing = true;
  struct Instruction *ins;
  while (executing) {
    get_instruction(vm, vm->ip, ins);
    eval_instruction(vm, ins);
  }
}

void parse_file(char *buf, struct VirtualMachine *vm) {
  int field_start = 0;
  int field_end = 0;
  int prog_idx = 0;
  char field[64];
  for (int i = 0; i < sizeof(buf); i++) {
    switch (buf[i]) {
    case ',':
      break;
    }
  }
}

int main(int argc, const char *argv[]) {
  printf("hi\n");
  return 0;
}
