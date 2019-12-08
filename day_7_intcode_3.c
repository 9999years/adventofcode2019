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
    printf("Attempted to push onto full stack!\n");
    return false;
  }
  stack->s[stack->top] = val;
  stack->top++;
  return true;
}

int pop(struct Stack *stack) {
  if (stack->top <= 0) {
    printf("Attempted to pop off of empty stack!\n");
    return 0;
  }
  stack->top--;
  return stack->s[stack->top];
}

bool enqueue(struct Stack *stack, int val) {
  if (stack->top >= INPUT_CAP - 1) {
    return false;
  }
  stack->s[stack->top] = stack->s[stack->top - 1];
  for (int i = stack->top - 1; i >= 0; i--) {
    stack->s[i] = stack->s[i - 1];
  }
  stack->s[0] = val;
  stack->top++;
  return true;
}

void print_arr(int *a, int items) {
  printf("[");
  for (int i = 0; i < items - 1; i++) {
    printf("%d, ", a[i]);
  }
  if (items != 0) {
    printf("%d", a[items - 1]);
  }
  printf("]\n");
}

void init_vm(struct VirtualMachine *vm) {
  vm->ip = 0;
  vm->input = malloc(sizeof(struct Stack));
  new_stack(vm->input);
  vm->output = malloc(sizeof(struct Stack));
  new_stack(vm->output);
}

void reset_vm(struct VirtualMachine *vm) {
  vm->ip = 0;
  vm->input->top = 0;
  vm->output->top = 0;
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
    enqueue(vm->output, args[OUTPUT_VAL]);
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

bool eval_one(struct VirtualMachine *vm, struct Instruction *ins) {
  get_instruction(vm, ins);
  return eval_instruction(vm, ins);
}

void eval(struct VirtualMachine *vm) {
  vm->ip = 0;
  struct Instruction *ins = malloc(sizeof(struct Instruction));
  init_instruction(ins);
  while (eval_one(vm, ins)) {
    if (vm->ip >= PROG_CAP) {
      printf("Instruction pointer out of bounds at %lu!\n", vm->ip);
      print_vm(vm);
      return;
    }
  }
}

bool eval_until_output(struct VirtualMachine *vm, struct Instruction *ins) {
  ins->op = OpAdd;
  while (ins->op != OpOutput) {
    if (!eval_one(vm, ins)) {
      return false;
    }
  }
  return true;
}

void eval_looped(struct VirtualMachine *vm[], int input[], size_t count) {
  bool *halted = malloc(sizeof(bool) * count);
  int halted_count = 0;
  for (int i = 0; i < count; i++) {
    push(vm[i]->input, input[i]);
    vm[i]->ip = 0;
    halted[i] = false;
  }
  enqueue(vm[0]->input, 0);

  struct Instruction *ins = malloc(sizeof(struct Instruction));
  init_instruction(ins);

  while (halted_count < count) {
    for (int i = 0; i < count; i++) {
      if (!halted[i]) {
        halted[i] |= !eval_until_output(vm[i], ins);
        if (halted[i]) {
          halted_count++;
        } else {
          size_t next = (i + 1 + count) % count;
          enqueue(vm[next]->input, pop(vm[i]->output));
        }
      }
    }
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

void load_amplifier_software(struct VirtualMachine *vm) {
  FILE *fp = fopen("data/day_7_amplifier_input.txt", "r");
  parse_file(fp, vm);
  fclose(fp);
}

int thruster_signal(struct VirtualMachine *vm, int input[5]) {
  int *prog = malloc(sizeof(int) * PROG_CAP);
  memcpy(prog, vm->program, sizeof(int) * PROG_CAP);

  int prev_output = 0;
  for (int i = 0; i < 5; i++) {
    push(vm->input, prev_output);
    push(vm->input, input[i]);
    eval(vm);
    prev_output = pop(vm->output);
    reset_vm(vm);
    memcpy(vm->program, prog, sizeof(int) * PROG_CAP);
  }
  return prev_output;
}

int find_max_thruster_signal(struct VirtualMachine *vm) {
  int cur_seq[] = {0, 0, 0, 0, 0};
  int max_signal = 0;
  for (int a = 0; a < 5; a++) {
    cur_seq[0] = a;
    for (int b = 0; b < 5; b++) {
      if (b == a) {
        continue;
      }
      cur_seq[1] = b;
      for (int c = 0; c < 5; c++) {
        if (c == a || c == b) {
          continue;
        }
        cur_seq[2] = c;
        for (int d = 0; d < 5; d++) {
          if (d == a || d == b || d == c) {
            continue;
          }
          cur_seq[3] = d;
          for (int e = 0; e < 5; e++) {
            if (e == a || e == b || e == c || e == d) {
              continue;
            }
            cur_seq[4] = e;
            int signal = thruster_signal(vm, cur_seq);
            if (signal > max_signal) {
              max_signal = signal;
            }
          }
        }
      }
    }
  }
  return max_signal;
}

int feedback_thruster_signal(struct VirtualMachine *vms[], int input[],
                             size_t count) {
  eval_looped(vms, input, count);
  return pop(vms[0]->input);
}

int find_max_feedback_thruster_signal(struct VirtualMachine *vm) {
  struct VirtualMachine *vms[5];
  for (int i = 0; i < 5; i++) {
    vms[i] = malloc(sizeof(struct VirtualMachine));
    init_vm(vms[i]);
    memcpy(vms[i]->program, vm->program, PROG_CAP);
  }

  int cur_seq[] = {5, 6, 7, 8, 9};
  int max_signal = 0;
  for (int a = 5; a < 10; a++) {
    cur_seq[0] = a;
    for (int b = 5; b < 10; b++) {
      if (b == a) {
        continue;
      }
      cur_seq[1] = b;
      for (int c = 5; c < 10; c++) {
        if (c == a || c == b) {
          continue;
        }
        cur_seq[2] = c;
        for (int d = 5; d < 10; d++) {
          if (d == a || d == b || d == c) {
            continue;
          }
          cur_seq[3] = d;
          for (int e = 5; e < 10; e++) {
            if (e == a || e == b || e == c || e == d) {
              continue;
            }
            cur_seq[4] = e;

            for (int i = 0; i < 5; i++) {
              memcpy(vms[i]->program, vm->program, PROG_CAP);
            }

            int signal = feedback_thruster_signal(vms, cur_seq, 5);
            if (signal > max_signal) {
              max_signal = signal;
            }
          }
        }
      }
    }
  }
  return max_signal;
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
  INTCODE_CHECK_IO(vm_->program, 678, 5, 8684145);
  TEST_FIN(failed)
}

void test_maximize_1() {
  bool failed = false;
  struct VirtualMachine *vm = malloc(sizeof(struct VirtualMachine));
  init_vm(vm);
  int prog[] = {3, 15, 3, 16, 1002, 16, 10, 16, 1, 16, 15, 15, 4, 15, 99, 0, 0};
  memcpy(vm->program, prog, sizeof(prog));
  int seq[] = {4, 3, 2, 1, 0};
  assert_eq(thruster_signal(vm, seq), 43210);
  assert_eq(find_max_thruster_signal(vm), 43210);
  TEST_FIN(failed)
}

void test_day_7_part_1() {
  bool failed = false;
  struct VirtualMachine *vm = malloc(sizeof(struct VirtualMachine));
  init_vm(vm);
  FILE *fp = fopen("data/day_7_amplifier_input.txt", "r");
  parse_file(fp, vm);
  fclose(fp);
  assert_eq(find_max_thruster_signal(vm), 255840);
  TEST_FIN(failed)
}

void test_feedback_thruster_signal() {
  bool failed = false;
  int prog[] = {3,  26, 1001, 26, -4, 26, 3, 27, 1002, 27,
                2,  27, 1,    27, 26, 27, 4, 27, 1001, 28,
                -1, 28, 1005, 28, 6,  99, 0, 0,  5};
  int input[] = {9, 8, 7, 6, 5};
  struct VirtualMachine *vms[5];
  for (int i = 0; i < 5; i++) {
    vms[i] = malloc(sizeof(struct VirtualMachine));
    init_vm(vms[i]);
    memcpy(vms[i]->program, prog, sizeof(prog));
  }
  assert_eq(find_max_feedback_thruster_signal(vms[0]), 139629729);
  assert_eq(feedback_thruster_signal(vms, input, LENGTH(input)), 139629729);
  TEST_FIN(failed)
}

void test_day_7_part_2() {
  bool failed = false;
  struct VirtualMachine *vm = malloc(sizeof(struct VirtualMachine));
  init_vm(vm);
  FILE *fp = fopen("data/day_7_amplifier_input.txt", "r");
  parse_file(fp, vm);
  fclose(fp);
  assert_eq(find_max_feedback_thruster_signal(vm), 84088865);
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
  test_maximize_1();

  test_day_7_part_1();
  test_feedback_thruster_signal();
  test_day_7_part_2();

  printf("\n");
  printf("Ran %d tests", TEST_COUNT);
  if (TEST_FAILURES == 0) {
    printf(".\n"
           "All tests pass!\n");
  } else {
    printf(", got %d failures.\n", TEST_FAILURES);
    return 1;
  }
  return 0;
}
