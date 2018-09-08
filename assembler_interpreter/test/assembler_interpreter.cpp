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
    std::cout << "here";
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
