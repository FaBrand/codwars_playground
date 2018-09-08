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
