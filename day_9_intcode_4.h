#ifndef DAY_9_INTCODE_4_H
#define DAY_9_INTCODE_4_H

#include <deque>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

template <typename E> constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

class VirtualMachine;
class Instruction;

enum class Op {
  Add = 1,
  Mul = 2,
  Input = 3,
  Output = 4,
  JNZ = 5,
  JZ = 6,
  LessThan = 7,
  Equals = 8,
  AddRelBase = 9,
  Halt = 99,
};

Op from_raw_instruction(long raw_ins);

enum class Mode {
  Position = 0,
  Immediate = 1,
  Relative = 2,
  ImmediateRelative,
};

class Args {
  std::vector<Mode> modes;

public:
  const static long OPCODE_SIZE = 100;
  const static int MAX_ARGS_LEN = 3;

  std::vector<long> args;

  explicit Args(long);

  template <typename E> long operator[](E e) const {
    return args[static_cast<std::size_t>(e)];
  }
  std::size_t size() const { return args.size(); }

  std::size_t arg_count(Op op) const;

  enum class Add { LHS = 0, RHS, RetAddr };
  enum class Mul { LHS = 0, RHS, RetAddr };
  enum class Input { RetAddr = 0 };
  enum class Output { Val = 0 };
  enum class Jmp { Cmp = 0, RetAddr };
  enum class LessThan { LHS = 0, RHS, RetAddr };
  enum class Equals { LHS = 0, RHS, RetAddr };
  enum class AddRelBase { Val = 0 };
  friend class Instruction;
};

class VirtualMachine {
  std::size_t relative_base;
  std::size_t program_size;

  void ensure(std::size_t size);

public:
  std::vector<long> program;
  std::size_t ip;
  std::deque<long> input;
  std::deque<long> output;

  VirtualMachine();
  VirtualMachine(std::initializer_list<long> init);
  template <class InputIt> VirtualMachine(InputIt first, InputIt last);

  void reset();
  void eval();
  void trim_to_program();

protected:
  const long &operator[](std::size_t idx) const {
    return program[static_cast<std::size_t>(idx)];
  }
  const Instruction *ins();
  long arg(Mode mode, std::size_t idx);
  Op step();

  friend class Instruction;
};

class Instruction {
public:
  Op op;
  Args args;

  Instruction(long);
  void get_args(VirtualMachine &);
};

std::vector<long> read_file(const std::string &filename);

#endif
