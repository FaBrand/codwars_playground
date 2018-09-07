#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using RawProgram = std::vector<std::string>;
using Registers = std::unordered_map<std::string, int>;
class Instruction;
using Instruction_ptr = std::unique_ptr<Instruction>;
using Program = std::vector<Instruction_ptr>;
using ProgramPtr = Program::iterator;

bool is_register(std::string const& val)
{
    const auto result{std::find_if(val.begin(), val.end(), [](auto const& c) { return std::isalpha(c); })};
    return result != val.end();
}

class ValueResolver
{
  public:
    ValueResolver(Registers* registers) : registers_{registers} {}
    int get_value_of(std::string const& in)
    {
        if (is_register(in))
        {
            return registers_->at(in);
        }
        else
        {
            const auto value_as_int{std::stoi(in)};
            return value_as_int;
        }
    };

  private:
    Registers* registers_{nullptr};
};

class InstructionFactory
{
  public:
    InstructionFactory(Registers&);
    Instruction_ptr create_instruction(std::string const& name, std::vector<std::string> const& arguments);

  private:
    template <typename T>
    std::unique_ptr<T> make_instruction(std::vector<std::string> const& tokens)
    {
        auto new_instruction{std::make_unique<T>(tokens)};
        new_instruction->set_resolver(&value_resolver_);
        return std::move(new_instruction);
    }

    std::unordered_map<std::string, std::function<Instruction_ptr(std::vector<std::string>)>> instruction_map_{};
    Registers& registers_;
    ValueResolver value_resolver_{&registers_};
};

class Machine
{
  public:
    void load_program(RawProgram const& prog);
    void run_program();
    int const& get_register(std::string const&) const;
    int& get_register(std::string const&);
    Registers const& get_registers() const;
    void advance_ip(std::ptrdiff_t diff);

  private:
    std::vector<std::string> split_tokens(std::string const& command);
    Instruction& get_current_instruction() const;
    Registers registers_{};
    ProgramPtr ip_{program_.begin()};
    Program program_{};
    InstructionFactory instruction_factory_{registers_};
};

class Instruction
{
  public:
    ~Instruction() = default;
    void set_resolver(ValueResolver* resolver)
    {
        value_resolver_ = resolver;
    }
    virtual void operate_on(Machine& machine) = 0;

  protected:
    std::string name_{};
    ValueResolver* value_resolver_{nullptr};
};

class UnaryInstruction : public Instruction
{
  public:
    UnaryInstruction(std::vector<std::string> const& tokens) : Instruction(), register_{tokens.at(0)} {}
    ~UnaryInstruction() = default;

  protected:
    std::string register_{};
};

class BinaryInstruction : public Instruction
{
  public:
    BinaryInstruction(std::vector<std::string> const& tokens)
        : Instruction(), register_{tokens.at(0)}, value_{tokens.at(1)}
    {
    }
    ~BinaryInstruction() = default;

  protected:
    std::string register_{};
    std::string value_{};
};

class Mov : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

class Inc : public UnaryInstruction
{
  public:
    using UnaryInstruction::UnaryInstruction;
    void operate_on(Machine& machine) override;
};

class Dec : public UnaryInstruction
{
  public:
    using UnaryInstruction::UnaryInstruction;
    void operate_on(Machine& machine) override;
};

class Jnz : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;

  private:
    int calculate_jump_distance();
};

void Mov::operate_on(Machine& machine)
{
    machine.get_register(register_) = value_resolver_->get_value_of(value_);
}

void Inc::operate_on(Machine& machine)
{
    ++(machine.get_register(register_));
}

void Dec::operate_on(Machine& machine)
{
    --(machine.get_register(register_));
}

int Jnz::calculate_jump_distance()
{
    return value_resolver_->get_value_of(value_);
}

void Jnz::operate_on(Machine& machine)
{
    const int jump_condition{value_resolver_->get_value_of(register_)};
    if (jump_condition != 0)
    {
        const std::ptrdiff_t jump_distance{calculate_jump_distance()};
        machine.advance_ip(jump_distance);
    }
}

InstructionFactory::InstructionFactory(Registers& registers) : registers_{registers}
{
    instruction_map_.emplace("mov", [this](auto const& tokens) { return make_instruction<Mov>(tokens); });
    instruction_map_.emplace("jnz", [this](auto const& tokens) { return make_instruction<Jnz>(tokens); });
    instruction_map_.emplace("inc", [this](auto const& tokens) { return make_instruction<Inc>(tokens); });
    instruction_map_.emplace("dec", [this](auto const& tokens) { return make_instruction<Dec>(tokens); });
}

Instruction_ptr InstructionFactory::create_instruction(std::string const& name,
                                                       std::vector<std::string> const& arguments)
{
    return instruction_map_.at(name)(arguments);
}

Instruction& Machine::get_current_instruction() const
{
    auto& is{*(ip_->get())};
    return is;
}

void Machine::advance_ip(std::ptrdiff_t diff)
{
    const auto new_position{next(ip_, diff - 1)};
    const bool is_after_begin{new_position >= program_.begin()};
    const bool is_before_end{new_position <= program_.end()};
    const bool is_new_position_in_range{is_after_begin && is_before_end};
    if (is_new_position_in_range)
    {
        std::advance(ip_, --diff);
    }
    else
    {
        std::advance(ip_, 1);
    }
}

void Machine::load_program(RawProgram const& prog)
{
    for (auto const& instruction : prog)
    {
        auto tokens{split_tokens(instruction)};
        program_.push_back(
            instruction_factory_.create_instruction(tokens.front(), {std::next(tokens.begin(), 1), tokens.end()}));
    }
    ip_ = program_.begin();
}

void Machine::run_program()
{
    while (ip_ != program_.end())
    {
        get_current_instruction().operate_on(*this);
        std::advance(ip_, 1);
    }
}

std::vector<std::string> Machine::split_tokens(std::string const& command)
{
    std::istringstream splitter(command);
    return std::vector<std::string>((std::istream_iterator<std::string>(splitter)),
                                    std::istream_iterator<std::string>());
}
int const& Machine::get_register(std::string const& register_name) const
{
    return registers_.at(register_name);
}

int& Machine::get_register(std::string const& register_name)
{
    return registers_[register_name];
}

Registers const& Machine::get_registers() const
{
    return registers_;
}

// static int& getReg(Registers& regs, std::string name)
// {
//     return regs.at(name);
// }

Registers assembler(RawProgram const& program)
{
    Machine machine{};
    machine.load_program(program);
    machine.run_program();
    return machine.get_registers();
}
