#include "command_table.h"
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>
#include "shell_utils.h"

// Built-in commands
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
    },
    {
        "history", [](const std::vector<std::string> &args) {
            HIST_ENTRY** hist_list = history_list();
       int start = 1;
       int end = history_length;
       if (args.size() == 2) {
         try {
           int n = std::stoi(args[1]);
                    if (n < end) start = std::max(1, end - n + 1);
                } catch (...) {}
       }
       if (hist_list) {
         for (int i = start - 1; i < end; ++i) {
           if (hist_list[i])
                        std::cout << i + history_base << "  " << hist_list[i]->line << std::endl;
         }
       }
       return false;
        }
    },
    {
        "pwd", [](const std::vector<std::string> &args) {
       char cwd[4096];
       if (getcwd(cwd, sizeof(cwd)) != nullptr) {
         std::cout << cwd << std::endl;
       } else {
         std::perror("pwd");
       }
       return false;
        }
    },
    {
        "cd", [](const std::vector<std::string> &args) {
            const char* target = nullptr;
       std::string path;
       if (args.size() < 2) {
         // No argument: go to HOME
         target = std::getenv("HOME");
                if (!target) target = "/";
       } else {
         path = args[1];
         // Expand ~ to HOME
         if (!path.empty() && path[0] == '~') {
           const char *home = std::getenv("HOME");
           if (home) {
             path = std::string(home) + path.substr(1);
           }
         }
         target = path.c_str();
       }
       if (chdir(target) != 0) {
                std::cerr << "cd: " << target << ": No such file or directory" << std::endl;
       }
       return false;
        }
    },
    {
        "which", [](const std::vector<std::string>& args) {
       if (args.size() < 2) {
         std::cerr << "which: missing operand\n";
         return false;
       }
       std::string path = find_executable(args[1]);
       if (!path.empty())
         std::cout << path << std::endl;
       else
         std::cerr << args[1] << ": command not found\n";
       return false;
        }
    }
};
