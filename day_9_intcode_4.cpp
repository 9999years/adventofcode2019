#include "day_9_intcode_4.h"
#include "/home/aquatica/projects/adventofcode2019/etc/day_9_build/googletest-src/googlemock/include/gmock/gmock-matchers.h"
#include "boost/range/irange.hpp"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <initializer_list>
#include <iostream>

Args::Args(int raw_ins) {
  raw_ins /= OPCODE_SIZE;
  for ([[maybe_unused]] int _ : boost::irange(0, MAX_ARGS_LEN)) {
    modes.push_back(static_cast<Mode>(raw_ins % 10));
    raw_ins /= 10;
  }
}

void Instruction::get_args(VirtualMachine &vm) {
  switch (this->op) {
  case Op::Add:
  case Op::Mul:
  case Op::LessThan:
  case Op::Equals:
    args.args.push_back(vm.arg(args.modes[0], vm.ip + 1));
    args.args.push_back(vm.arg(args.modes[1], vm.ip + 2));
    args.args.push_back(vm.arg(Mode::Immediate, vm.ip + 3));
    break;
  case Op::Input:
    args.args.push_back(vm.arg(Mode::Immediate, vm.ip + 1));
    break;
  case Op::Output:
  case Op::AddRelBase:
    args.args.push_back(vm.arg(args.modes[0], vm.ip + 1));
    break;
  case Op::JNZ:
  case Op::JZ:
    args.args.push_back(vm.arg(args.modes[0], vm.ip + 1));
    args.args.push_back(vm.arg(args.modes[1], vm.ip + 2));
    break;
  case Op::Halt:
    break;
  }
}

VirtualMachine::VirtualMachine() {
  ip = 0;
  relative_base = 0;
  program = std::vector<int>();
  input = std::deque<int>();
  output = std::deque<int>();
}

VirtualMachine::VirtualMachine(std::initializer_list<int> init)
    : VirtualMachine() {
  program.assign(init);
}

void VirtualMachine::reset() {
  ip = 0;
  input.clear();
  output.clear();
}

Instruction::Instruction(int raw_ins)
    : op(static_cast<Op>(raw_ins % Args::OPCODE_SIZE)), args(raw_ins) {}

const Instruction *VirtualMachine::ins() {
  Instruction *ins = new Instruction(program[ip]);
  ins->get_args(*this);
  return ins;
}

int VirtualMachine::arg(Mode mode, std::size_t idx) const {
  int immediate = this->program[idx];
  int val;
  switch (mode) {
  case Mode::Immediate:
    val = immediate;
    break;
  case Mode::Position:
    val = program[immediate];
    break;
  case Mode::Relative:
    val = program[immediate + relative_base];
    break;
  }
  return val;
}

Op VirtualMachine::step() {
  const Instruction *ins = this->ins();
  Args args = ins->args;
  // Move the instruction pointer to the next instruction.
  std::size_t new_ip = ip + args.size() + 1;

  switch (ins->op) {
  case Op::Add:
    program[args[Args::Add::RetAddr]] =
        args[Args::Add::LHS] + args[Args::Add::RHS];
    break;
  case Op::Mul:
    program[args[Args::Mul::RetAddr]] =
        args[Args::Mul::LHS] * args[Args::Mul::RHS];
    break;
  case Op::Input:
    program[args[Args::Input::RetAddr]] = input.back();
    input.pop_back();
    break;
  case Op::Output:
    output.push_front(args[Args::Output::Val]);
    break;
  case Op::JNZ:
  case Op::JZ:
    if ((args[Args::Jmp::Cmp] == 0) ^ (ins->op == Op::JNZ)) {
      new_ip = args[Args::Jmp::RetAddr];
    }
    break;
  case Op::LessThan:
    program[args[Args::LessThan::RetAddr]] =
        args[Args::LessThan::LHS] < args[Args::LessThan::RHS] ? 1 : 0;
    break;
  case Op::Equals:
    program[args[Args::Equals::RetAddr]] =
        args[Args::Equals::LHS] == args[Args::Equals::RHS] ? 1 : 0;
    break;
  case Op::AddRelBase:
    relative_base += args[Args::AddRelBase::Val];
    break;
  case Op::Halt:
    break;
  }

  ip = new_ip;
  return ins->op;
}

void VirtualMachine::eval() {
  while (step() != Op::Halt) {
    // 'step' performs the work
  }
}

using namespace testing;

class IntcodeTest : public ::testing::Test {
protected:
  void assert_program(std::initializer_list<int> init_program,
                      std::initializer_list<int> final_program) {
    VirtualMachine vm(init_program);
    vm.eval();
    ASSERT_THAT(vm.program, ElementsAreArray(final_program));
  }

  void assert_io(std::initializer_list<int> program, int input, int output) {
    VirtualMachine vm(program);
    vm.input.push_back(input);
    vm.eval();
    ASSERT_THAT(vm.output, ElementsAreArray({output}));
  }
};

TEST_F(IntcodeTest, Simple1) {
  assert_program({2, 3, 0, 3, 99}, {2, 3, 0, 6, 99});
}

TEST_F(IntcodeTest, Simple2) {
  assert_program({2, 4, 4, 5, 99, 0}, {2, 4, 4, 5, 99, 9801});
}

TEST_F(IntcodeTest, Simple3) {
  assert_program({1, 0, 0, 0, 99}, {2, 0, 0, 0, 99});
}

TEST_F(IntcodeTest, Simple4) {
  assert_program({1, 1, 1, 4, 99, 5, 6, 0, 99}, {30, 1, 1, 4, 2, 5, 6, 0, 99});
}

TEST_F(IntcodeTest, EqualTo8Positional) {
  auto prog = {3, 9, 8, 9, 10, 9, 4, 9, 99, -1, 8};
  assert_io(prog, 8, 1);
  assert_io(prog, 7, 0);
  assert_io(prog, 9, 0);
  assert_io(prog, -3, 0);
}

TEST_F(IntcodeTest, EqualTo8Immediate) {
  auto prog = {3, 3, 1108, -1, 8, 3, 4, 3, 99};
  assert_io(prog, 8, 1);
  assert_io(prog, 7, 0);
  assert_io(prog, 9, 0);
  assert_io(prog, -3, 0);
}

TEST_F(IntcodeTest, LessThanEqual8Positional) {
  auto prog = {3, 9, 7, 9, 10, 9, 4, 9, 99, -1, 8};
  assert_io(prog, 8, 0);
  assert_io(prog, 7, 1);
  assert_io(prog, 9, 0);
  assert_io(prog, 10, 0);
  assert_io(prog, -3, 1);
}

TEST_F(IntcodeTest, LessThanEqual8Immediate) {
  auto prog = {3, 3, 1107, -1, 8, 3, 4, 3, 99};
  assert_io(prog, 8, 0);
  assert_io(prog, 7, 1);
  assert_io(prog, 9, 0);
  assert_io(prog, 10, 0);
  assert_io(prog, -3, 1);
}

TEST_F(IntcodeTest, JumpsPositional) {
  auto prog = {3, 12, 6, 12, 15, 1, 13, 14, 13, 4, 13, 99, -1, 0, 1, 9};
  assert_io(prog, 0, 0);
  assert_io(prog, 1, 1);
  assert_io(prog, 993802, 1);
  assert_io(prog, -392, 1);
}

TEST_F(IntcodeTest, JumpsImmediate) {
  auto prog = {3, 3, 1105, -1, 9, 1101, 0, 0, 12, 4, 12, 99, 1};
  assert_io(prog, 0, 0);
  assert_io(prog, 1, 1);
  assert_io(prog, 993802, 1);
  assert_io(prog, -392, 1);
}

TEST_F(IntcodeTest, ComplexIntcode) {
  auto prog = {3,    21,   1008, 21, 8,    20, 1005, 20,  22,  107,  8,    21,
               20,   1006, 20,   31, 1106, 0,  36,   98,  0,   0,    1002, 21,
               125,  20,   4,    20, 1105, 1,  46,   104, 999, 1105, 1,    46,
               1101, 1000, 1,    20, 4,    20, 1105, 1,   46,  98,   99};
  assert_io(prog, -3, 999);
  assert_io(prog, 0, 999);
  assert_io(prog, 1, 999);
  assert_io(prog, 7, 999);
  assert_io(prog, 8, 1000);
  assert_io(prog, 9, 1001);
  assert_io(prog, 10, 1001);
  assert_io(prog, 1039, 1001);
  assert_io(prog, 4895096, 1001);
}
