#include "day_9_intcode_4.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

Op from_raw_instruction(long raw_ins) {
  return static_cast<Op>(raw_ins % Args::OPCODE_SIZE);
}

std::size_t Args::arg_count(Op op) const {
  switch (op) {
  case Op::Add:
  case Op::Mul:
  case Op::LessThan:
  case Op::Equals:
    return 3;
  case Op::Input:
  case Op::Output:
  case Op::AddRelBase:
    return 1;
  case Op::JNZ:
  case Op::JZ:
    return 2;
  case Op::Halt:
    return 0;
  }
  std::cout << "what the fuck?? " << static_cast<int>(op) << std::endl;
  throw std::invalid_argument("Bad op in arg_count");
}

Args::Args(long raw_ins) {
  std::size_t arg_ct = arg_count(from_raw_instruction(raw_ins));
  raw_ins /= OPCODE_SIZE;
  for (size_t i = 0; i < arg_ct; i++) {
    modes.push_back(static_cast<Mode>(raw_ins % 10));
    raw_ins /= 10;
  }
}

Mode addr_mode(Mode mode) {
  switch (mode) {
  case Mode::Position:
  case Mode::Immediate:
    return Mode::Immediate;
  case Mode::Relative:
  case Mode::ImmediateRelative:
    return Mode::ImmediateRelative;
  }
  throw std::invalid_argument("Bad mode in addr_mode");
}

void Instruction::get_args(VirtualMachine &vm) {
  switch (this->op) {
  case Op::Add:
  case Op::Mul:
  case Op::LessThan:
  case Op::Equals:
    args.args.push_back(vm.arg(args.modes[0], vm.ip + 1));
    args.args.push_back(vm.arg(args.modes[1], vm.ip + 2));
    args.args.push_back(vm.arg(addr_mode(args.modes[2]), vm.ip + 3));
    break;
  case Op::Input:
    args.args.push_back(vm.arg(addr_mode(args.modes[0]), vm.ip + 1));
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
  program = std::vector<long>();
  input = std::deque<long>();
  output = std::deque<long>();
}

VirtualMachine::VirtualMachine(std::initializer_list<long> init)
    : VirtualMachine() {
  program_size = init.size();
  program.reserve(init.size() * 2);
  program.assign(init);
  ensure(program.capacity());
}

template <class InputIt>
VirtualMachine::VirtualMachine(InputIt first, InputIt last) : VirtualMachine() {
  std::copy(first, last, std::back_inserter(program));
  program_size = program.size();
  ensure(program.size() * 2);
}

void VirtualMachine::reset() {
  ip = 0;
  relative_base = 0;
  input.clear();
  output.clear();
}

void VirtualMachine::ensure(std::size_t size) {
  if (program.size() < size + 1) {
    program.resize(size + 1, 0);
  }
}

void VirtualMachine::trim_to_program() { program.resize(program_size); }

Instruction::Instruction(long raw_ins)
    : op(from_raw_instruction(raw_ins)), args(raw_ins) {}

const Instruction *VirtualMachine::ins() {
  // std::cout << "Raw: " << program[ip] << std::endl;
  Instruction *ins = new Instruction(program[ip]);
  ins->get_args(*this);
  return ins;
}

long VirtualMachine::arg(Mode mode, std::size_t idx) {
  long immediate = this->program[idx];
  switch (mode) {
  case Mode::Immediate:
    return immediate;
  case Mode::Position:
    ensure(immediate);
    return program.at(immediate);
  case Mode::Relative:
  case Mode::ImmediateRelative:
    std::size_t addr = immediate + relative_base;
    ensure(addr);
    return mode == Mode::Relative ? program.at(addr) : addr;
  }
  throw std::invalid_argument("Bad mode in VirtualMachine::arg");
}

Op VirtualMachine::step() {
  const Instruction *ins = this->ins();
  Args args = ins->args;
  // Move the instruction pointer to the next instruction.
  std::size_t new_ip = ip + args.size() + 1;
  // std::cout << "Current instruction: " << static_cast<int>(ins->op)
  // << ", args.size() = " << args.size() << std::endl;

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
    output.push_back(args[Args::Output::Val]);
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
    // std::cout << "new rel base: " << relative_base << std::endl;
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
    // std::cout << "hello" << std::endl;
  }
  // std::cout << "done" << std::endl;
}

std::vector<long> read_file(const std::string &filename) {
  std::basic_ifstream<char> fs = std::basic_ifstream<char>(filename);
  std::vector<long> prog;
  long num;
  char delimiter;
  while (fs >> num >> delimiter && delimiter == ',') {
    prog.push_back(num);
  }
  // Get the last number (there's no trailing comma)
  prog.push_back(num);
  return prog;
}

using namespace testing;

class IntcodeTest : public ::testing::Test {
protected:
  void assert_program(std::initializer_list<long> init_program,
                      std::initializer_list<long> final_program) {
    VirtualMachine vm(init_program);
    vm.eval();
    vm.trim_to_program();
    ASSERT_THAT(vm.program, ElementsAreArray(final_program));
  }

  void assert_io(VirtualMachine &vm, std::initializer_list<long> input,
                 std::initializer_list<long> output) {
    vm.input.assign(input);
    vm.eval();
    ASSERT_THAT(vm.output, ElementsAreArray(output));
  }

  void assert_io(std::initializer_list<long> program,
                 std::initializer_list<long> input,
                 std::initializer_list<long> output) {
    VirtualMachine vm(program);
    assert_io(vm, input, output);
  }

  void assert_io(std::initializer_list<long> program, long input, long output) {
    assert_io(program, {input}, {output});
  }
};

// Day 2 tests.
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

TEST_F(IntcodeTest, Simple5) {
  assert_program({1, 9, 10, 3, 2, 3, 11, 0, 99, 30, 40, 50},
                 {3500, 9, 10, 70, 2, 3, 11, 0, 99, 30, 40, 50});
}

// Day 5 tests.
TEST_F(IntcodeTest, EqualTo8Positional) {
  std::initializer_list<long> prog = {3, 9, 8, 9, 10, 9, 4, 9, 99, -1, 8};
  assert_io(prog, 8, 1);
  assert_io(prog, 7, 0);
  assert_io(prog, 9, 0);
  assert_io(prog, -3, 0);
}

TEST_F(IntcodeTest, EqualTo8Immediate) {
  std::initializer_list<long> prog = {3, 3, 1108, -1, 8, 3, 4, 3, 99};
  assert_io(prog, 8, 1);
  assert_io(prog, 7, 0);
  assert_io(prog, 9, 0);
  assert_io(prog, -3, 0);
}

TEST_F(IntcodeTest, LessThanEqual8Positional) {
  std::initializer_list<long> prog = {3, 9, 7, 9, 10, 9, 4, 9, 99, -1, 8};
  assert_io(prog, 8, 0);
  assert_io(prog, 7, 1);
  assert_io(prog, 9, 0);
  assert_io(prog, 10, 0);
  assert_io(prog, -3, 1);
}

TEST_F(IntcodeTest, LessThanEqual8Immediate) {
  std::initializer_list<long> prog = {3, 3, 1107, -1, 8, 3, 4, 3, 99};
  assert_io(prog, 8, 0);
  assert_io(prog, 7, 1);
  assert_io(prog, 9, 0);
  assert_io(prog, 10, 0);
  assert_io(prog, -3, 1);
}

TEST_F(IntcodeTest, JumpsPositional) {
  std::initializer_list<long> prog = {3,  12, 6,  12, 15, 1, 13, 14,
                                      13, 4,  13, 99, -1, 0, 1,  9};
  assert_io(prog, 0, 0);
  assert_io(prog, 1, 1);
  assert_io(prog, 993802, 1);
  assert_io(prog, -392, 1);
}

TEST_F(IntcodeTest, JumpsImmediate) {
  std::initializer_list<long> prog = {3, 3,  1105, -1, 9,  1101, 0,
                                      0, 12, 4,    12, 99, 1};
  assert_io(prog, 0, 0);
  assert_io(prog, 1, 1);
  assert_io(prog, 993802, 1);
  assert_io(prog, -392, 1);
}

TEST_F(IntcodeTest, ComplexIntcode) {
  std::initializer_list<long> prog = {
      3,    21,   1008, 21, 8,    20, 1005, 20,  22,  107,  8,    21,
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

// Day 9 tests.
TEST_F(IntcodeTest, Quine) {
  std::initializer_list<long> prog = {109,  1,   204, -1,  1001, 100, 1, 100,
                                      1008, 100, 16,  101, 1006, 101, 0, 99};
  assert_io(prog, {}, prog);
}

TEST_F(IntcodeTest, LargeNumbers) {
  long big_number = 1125899906842624;
  std::initializer_list<long> prog = {104, big_number, 99};
  assert_io(prog, {}, {big_number});
  assert_io({1102, 34915192, 34915192, 7, 4, 7, 99, 0}, {}, {1219070632396864});
}

// Day 2 solution.
TEST_F(IntcodeTest, GravityAssist) {
  auto prog = read_file("data/day_2_gravity_assist_input.txt");
  VirtualMachine vm = VirtualMachine(prog.begin(), prog.end());
  vm.program[1] = 12;
  vm.program[2] = 2;
  assert_io(vm, {}, {});
  ASSERT_EQ(vm.program[0], 6327510);
}

// Day 5 solution.
TEST_F(IntcodeTest, TESTProgram) {
  auto prog = read_file("data/day_5_TEST_input.txt");
  ASSERT_EQ(prog.size(), 678);
  VirtualMachine vm = VirtualMachine(prog.begin(), prog.end());
  // Part 1.
  assert_io(vm, {1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 14155342});
  vm.reset();
  vm.program.assign(prog.begin(), prog.end());
  // Part 2.
  assert_io(vm, {5}, {8684145});
}

TEST_F(IntcodeTest, BoostKeycode) {
  auto prog = read_file("data/day_9_boost_program.txt");
  VirtualMachine vm = VirtualMachine(prog.begin(), prog.end());
  assert_io(vm, {1}, {4234906522});
  vm = VirtualMachine(prog.begin(), prog.end());
  assert_io(vm, {2}, {60962});
}
