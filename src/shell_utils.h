#pragma once
#include <string>
#include <vector>

std::string trim_whitespace(const std::string& str);
std::string find_executable(const std::string &cmd_name);
void run_external_command(const std::vector<std::string> &tokens);
bool execute_command(const std::vector<std::string> &tokens);
std::vector<std::string> tokenize_input(const std::string& input);
