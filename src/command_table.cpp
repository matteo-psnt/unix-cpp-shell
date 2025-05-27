#include "command_table.h"
#include "shell_utils.h"
#include <iostream>

std::unordered_map<std::string, CommandHandler> command_table = {
    {
        "exit", [](const std::vector<std::string> &/*args*/) {
            return true;
        }
    },
    {
        "echo", [](const std::vector<std::string> &args) {
            for (size_t i = 1; i < args.size(); ++i) {
                std::cout << args[i] << (i == args.size() - 1 ? "" : " ");
            }
            std::cout << std::endl;
            return false;
        }
    },
    {
        "type", [](const std::vector<std::string> &args) {
            if (args.size() < 2) {
                std::cerr << "type: missing argument" << std::endl;
            } else {
                const std::string &cmd_to_check = args[1];
                if (command_table.count(cmd_to_check)) {
                    std::cout << cmd_to_check << " is a shell builtin" << std::endl;
                } else {
                    const std::string cmd_path_str = find_executable(cmd_to_check);
                    if (!cmd_path_str.empty()) {
                        std::cout << cmd_to_check << " is " << cmd_path_str << std::endl;
                    } else {
                        std::cout << cmd_to_check << ": not found" << std::endl;
                    }
                }
            }
            return false;
        }
    }
    // Built-in commands
};
