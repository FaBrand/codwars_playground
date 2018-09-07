#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <unordered_map>
#include "simple_assembler_interpreter/src/assembler_main.h"

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
  std::vector<std::string> program{ "mov a 5" };
  std::unordered_map<std::string, int> out{ { "a", 5 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleMovRegisterProgram)
{
  std::vector<std::string> program{ "mov a 5", "mov a a" };
  std::unordered_map<std::string, int> out{ { "a", 5 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleIncProgram)
{
  std::vector<std::string> program{ "mov a 5", "inc a" };
  std::unordered_map<std::string, int> out{ { "a", 6 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleDecProgram)
{
  std::vector<std::string> program{ "mov a 5", "dec a" };
  std::unordered_map<std::string, int> out{ { "a", 4 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, ComplexMovDecIncProgram)
{
  std::vector<std::string> program{ "mov a 5", "dec a", "mov b a", "inc b", "mov c 6", "dec c" };
  std::unordered_map<std::string, int> out{ { "a", 4 },{ "b", 5 }, { "c", 5 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

TEST(SimpleAssembler_1, SimpleJnzProgram)
{
  std::vector<std::string> program{ "mov a 5", "dec a", "jnz a -1" };
  std::unordered_map<std::string, int> out{ { "a", 0 } };
  EXPECT_THAT(assembler(program), ::testing::ContainerEq(out));
}

