#include <string>
#include <unordered_map>
#include <vector>
#include "assembler_interpreter/src/assembler_main.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ContainerEq;

TEST(AssemblerInterpreter, EmptyProgram)
{
    std::string program{};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, BasicProgramSyntax)
{
    std::string program{R"(
; My first program
mov  a, 5
inc  a
)"};

    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, BasicProgramSyntaxVariedEndParen)
{
    std::string program{R"(
; My first program
mov  a, 5
inc  a)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, BasicProgramSyntaxVariedFrontParen)
{
    std::string program{R"( ; My first program
mov  a, 5
inc  a
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, MsgInstruction)
{
    std::string program{R"( ; My first program
mov  a, 5
msg 'Reg: ', a
end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 5");
}

TEST(AssemblerInterpreter, MsgInstructionTrailingComment)
{
    std::string program{R"( ; My first program
mov  a, 5
msg 'Reg: ', a ; This  is a trailing comment
end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 5");
}

TEST(AssemblerInterpreter, MsgOnlyWrittenIfEndIsExecuted)
{
    std::string program{R"( ; My first program
mov  a, 5
msg 'Reg: ', a
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, LabelDefinition)
{
    std::string program{R"(
Function:
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, CallInstruction)
{
    std::string program{R"( ; My first program
mov  a, 5
call Func
msg 'Reg: ', a ; This  is a trailing comment
end
Func:
    inc a
    ret
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 6");
}

TEST(AssemblerInterpreter, NestedCalls)
{
    std::string program{R"( ; My first program
mov  a, 5
call Func
msg 'Reg: ', a ; This  is a trailing comment
end

Func:
    inc a
    call OtherFunc
    ret

OtherFunc:
    inc a
    ret
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 7");
}

TEST(AssemblerInterpreter, CmpOperatorWithConstants)
{
    std::string program{R"(
cmp 1, 1
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, CmpOperatorWithOneRegister)
{
    std::string program{R"(
mov a, 1
cmp a, 1
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, CmpOperatorWithTwoRegisters)
{
    std::string program{R"(
mov a, 1
cmp a, a
)"};
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(AssemblerInterpreter, JumpIfEqualWhenEqual)
{
    std::string program{R"(
mov a, 1
cmp 1, 1
je label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfEqualWhenLess)
{
    std::string program{R"(
mov a, 1
cmp 1, 2
je label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfEqualWhenGreater)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
je label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfNotEqualWhenLess)
{
    std::string program{R"(
mov a, 1
cmp 1, 2
jne label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfNotEqualWhenGreater)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
jne label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfNotEqualWhenEqual)
{
    std::string program{R"(
mov a, 1
cmp 1, 1
jne label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfGreaterOrEqualWhenEqual)
{
    std::string program{R"(
mov a, 1
cmp 1, 1
jge label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfGreaterOrEqualWhenGreater)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
jge label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfGreaterOrEqualWhenLess)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
jge label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfGreaterWhenGreater)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
jg label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfGreaterWhenEqual)
{
    std::string program{R"(
mov a, 1
cmp 1, 1
jg label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfGreaterWhenLess)
{
    std::string program{R"(
mov a, 1
cmp 1, 2
jg label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfLessOrEqualWhenGreater)
{
    std::string program{R"(
mov a, 1
cmp 2, 1
jle label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 1");
}

TEST(AssemblerInterpreter, JumpIfLessOrEqualWhenEqual)
{
    std::string program{R"(
mov a, 1
cmp 1, 1
jle label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(AssemblerInterpreter, JumpIfLessOrEqualWhenLess)
{
    std::string program{R"(
mov a, 1
cmp 1, 2
jle label
msg 'Reg: ', a
end

label:
    inc a
    msg 'Reg: ', a
    end
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 2");
}

TEST(SampleTests, Tests1)
{
    std::string program = R"(
; My first program
mov  a, 5
inc  a
call function
msg  '(5+1)/2 = ', a    ; output message
end

function:
    div  a, 2
    ret)";
    EXPECT_EQ(assembler_interpreter(program), "(5+1)/2 = 3");
}

TEST(SampleTests, Tests2)
{
    std::string program = R"(
mov   a, 5
mov   b, a
mov   c, a
call  proc_fact
call  print
end

proc_fact:
    dec   b
    mul   c, b
    cmp   b, 1
    jne   proc_fact
    ret

print:
    msg   a, '! = ', c ; output text
    ret
)";
    EXPECT_EQ(assembler_interpreter(program), "5! = 120");
}

TEST(SampleTests, Tests3)
{
    std::string program = R"(
mov   a, 8            ; value
mov   b, 0            ; next
mov   c, 0            ; counter
mov   d, 0            ; first
mov   e, 1            ; second
call  proc_fib
call  print
end

proc_fib:
    cmp   c, 2
    jl    func_0
    mov   b, d
    add   b, e
    mov   d, e
    mov   e, b
    inc   c
    cmp   c, a
    jle   proc_fib
    ret

func_0:
    mov   b, c
    inc   c
    jmp   proc_fib

print:
    msg   'Term ', a, ' of Fibonacci series is: ', b        ; output text
    ret)";
    EXPECT_EQ(assembler_interpreter(program), "Term 8 of Fibonacci series is: 21");
}

TEST(SampleTests, Tests4)
{
    std::string program = R"(
mov   a, 11           ; value1
mov   b, 3            ; value2
call  mod_func
msg   'mod(', a, ', ', b, ') = ', d        ; output
end

; Mod function
mod_func:
    mov   c, a        ; temp1
    div   c, b
    mul   c, b
    mov   d, a        ; temp2
    sub   d, c
    ret)";
    EXPECT_EQ(assembler_interpreter(program), "mod(11, 3) = 2");
}

TEST(SampleTests, Tests5)
{
    std::string program = R"(
mov   a, 81         ; value1
mov   b, 153        ; value2
call  init
call  proc_gcd
call  print
end

proc_gcd:
    cmp   c, d
    jne   loop
    ret

loop:
    cmp   c, d
    jg    a_bigger
    jmp   b_bigger

a_bigger:
    sub   c, d
    jmp   proc_gcd

b_bigger:
    sub   d, c
    jmp   proc_gcd

init:
    cmp   a, 0
    jl    a_abs
    cmp   b, 0
    jl    b_abs
    mov   c, a            ; temp1
    mov   d, b            ; temp2
    ret

a_abs:
    mul   a, -1
    jmp   init

b_abs:
    mul   b, -1
    jmp   init

print:
    msg   'gcd(', a, ', ', b, ') = ', c
    ret)";
    EXPECT_EQ(assembler_interpreter(program), "gcd(81, 153) = 9");
}

TEST(SampleTests, Tests6)
{
    std::string program = R"(
call  func1
call  print
end

func1:
    call  func2
    ret

func2:
    ret

print:
    msg 'This program should return -1')";
    EXPECT_EQ(assembler_interpreter(program), "-1");
}

TEST(SampleTests, Tests7)
{
    std::string program = R"(
mov   a, 2            ; value1
mov   b, 10           ; value2
mov   c, a            ; temp1
mov   d, b            ; temp2
call  proc_func
call  print
end

proc_func:
    cmp   d, 1
    je    continue
    mul   c, a
    dec   d
    call  proc_func

continue:
    ret

print:
    msg a, '^', b, ' = ', c
    ret)";
    EXPECT_EQ(assembler_interpreter(program), "2^10 = 1024");
}
