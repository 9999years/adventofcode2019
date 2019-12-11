#include "day_9_intcode_4.h"
#include "boost/range/irange.hpp"
#include "gtest/gtest.h"
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
  input = std::deque<int>();
  output = std::deque<int>();
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
  case Mode::Relative:
    val = program[immediate + relative_base];
  }
  return val;
}

Op VirtualMachine::step() {
  const Instruction *ins = this->ins();
  Args args = ins->args;
  std::size_t new_ip = ip + args.size() + 1;
  switch (ins->op) {
  case Op::Add:
    this[Args::Add::RetAddr] = args[Args::Add::LHS] + args[Args::Add::RHS];
    break;
  case Op::Mul:
    break;
  case Op::Input:
    break;
  case Op::Output:
    break;
  case Op::JNZ:
    break;
  case Op::JZ:
    break;
  case Op::LessThan:
    break;
  case Op::Equals:
    break;
  case Op::Halt:
    break;
  }
  return ins->op;
}

void VirtualMachine::eval() {
  while (step() != Op::Halt) {
    // 'step' performs the work
  }
}

// TEST(SmokeTest, SmokeTest) {
// ASSERT_EQ(1, 0);
// }
