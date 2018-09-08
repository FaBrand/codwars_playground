#ifndef MAIN_H
#define MAIN_H

std::unordered_map<std::string, int> assembler(std::vector<std::string> const& program);
std::string assembler_interpreter(std::string program);

#endif /* MAIN_H */
