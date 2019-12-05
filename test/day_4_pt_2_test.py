#!/usr/bin/env python3
"""
A partial reimplementation of the befunge stack machine, to make writing
befunge programs easier.
"""
from typing import List, Tuple, Optional


class Stack:
    _s: List[int]

    def __init__(self, lst: Optional[List[int]] = None, debug: bool = False):
        self._s = []
        if lst:
            self._s.extend(lst)

    def log(self, msg: str):
        pass

    def push(self, digit: int):
        self._s.append(digit)

    def pop(self) -> int:
        return self._s.pop()

    def pop2(self) -> Tuple[int, int]:
        return self.pop(), self.pop()

    def add(self):
        self.push(self.pop() + self.pop())

    def sub(self):
        a, b = self.pop2()
        self.push(b - a)

    def mul(self):
        self.push(self.pop() * self.pop())

    def div(self):
        a, b = self.pop2()
        self.push(b // a)

    def mod(self):
        a, b = self.pop2()
        self.push(b % a)

    def inv(self):
        """Logical not.
        """
        self.push(1 if self.pop() == 0 else 0)

    def gt(self):
        a, b = self.pop2()
        self.push(1 if b > a else 0)

    def dup(self):
        self.push(self._s[-1])

    def swp(self):
        a, b = self.pop2()
        self.push(a)
        self.push(b)

    def list(self) -> List[int]:
        return list(self._s)

    def __repr__(self) -> str:
        return f"Stack({self._s})"


def test_stack():
    s = Stack()
    s.push(231832)
    s.push(0)
    assert s.list() == [231832, 0]
    assert 0 == s.pop()
    assert 231832 == s.pop()
    assert s.list() == []
    s.push(231832)
    s.push(0)
    s.swp()
    assert s.list() == [0, 231832]
    s.inv()
    assert s.list() == [0, 0]
    s.inv()
    assert s.list() == [0, 1]
    s.add()
    assert s.list() == [1]


def divide_by_10(s: Stack) -> Stack:
    s.push(5)
    s.dup()
    s.add()
    s.div()
    return s


def test_divide_by_10():
    assert divide_by_10(Stack([10])).list() == [1]
    assert divide_by_10(Stack([9])).list() == [0]
    assert divide_by_10(Stack([1, 2, 9])).list() == [1, 2, 0]
    assert divide_by_10(Stack([185039])).list() == [18503]


def check_last_2_digits_same(s: Stack) -> Stack:
    s.dup()
    # Push 2nd-to-last digit
    s.dup()
    s.push(10)
    s.div()
    s.push(10)
    s.mod()

    s.swp()
    # Replace top with last digit
    s.push(10)
    s.mod()

    s.sub()
    s.inv()
    s.inv()
    return s


def test_last_2():
    assert check_last_2_digits_same(Stack([1111])).list() == [1111, 0]
    assert check_last_2_digits_same(Stack([1110])).list() == [1110, 1]
    assert check_last_2_digits_same(Stack([8])).list() == [8, 1]
    assert check_last_2_digits_same(Stack([83])).list() == [83, 1]
    assert check_last_2_digits_same(Stack([33])).list() == [33, 0]
    assert check_last_2_digits_same(Stack([173349])).list() == [173349, 1]
    assert check_last_2_digits_same(Stack([173344])).list() == [173344, 0]


def has_adj_not_in_run(s: Stack) -> Stack:
    def push_in_run(s: Stack) -> Stack:
        """With ctx on the top of the stack, pushes is_run above it.
        """
        s.dup()
        s.push(2)
        s.div()
        return s

    def push_continue_run(s: Stack) -> Stack:
        """With ctx on the top of the stack, pushes continue_run above it.
        """
        s.dup()
        s.push(2)
        s.mod()
        return s

    # call the input int ORIG, and our modified copy N
    # s: [ORIG, ...]
    s.dup()
    # Context ctx
    s.push(3)#0b11
    #        ^ are we currently in a run?
    #         ^ are we currently in a 3-or-more-digit run?
    s.swp()
    # s: [N, ctx, ...]
    while True:
        # check gt 10
        s.dup()
        s.push(10)
        s.gt()
        if s.pop() == 1:
            # top > 10
            check_last_2_digits_same(s)
            if s.pop() == 0:
                # last 2 are same
                s.swp()
                push_in_run(s)
                if s.pop() == 0:
                    # At least 3 in a row
                    #  continue_run = True
                    s.pop()
                    s.push(0)
                else:
                    s.pop()
                    s.push(1)
            else:
                # last 2 not same
                # If we just saw *only* a pair, we're good!
                s.swp()  # Check in_run
                push_in_run(s)
                if s.pop() == 0:  # Check in_run
                    push_continue_run(s)
                    if s.pop() == 1:  # not continue_run
                        s.swp()
                        break
                s.pop()
                s.push(3)
        else:
            # top < 10
            break
        s.swp()
        # divide by 10 for next iteration
        s.push(10)
        s.div()

    # after break
    s.pop()

    push_in_run(s)
    if s.pop() == 0:  # Check in_run
        s.push(2)
        s.mod()  # continue_run at top of stack now
        s.inv()  # push 0 if not continue_run else push 1, effectively
    else:
        s.pop()
        s.push(1)
    return s


def test_check_adjacent():
    assert has_adj_not_in_run(Stack([231832])).list() == [231832, 1]
    assert has_adj_not_in_run(Stack([231132])).list() == [231132, 0]
    assert has_adj_not_in_run(Stack([231112])).list() == [231112, 1]
    assert has_adj_not_in_run(Stack([23111])).list() == [23111, 1]
    assert has_adj_not_in_run(Stack([22111])).list() == [22111, 0]
