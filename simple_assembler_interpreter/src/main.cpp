#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

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
    static int get_value_of(Registers const& registers, std::string const& in)
    {
        if (is_register(in))
        {
            return registers.at(in);
        }
        else
        {
            const auto value_as_int{std::stoi(in)};
            return value_as_int;
        }
    };
};

class InstructionFactory
{
  public:
    InstructionFactory();
    Instruction_ptr create_instruction(std::string const& name, std::vector<std::string> const& arguments);

  private:
    std::unordered_map<std::string, std::function<Instruction_ptr(std::vector<std::string>)>> instruction_map_{};
};

class Machine
{
  public:
    void load_program(RawProgram const& prog);
    void run_program();
    Registers const& get_registers() const;
    Registers& get_registers();
    void advance_ip(std::ptrdiff_t diff);

  private:
    std::vector<std::string> split_tokens(std::string const& command);
    Instruction& get_current_instruction() const;
    Registers registers_{};
    ProgramPtr ip_{program_.begin()};
    Program program_{};
    InstructionFactory instruction_factory_{};
};

class Instruction
{
  public:
    ~Instruction() = default;
    virtual void operate_on(Machine& machine) = 0;

  private:
    std::string name_{};
};

class Mov : public Instruction
{
  public:
    Mov(std::string param_1, std::string param_2) : register_(param_1), value_(param_2) {}

    void operate_on(Machine& machine) override;

  private:
    std::string register_{};
    std::string value_{};
};

class Inc : public Instruction
{
  public:
    Inc(std::string _register) : register_{_register} {}

    void operate_on(Machine& machine) override;

  private:
    std::string register_{};
};

class Dec : public Instruction
{
  public:
    Dec(std::string _register) : register_{_register} {}

    void operate_on(Machine& machine) override;

  private:
    std::string register_{};
};

class Jnz : public Instruction
{
  public:
    Jnz(std::string param_1, std::string param_2) : register_(param_1), value_(param_2) {}

    void operate_on(Machine& machine) override;

  private:
    int calculate_jump_distance(Registers const& registers);
    std::string register_{};
    std::string value_{};
};

void Mov::operate_on(Machine& machine)
{
    machine.get_registers()[register_] = ValueResolver::get_value_of(machine.get_registers(), value_);
}

void Inc::operate_on(Machine& machine)
{
    ++(machine.get_registers().at(register_));
}

void Dec::operate_on(Machine& machine)
{
    --(machine.get_registers().at(register_));
}

int Jnz::calculate_jump_distance(Registers const& registers)
{
    return ValueResolver::get_value_of(registers, value_);
}

void Jnz::operate_on(Machine& machine)
{
    const int jump_condition{ValueResolver::get_value_of(machine.get_registers(), register_)};
    if (jump_condition != 0)
    {
        const std::ptrdiff_t jump_distance{calculate_jump_distance(machine.get_registers())};
        machine.advance_ip(jump_distance);
    }
}

InstructionFactory::InstructionFactory()
{
    instruction_map_.emplace("mov",
                             [](auto const& tokens) { return std::make_unique<Mov>(tokens.at(0), tokens.at(1)); });
    instruction_map_.emplace("jnz",
                             [](auto const& tokens) { return std::make_unique<Jnz>(tokens.at(0), tokens.at(1)); });
    instruction_map_.emplace("inc", [](auto const& tokens) { return std::make_unique<Inc>(tokens.at(0)); });
    instruction_map_.emplace("dec", [](auto const& tokens) { return std::make_unique<Dec>(tokens.at(0)); });
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
    bool is_after_begin{new_position >= program_.begin()};
    bool is_before_end{new_position <= program_.end()};
    if (is_after_begin && is_before_end)
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

Registers const& Machine::get_registers() const
{
    return registers_;
}

Registers& Machine::get_registers()
{
    return registers_;
}

static int& getReg(Registers& regs, std::string name)
{
    return regs.at(name);
}

Registers assembler(RawProgram const& program)
{
    Machine machine{};
    machine.load_program(program);
    machine.run_program();
    return machine.get_registers();
}
