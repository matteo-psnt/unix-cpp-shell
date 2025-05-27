#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

using CommandHandler = std::function<bool(const std::vector<std::string> &)>;
extern std::unordered_map<std::string, CommandHandler> command_table;
