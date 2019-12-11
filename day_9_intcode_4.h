#ifndef DAY_9_INTCODE_4_H
#define DAY_9_INTCODE_4_H

#include <deque>
#include <type_traits>
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
  Halt = 99,
};

enum class Mode {
  Position = 0,
  Immediate = 1,
  Relative = 2,
};

class Args {
  std::vector<Mode> modes;

public:
  const static int OPCODE_SIZE = 100;
  const static int MAX_ARGS_LEN = 3;

  std::vector<int> args;

  Args(int);

  template <typename E> int operator[](E e) const {
    return args[static_cast<std::size_t>(e)];
  }
  std::size_t size() const { return args.size(); }

  enum class Add {
    LHS = 0,
    RHS,
    RetAddr,
  };

  enum class Mul {
    LHS = 0,
    RHS,
    RetAddr,
  };

  enum class Input { RetAddr = 0 };

  enum class Output { Val = 0 };

  enum class JNZ {
    Cmp = 0,
    RetAddr,
  };

  enum class JZ {
    Cmp = 0,
    RetAddr,
  };

  enum class LessThan { LHS = 0, RHS, RetAddr };

  enum class Equals { LHS = 0, RHS, RetAddr };

  friend class Instruction;
};

class VirtualMachine {
  std::size_t relative_base;

public:
  std::vector<int> program;
  std::size_t ip;
  std::deque<int> input;
  std::deque<int> output;

  VirtualMachine();

  void reset();
  void eval();

protected:
  template <typename E> const int &operator[](E e) const {
    return program[static_cast<std::size_t>(e)];
  }
  template <typename E> int &operator[](E e) {
    return program[static_cast<std::size_t>(e)];
  }
  const Instruction *ins();
  int arg(Mode mode, std::size_t idx) const;
  Op step();

  friend class Instruction;
};

class Instruction {
public:
  Op op;
  Args args;

  Instruction(int);
  void get_args(VirtualMachine &);
};
#endif
