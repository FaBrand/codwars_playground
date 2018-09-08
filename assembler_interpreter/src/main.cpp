#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <regex>
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
    void end_execution();
    std::string flush();

  public:
    std::stringstream std_out{};

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
    NaryInstruction(std::vector<std::string> const& tokens) : Instruction(), arguments_{tokens}
    {
        for (auto i : arguments_)
        {
            std::cout << std::quoted(i) << '\n';
        }
    }
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
    std::cout << std::quoted(arg) << '\n';
    return arg;
}

void Msg::operate_on(Machine& machine)
{
    std::transform(arguments_.begin(),
                   arguments_.end(),
                   std::ostream_iterator<std::string>(machine.std_out, ""),
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
void Machine::end_execution()
{
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
    return std_out.str();
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
    raw_program = raw_program.substr(1, raw_program.size() - 1);
}

std::string assembler_interpreter(std::string raw_program)
{
    if (raw_program.size() == 0)
        return "";
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

    Machine machine{};
    machine.load_program(program);
    machine.run_program();
    return machine.flush();
}
