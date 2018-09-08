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
    EXPECT_EQ(assembler_interpreter(program), "");
}

TEST(AssemblerInterpreter, BasicProgramSyntax)
{
    std::string program{R"(
; My first program
mov  a, 5
inc  a
)"};

    EXPECT_EQ(assembler_interpreter(program), "");
}

TEST(AssemblerInterpreter, BasicProgramSyntaxVariedEndParen)
{
    std::string program{R"(
; My first program
mov  a, 5
inc  a)"};
    EXPECT_EQ(assembler_interpreter(program), "");
}

TEST(AssemblerInterpreter, BasicProgramSyntaxVariedFrontParen)
{
    std::string program{R"( ; My first program
mov  a, 5
inc  a
)"};
    EXPECT_EQ(assembler_interpreter(program), "");
}

TEST(AssemblerInterpreter, MsgInstruction)
{
    std::string program{R"( ; My first program
mov  a, 5
msg 'Reg: ', a
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 5");
}

TEST(AssemblerInterpreter, MsgInstructionTrailingComment)
{
    std::string program{R"( ; My first program
mov  a, 5
msg 'Reg: ', a ; This  is a trailing comment
)"};
    EXPECT_EQ(assembler_interpreter(program), "Reg: 5");
}
