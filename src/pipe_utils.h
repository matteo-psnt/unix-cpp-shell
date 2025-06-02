
#pragma once
#include "command_parser.h"

extern bool (*execute_command_ptr)(const std::vector<std::string>&);
void run_pipeline(const ParsedCommand& cmd);
