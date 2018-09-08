#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <regex>
#include <sstream>
#include <stack>
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

enum CmpStatusFlags : unsigned int
{
    Invalid = 0,
    Equal = 0b000001,
    NotEqual = 0b000010,
    GreaterOrEqual = 0b000100,
    Greater = 0b001000,
    LessOrEqual = 0b010000,
    Less = 0b100000
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
    void end_execution();
    std::string flush();
    void add_label_reference(std::string name);
    void jump_to(std::string name);
    void _return();
    void set_comparison_status_flag(CmpStatusFlags new_status);
    void jump_if_flag_is_set(std::string label, CmpStatusFlags flag);

  public:
    std::stringstream msg_port{};

  private:
    void pre_run();

    std::stringstream default_out{"-1"};
    std::stringstream* std_out{&default_out};
    std::vector<std::string> split_tokens(std::string const& command);
    Instruction& get_current_instruction() const;
    CmpStatusFlags comparison_status_register_{CmpStatusFlags::Invalid};
    Registers registers_{};
    std::unordered_map<std::string, ProgramPtr> label_map_{};
    ProgramPtr ip_{program_.begin()};
    Program program_{};
    InstructionFactory instruction_factory_{registers_};
    std::stack<ProgramPtr> jump_stack_{};
};

class Instruction
{
  public:
    ~Instruction() = default;
    void set_resolver(ValueResolver* resolver)
    {
        value_resolver_ = resolver;
    }
    virtual void pre_run(Machine& machine) {}
    virtual void operate_on(Machine& machine) = 0;

  protected:
    std::string name_{};
    ValueResolver* value_resolver_{nullptr};
};

class NullaryInstruction : public Instruction
{
  public:
    NullaryInstruction(std::vector<std::string> const& tokens) : Instruction() {}
    ~NullaryInstruction() = default;
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

class NaryInstruction : public Instruction
{
  public:
    NaryInstruction(std::vector<std::string> const& tokens) : Instruction(), arguments_{tokens} {}
    ~NaryInstruction() = default;

  protected:
    std::vector<std::string> arguments_{};
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

class Add : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

void Add::operate_on(Machine& machine)
{
    machine.get_register(register_) = machine.get_register(register_) + value_resolver_->get_value_of(value_);
}

class Sub : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

void Sub::operate_on(Machine& machine)
{
    machine.get_register(register_) = machine.get_register(register_) - value_resolver_->get_value_of(value_);
}

class Mul : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

void Mul::operate_on(Machine& machine)
{
    machine.get_register(register_) = machine.get_register(register_) * value_resolver_->get_value_of(value_);
}

class Div : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

void Div::operate_on(Machine& machine)
{
    machine.get_register(register_) = machine.get_register(register_) / value_resolver_->get_value_of(value_);
}

class End : public NullaryInstruction
{
  public:
    using NullaryInstruction::NullaryInstruction;
    void operate_on(Machine& machine) override;
};

void End::operate_on(Machine& machine)
{
    machine.end_execution();
}

class Msg : public NaryInstruction
{
  public:
    using NaryInstruction::NaryInstruction;
    void operate_on(Machine& machine) override;

  private:
    static bool is_arg_text(std::string const& arg);
    static std::string strippedQuotes(std::string arg);
};
bool Msg::is_arg_text(std::string const& arg)
{
    return arg.find("'") != std::string::npos;
}
std::string Msg::strippedQuotes(std::string arg)
{
    arg.erase(std::remove(arg.begin(), arg.end(), '\''), arg.end());
    return arg;
}

void Msg::operate_on(Machine& machine)
{
    std::transform(arguments_.begin(),
                   arguments_.end(),
                   std::ostream_iterator<std::string>(machine.msg_port, ""),
                   [this](auto const& arg) {
                       if (Msg::is_arg_text(arg))
                       {
                           return Msg::strippedQuotes(arg);
                       }
                       else
                       {
                           return std::to_string(value_resolver_->get_value_of(arg));
                       }
                   });
}

class Label : public UnaryInstruction
{
  public:
    using UnaryInstruction::UnaryInstruction;
    void pre_run(Machine& machine) override;
    void operate_on(Machine& machine) override;
};

void Label::pre_run(Machine& machine)
{
    machine.add_label_reference(register_);
}

void Label::operate_on(Machine& machine) {}

class Call : public UnaryInstruction
{
  public:
    using UnaryInstruction::UnaryInstruction;
    void operate_on(Machine& machine) override;
};

void Call::operate_on(Machine& machine)
{
    machine.jump_to(register_);
}

class Ret : public NullaryInstruction
{
  public:
    using NullaryInstruction::NullaryInstruction;
    void operate_on(Machine& machine) override;
};

void Ret::operate_on(Machine& machine)
{
    machine._return();
}

class Cmp : public BinaryInstruction
{
  public:
    using BinaryInstruction::BinaryInstruction;
    void operate_on(Machine& machine) override;
};

void Cmp::operate_on(Machine& machine)
{
    auto const lhs{value_resolver_->get_value_of(register_)};
    auto const rhs{value_resolver_->get_value_of(value_)};

    machine.set_comparison_status_flag(CmpStatusFlags::Invalid);
    if (lhs == rhs)
    {
        machine.set_comparison_status_flag(CmpStatusFlags::Equal);
        machine.set_comparison_status_flag(CmpStatusFlags::LessOrEqual);
        machine.set_comparison_status_flag(CmpStatusFlags::GreaterOrEqual);
    }
    else
    {
        machine.set_comparison_status_flag(CmpStatusFlags::NotEqual);
    }

    if (lhs < rhs)
    {
        machine.set_comparison_status_flag(CmpStatusFlags::Less);
        machine.set_comparison_status_flag(CmpStatusFlags::LessOrEqual);
    }

    if (lhs > rhs)
    {
        machine.set_comparison_status_flag(CmpStatusFlags::Greater);
        machine.set_comparison_status_flag(CmpStatusFlags::GreaterOrEqual);
    }
}

class ConditionalJumpInstruction : public UnaryInstruction
{
  public:
    using UnaryInstruction::UnaryInstruction;
    void operate_on(Machine& machine) override;

  protected:
    virtual CmpStatusFlags get_instruction_flag() const = 0;
};

void ConditionalJumpInstruction::operate_on(Machine& machine)
{
    machine.jump_if_flag_is_set(register_, get_instruction_flag());
}

class Jne : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::NotEqual;
    }
};

class Je : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::Equal;
    }
};

class Jge : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::GreaterOrEqual;
    }
};

class Jg : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::Greater;
    }
};

class Jle : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::LessOrEqual;
    }
};

class Jl : public ConditionalJumpInstruction
{
  public:
    using ConditionalJumpInstruction::ConditionalJumpInstruction;

  protected:
    CmpStatusFlags get_instruction_flag() const override
    {
        return CmpStatusFlags::Less;
    }
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
    instruction_map_.emplace("add", [this](auto const& tokens) { return make_instruction<Add>(tokens); });
    instruction_map_.emplace("sub", [this](auto const& tokens) { return make_instruction<Sub>(tokens); });
    instruction_map_.emplace("mul", [this](auto const& tokens) { return make_instruction<Mul>(tokens); });
    instruction_map_.emplace("div", [this](auto const& tokens) { return make_instruction<Div>(tokens); });
    instruction_map_.emplace("end", [this](auto const& tokens) { return make_instruction<End>(tokens); });
    instruction_map_.emplace("msg", [this](auto const& tokens) { return make_instruction<Msg>(tokens); });
    instruction_map_.emplace("label", [this](auto const& tokens) { return make_instruction<Label>(tokens); });
    instruction_map_.emplace("call", [this](auto const& tokens) { return make_instruction<Call>(tokens); });
    instruction_map_.emplace("ret", [this](auto const& tokens) { return make_instruction<Ret>(tokens); });
    instruction_map_.emplace("cmp", [this](auto const& tokens) { return make_instruction<Cmp>(tokens); });
    instruction_map_.emplace("jne", [this](auto const& tokens) { return make_instruction<Jne>(tokens); });
    instruction_map_.emplace("je", [this](auto const& tokens) { return make_instruction<Je>(tokens); });
    instruction_map_.emplace("jge", [this](auto const& tokens) { return make_instruction<Jge>(tokens); });
    instruction_map_.emplace("jg", [this](auto const& tokens) { return make_instruction<Jg>(tokens); });
    instruction_map_.emplace("jle", [this](auto const& tokens) { return make_instruction<Jle>(tokens); });
    instruction_map_.emplace("jl", [this](auto const& tokens) { return make_instruction<Jl>(tokens); });
}

Instruction_ptr InstructionFactory::create_instruction(std::string const& name,
                                                       std::vector<std::string> const& arguments)
{
    const auto find_iter{name.find(":")};
    const auto is_label{find_iter != std::string::npos};
    if (is_label)
    {
        std::vector<std::string> tmp_arguments{arguments.begin(), arguments.end()};
        tmp_arguments.push_back(name.substr(0, find_iter));
        return instruction_map_.at("label")(tmp_arguments);
    }
    else
    {
        return instruction_map_.at(name)(arguments);
    }
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
void Machine::end_execution()
{
    std_out = &msg_port;
    ip_ = next(program_.end(), -1);
}
void Machine::load_program(RawProgram const& prog)
{
    for (auto instruction : prog)
    {
        auto tokens{split_tokens(instruction)};
        program_.push_back(
            instruction_factory_.create_instruction(tokens.front(), {std::next(tokens.begin(), 1), tokens.end()}));
    }
    pre_run();
}

void Machine::pre_run()
{
    for (ip_ = program_.begin(); ip_ != program_.end(); std::advance(ip_, 1))
    {
        get_current_instruction().pre_run(*this);
    }
}

void Machine::run_program()
{
    for (ip_ = program_.begin(); ip_ != program_.end(); std::advance(ip_, 1))
    {
        get_current_instruction().operate_on(*this);
    }
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

std::string Machine::flush()
{
    return std_out->str();
}

void Machine::add_label_reference(std::string name)
{
    label_map_[name] = ip_;
}

void Machine::jump_to(std::string name)
{
    jump_stack_.push(ip_);
    ip_ = label_map_.at(name);
}

void Machine::_return()
{
    ip_ = jump_stack_.top();
    jump_stack_.pop();
}

void Machine::set_comparison_status_flag(CmpStatusFlags new_status)
{
    if (new_status == CmpStatusFlags::Invalid)
    {
        comparison_status_register_ = CmpStatusFlags::Invalid;
    }
    else
    {
        comparison_status_register_ = static_cast<CmpStatusFlags>(comparison_status_register_ | new_status);
    }
}
void Machine::jump_if_flag_is_set(std::string label, CmpStatusFlags flag)
{
    if (comparison_status_register_ & flag)
    {
        jump_to(label);
    }
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

class TokenSplitter
{
  public:
    TokenSplitter(std::string const& raw_token) : raw_token_{raw_token}
    {
        parse();
    }
    std::vector<std::string> get_tokens()
    {
        return splitup_tokens_;
    }

  private:
    void parse();
    std::vector<std::string> splitup_tokens_{};
    std::string raw_token_{};
};

std::vector<std::string> Machine::split_tokens(std::string const& command)
{
    return TokenSplitter(command).get_tokens();
}

void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

std::vector<std::string> get_lines(std::string const& input)
{
    std::vector<std::string> lines;
    std::istringstream ps(input);
    for (std::string line; std::getline(ps, line);)
    {
        ltrim(line);
        if (line != "")
            lines.push_back(line);
    }
    return lines;
}

void TokenSplitter::parse()
{
    std::regex delimiter(",?\\s+(?=(?:[^']*'[^']*')*[^']*$)");
    raw_token_ = std::regex_replace(raw_token_, delimiter, "\n");
    splitup_tokens_ = get_lines(raw_token_);
}

void delete_braces_front_and_end(std::string& raw_program)
{
    if (raw_program.size() > 2)
        raw_program = raw_program.substr(1, raw_program.size() - 1);
}

std::vector<std::string> sanitize_raw_program(std::string& raw_program)
{
    delete_braces_front_and_end(raw_program);
    std::vector<std::string> program{get_lines(raw_program)};
    std::transform(program.begin(), program.end(), program.begin(), [](auto instruction) {
        std::regex delimiter(";.*(?=(?:[^']*'[^']*')*[^']*$)");
        return std::regex_replace(instruction, delimiter, "");
    });
    program.erase(
        std::remove_if(program.begin(),
                       program.end(),
                       [](auto const& instruction) {
                           const bool is_only_whitespace{instruction.find_first_not_of(' ') == std::string::npos};
                           return is_only_whitespace;
                       }),
        program.end());
    return program;
}

std::string assembler_interpreter(std::string raw_program)
{
    auto program{sanitize_raw_program(raw_program)};
    Machine machine{};
    machine.load_program(program);
    machine.run_program();
    return machine.flush();
}
