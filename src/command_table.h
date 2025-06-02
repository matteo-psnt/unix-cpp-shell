#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
extern std::unordered_map<std::string, CommandHandler> command_table;
