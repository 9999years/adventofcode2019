#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define TEST_OK                                                                \
  printf("\x1B[32m.\x1B[0m");                                                  \
  TEST_COUNT++;                                                                \
  return;
#define TEST_NOT_OK                                                            \
  printf("\x1B[31m.\x1B[0m");                                                  \
  TEST_COUNT++;                                                                \
  TEST_FAILURES++;                                                             \
  return;
#define TEST_FIN(failed__)                                                     \
  if (failed__) {                                                              \
    TEST_NOT_OK                                                                \
  } else {                                                                     \
    TEST_OK                                                                    \
  }

#define PROG_CAP 2048
#define INPUT_CAP 16
#define ARG_CAP 8
#define INSTRUCTION_SIZE 100;

#define ADD_LHS 0
#define ADD_RHS 1
#define ADD_RET 2

#define INPUT_RET 0
#define OUTPUT_VAL 0

#define JMP_COMPARE 0
#define JMP_DEST 1

enum bool {
  false = 0,
  true = 1,
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
  enum Mode modes[ARG_CAP];
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
  enum Operator op;
  struct Args *args;
};

int TEST_COUNT;
int TEST_FAILURES;

void init_instruction(struct Instruction *ins) {
  ins->op = OpAdd;
  ins->args = malloc(sizeof(struct Args));
  ins->args->len = 0;
}

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

void print_arr(int *a, int items) {
  printf("[");
  for (int i = 0; i < items - 1; i++) {
    printf("%d, ", a[i]);
  }
  printf("%d]\n", a[items - 1]);
}

void init_vm(struct VirtualMachine *vm) {
  vm->ip = 0;
  vm->input = malloc(sizeof(struct Stack));
  new_stack(vm->input);
  vm->output = malloc(sizeof(struct Stack));
  new_stack(vm->output);
}

int get_arg(struct VirtualMachine *vm, enum Mode mode, size_t idx) {
  int immediate = vm->program[idx];
  switch (mode) {
  case ImmediateMode:
    return immediate;
  case PositionMode:
    return vm->program[immediate];
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
    args->args[i] = get_arg(vm, args->modes[i], vm->ip + i + 1);
    i++;
    args->args[i] = get_arg(vm, args->modes[i], vm->ip + i + 1);
    i++;
    args->args[i] = get_arg(vm, ImmediateMode, vm->ip + i + 1);
    break;
  case OpInput:
    args->args[0] = get_arg(vm, ImmediateMode, vm->ip + 1);
    break;
  case OpOutput:
    args->args[0] = get_arg(vm, args->modes[0], vm->ip + 1);
    break;
  case OpJNZ:
  case OpJZ:
    args->args[i] = get_arg(vm, args->modes[i], vm->ip + i + 1);
    i++;
    args->args[i] = get_arg(vm, args->modes[i], vm->ip + i + 1);
    break;
  case OpHalt:
    break;
  }
}

int num_args(enum Operator op) {
  switch (op) {
  case OpHalt:
    return 0;
  case OpInput:
  case OpOutput:
    return 1;
  case OpJNZ:
  case OpJZ:
    return 2;
  case OpAdd:
  case OpMul:
  case OpLessThan:
  case OpEquals:
    return 3;
  }
  return 0;
}

void get_modes(struct Args *args, int raw_ins) {
  raw_ins /= INSTRUCTION_SIZE;
  for (int i = 0; i < args->len; i++) {
    args->modes[i] = raw_ins % 10;
    raw_ins /= 10;
  }
}

void get_instruction(struct VirtualMachine *vm, struct Instruction *ins) {
  ins->op = vm->program[vm->ip] % INSTRUCTION_SIZE;
  ins->args->len = num_args(ins->op);
  get_modes(ins->args, vm->program[vm->ip]);
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
    if ((args[JMP_COMPARE] == 0) ^ (ins->op == OpJNZ)) {
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

void print_vm(struct VirtualMachine *vm) {
  printf(" Input stack: ");
  print_arr(vm->input->s, vm->input->top);
  printf("Output stack: ");
  print_arr(vm->output->s, vm->output->top);
  printf("     Program: ");
  print_arr(vm->program, PROG_CAP);
}

void eval(struct VirtualMachine *vm) {
  vm->ip = 0;
  struct Instruction *ins = malloc(sizeof(struct Instruction));
  init_instruction(ins);
  get_instruction(vm, ins);
  while (eval_instruction(vm, ins)) {
    if (vm->ip >= PROG_CAP) {
      printf("Instruction pointer out of bounds at %lu!\n", vm->ip);
      print_vm(vm);
      return;
    }
    get_instruction(vm, ins);
  }
}

int parse_file(FILE *fp, struct VirtualMachine *vm) {
  int *cur = malloc(sizeof(int));
  int i;
  for (i = 0; fscanf(fp, "%d%*[,\n]", cur) == 1; i++) {
    vm->program[i] = *cur;
  }
  return i;
}

bool assert_eq(int a, int b) {
  if (a != b) {
    printf("Assertion failed:\n"
           "    Left: %d\n"
           "   Right: %d\n",
           a, b);
    return false;
  }
  return true;
}

bool assert_vm_prog(struct VirtualMachine *vm, int *program, size_t prog_len) {
  for (int i = 0; i < prog_len; i++) {
    if (!assert_eq(vm->program[i], program[i])) {
      printf("Assertion failed at index %d.\n"
             "Expected output: ",
             i);
      print_arr(program, prog_len);
      printf("  Actual output: ");
      print_arr(vm->program, prog_len);
      return false;
    }
  }
  return true;
}

#define INTCODE_CHECK_IO(program__, len__, input__, output__)                  \
  {                                                                            \
    struct VirtualMachine *vm = malloc(sizeof(struct VirtualMachine));         \
    init_vm(vm);                                                               \
    memcpy(vm->program, program__, len__ * sizeof(int));                       \
    push(vm->input, input__);                                                  \
    eval(vm);                                                                  \
    failed = failed || !assert_eq(pop(vm->output), output__);                  \
  }

#define INTCODE_CHECK(program__, expect__)                                     \
  {                                                                            \
    struct VirtualMachine *vm = malloc(sizeof(struct VirtualMachine));         \
    memcpy(vm->program, program__, sizeof(program__));                         \
    eval(vm);                                                                  \
    if (assert_vm_prog(vm, expect__, LENGTH(expect))) {                        \
      TEST_OK                                                                  \
    } else {                                                                   \
      TEST_NOT_OK                                                              \
    }                                                                          \
  }

void test_intcode_simple_1() {
  int prog[] = {2, 3, 0, 3, 99};
  int expect[] = {2, 3, 0, 6, 99};
  INTCODE_CHECK(prog, expect)
}

void test_intcode_simple_2() {
  int prog[] = {2, 4, 4, 5, 99, 0};
  int expect[] = {2, 4, 4, 5, 99, 9801};
  INTCODE_CHECK(prog, expect)
}

void test_intcode_eq8_pos() {
  bool failed = false;
  int prog[] = {3, 9, 8, 9, 10, 9, 4, 9, 99, -1, 8};
  INTCODE_CHECK_IO(prog, 11, 8, 1)
  INTCODE_CHECK_IO(prog, 11, 7, 0)
  INTCODE_CHECK_IO(prog, 11, 9, 0)
  INTCODE_CHECK_IO(prog, 11, -3, 0)
  TEST_FIN(failed)
}

void test_intcode_le8_pos() {
  bool failed = false;
  int prog[] = {3, 9, 7, 9, 10, 9, 4, 9, 99, -1, 8};
  INTCODE_CHECK_IO(prog, 11, 8, 0)
  INTCODE_CHECK_IO(prog, 11, 7, 1)
  INTCODE_CHECK_IO(prog, 11, 9, 0)
  INTCODE_CHECK_IO(prog, 11, 10, 0)
  INTCODE_CHECK_IO(prog, 11, -3, 1)
  TEST_FIN(failed)
}

void test_eq8_imm() {
  bool failed = false;
  int prog[] = {3, 3, 1108, -1, 8, 3, 4, 3, 99};
  INTCODE_CHECK_IO(prog, 9, 8, 1)
  INTCODE_CHECK_IO(prog, 9, 7, 0)
  INTCODE_CHECK_IO(prog, 9, 9, 0)
  INTCODE_CHECK_IO(prog, 9, -3, 0)
  TEST_FIN(failed)
}

void test_le8_imm() {
  bool failed = false;
  int prog[] = {3, 3, 1107, -1, 8, 3, 4, 3, 99};
  INTCODE_CHECK_IO(prog, 11, 8, 0)
  INTCODE_CHECK_IO(prog, 11, 7, 1)
  INTCODE_CHECK_IO(prog, 11, 9, 0)
  INTCODE_CHECK_IO(prog, 11, 10, 0)
  INTCODE_CHECK_IO(prog, 11, -3, 1)
  TEST_FIN(failed)
}

void test_jumps_pos() {
  bool failed = false;
  int prog[] = {3, 12, 6, 12, 15, 1, 13, 14, 13, 4, 13, 99, -1, 0, 1, 9};
  INTCODE_CHECK_IO(prog, 16, 0, 0)
  INTCODE_CHECK_IO(prog, 16, 1, 1)
  INTCODE_CHECK_IO(prog, 16, 993802, 1)
  INTCODE_CHECK_IO(prog, 16, -392, 1)
  TEST_FIN(failed)
}

void test_jumps_imm() {
  bool failed = false;
  int prog[] = {3, 3, 1105, -1, 9, 1101, 0, 0, 12, 4, 12, 99, 1};
  INTCODE_CHECK_IO(prog, 13, 0, 0)
  INTCODE_CHECK_IO(prog, 13, 1, 1)
  INTCODE_CHECK_IO(prog, 13, -1, 1)
  INTCODE_CHECK_IO(prog, 13, 993802, 1)
  INTCODE_CHECK_IO(prog, 13, -392, 1)
  TEST_FIN(failed)
}

void test_day_5_part_2() {
  bool failed = false;
  struct VirtualMachine *vm_ = malloc(sizeof(struct VirtualMachine));
  FILE *fp = fopen("data/day_5_TEST_input.txt", "r");
  parse_file(fp, vm_);
  fclose(fp);
  // INTCODE_CHECK_IO(vm_->program, 678, 5, 8684145);
  TEST_FIN(failed)
}

int main(int argc, const char *argv[]) {
  TEST_COUNT = 0;
  TEST_FAILURES = 0;

  test_intcode_simple_1();
  test_intcode_simple_2();
  test_intcode_eq8_pos();
  test_intcode_le8_pos();
  test_eq8_imm();
  test_le8_imm();
  test_jumps_pos();
  test_jumps_imm();
  test_day_5_part_2();

  printf("\n");
  printf("Ran %d tests", TEST_COUNT);
  if (TEST_FAILURES == 0) {
    printf(".\n"
           "All tests pass!\n");
  } else {
    printf(", got %d failures.\n", TEST_FAILURES);
  }
  return 0;
}
