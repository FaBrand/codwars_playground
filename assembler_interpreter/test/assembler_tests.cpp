#include <string>
#include <unordered_map>
#include <vector>
#include "assembler_interpreter/src/assembler_main.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ContainerEq;

// TODO: Replace examples and use TDD development by writing your own tests
TEST(SimpleAssembler_1, EmptyProgram)
{
    std::vector<std::string> program{};
    std::unordered_map<std::string, int> out{};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleMovValueProgram)
{
    std::vector<std::string> program{"mov a 5"};
    std::unordered_map<std::string, int> out{{"a", 5}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleMovRegisterProgram)
{
    std::vector<std::string> program{"mov a 5", "mov a a"};
    std::unordered_map<std::string, int> out{{"a", 5}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleIncProgram)
{
    std::vector<std::string> program{"mov a 5", "inc a"};
    std::unordered_map<std::string, int> out{{"a", 6}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleDecProgram)
{
    std::vector<std::string> program{"mov a 5", "dec a"};
    std::unordered_map<std::string, int> out{{"a", 4}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, ComplexMovDecIncProgram)
{
    std::vector<std::string> program{"mov a 5", "dec a", "mov b a", "inc b", "mov c 6", "dec c"};
    std::unordered_map<std::string, int> out{{"a", 4}, {"b", 5}, {"c", 5}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleJnzProgram)
{
    std::vector<std::string> program{"mov a 5", "dec a", "jnz a -1"};
    std::unordered_map<std::string, int> out{{"a", 0}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, Complex1Fake)
{
    std::vector<std::string> program{"mov a 1",  "mov b 1",  "mov c 0", "mov d 26", "jnz c 2",  "jnz 1 5",
                                     "mov c 7",  "inc d",    "dec c",   "jnz c -2", "mov c a",  "inc a",
                                     "dec b",    "jnz b -2", "mov b c", "dec d",    "jnz d -6", "mov c 18",
                                     "mov d 11", "inc a",    "dec d",   "jnz d -2", "dec c",    "jnz c -5"};
    assembler(program);
}

TEST(Problematic, Complex2Fake)
{
    std::vector<std::string> program{
        "mov d 100", "dec d", "mov b d", "jnz b -2", "inc d", "mov a d", "jnz 5 10", "mov c a"};
    assembler(program);
}

TEST(SimpleAssembler_1, Complex3Fake)
{
    std::vector<std::string> program{"mov c 12",
                                     "mov b 0",
                                     "mov a 200",
                                     "dec a",
                                     "inc b",
                                     "jnz a -2",
                                     "dec c",
                                     "mov a b",
                                     "jnz c -5",
                                     "jnz 0 1",
                                     "mov c a"};
    assembler(program);
}

TEST(SimpleAssembler_1, SimpleAdd)
{
    std::vector<std::string> program{"mov a 5", "add a 1"};
    std::unordered_map<std::string, int> out{{"a", 6}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleSub)
{
    std::vector<std::string> program{"mov a 5", "sub a 1"};
    std::unordered_map<std::string, int> out{{"a", 4}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleMul)
{
    std::vector<std::string> program{"mov a 5", "mul a 2"};
    std::unordered_map<std::string, int> out{{"a", 10}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleDiv)
{
    std::vector<std::string> program{"mov a 4", "div a 2"};
    std::unordered_map<std::string, int> out{{"a", 2}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleDivFloor)
{
    std::vector<std::string> program{"mov a 5", "div a 2"};
    std::unordered_map<std::string, int> out{{"a", 2}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, IndentedOperatorIsRead)
{
    std::vector<std::string> program{"  mov a 5", "  div a 2"};
    std::unordered_map<std::string, int> out{{"a", 2}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, EndInstructionEndsProgramPremature)
{
    std::vector<std::string> program{"mov a 5", "end", "div a 2"};
    std::unordered_map<std::string, int> out{{"a", 5}};
    EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}
