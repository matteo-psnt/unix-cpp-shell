#include "command_table.h"
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>
#include <algorithm>
#include "shell_utils.h"
#include "alias_manager.h"

// Built-in commands
std::unordered_map<std::string, CommandHandler> command_table = {
    {
        "exit", [](const std::vector<std::string> &/*args*/) {
            return true;
        }
    },
    {
        "echo", [](const std::vector<std::string> &args) {
            bool interpret_escapes = false;
            size_t start_index = 1;
            
            // Check for -e flag
            if (args.size() > 1 && args[1] == "-e") {
                interpret_escapes = true;
                start_index = 2;
            }
            
            for (size_t i = start_index; i < args.size(); ++i) {
                std::string output = args[i];
                
                if (interpret_escapes) {
                    // Process escape sequences
                    std::string processed;
                    for (size_t j = 0; j < output.size(); ++j) {
                        if (output[j] == '\\' && j + 1 < output.size()) {
                            char next = output[j + 1];
                            switch (next) {
                                case 'n': processed += '\n'; break;
                                case 't': processed += '\t'; break;
                                case 'r': processed += '\r'; break;
                                case 'b': processed += '\b'; break;
                                case 'a': processed += '\a'; break;
                                case 'f': processed += '\f'; break;
                                case 'v': processed += '\v'; break;
                                case '\\': processed += '\\'; break;
                                default: processed += '\\'; processed += next; break;
                            }
                            ++j; // Skip the next character
                        } else {
                            processed += output[j];
                        }
                    }
                    output = processed;
                }
                
                std::cout << output << (i == args.size() - 1 ? "" : " ");
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
            static std::string prev_dir;
            const char* target = nullptr;
            std::string path;
            char cwd[4096];
            if (getcwd(cwd, sizeof(cwd)) == nullptr) {
                std::perror("getcwd");
                return false;
            }
            if (args.size() < 2) {
                // No argument: go to HOME
                target = std::getenv("HOME");
                if (!target) target = "/";
            } else if (args[1] == "-") {
                if (prev_dir.empty()) {
                    return false;
                }
                target = prev_dir.c_str();
                std::cout << target << std::endl;
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
            } else {
                prev_dir = cwd;
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
    },
    {
        "export", [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                std::cerr << "export: missing argument" << std::endl;
                return false;
            }
            
            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& arg = args[i];
                size_t eq_pos = arg.find('=');
                
                if (eq_pos != std::string::npos) {
                    std::string name = arg.substr(0, eq_pos);
                    std::string value = arg.substr(eq_pos + 1);
                    
                    if (setenv(name.c_str(), value.c_str(), 1) != 0) {
                        perror("export");
                    }
                } else {
                    // Export existing variable (make it available to child processes)
                    const char* value = std::getenv(arg.c_str());
                    if (value) {
                        if (setenv(arg.c_str(), value, 1) != 0) {
                            perror("export");
                        }
                    } else {
                        // Variable doesn't exist, set it to empty
                        if (setenv(arg.c_str(), "", 1) != 0) {
                            perror("export");
                        }
                    }
                }
            }
            return false;
        }
    },
    {
        "true", [](const std::vector<std::string>& /*args*/) {
            return false; // true command never causes shell exit
        }
    },
    {
        "false", [](const std::vector<std::string>& /*args*/) {
            return false; // false command also never causes shell exit, but indicates failure
        }
    },
    {
        "alias", [](const std::vector<std::string>& args) {
            if (args.size() == 1) {
                // List all aliases
                const auto& all_aliases = alias_manager.get_all_aliases();
                if (all_aliases.empty()) {
                    // No output if no aliases are defined
                    return false;
                }
                
                // Sort aliases by name for consistent output
                std::vector<std::pair<std::string, std::string>> sorted_aliases(all_aliases.begin(), all_aliases.end());
                std::sort(sorted_aliases.begin(), sorted_aliases.end());
                
                for (const auto& pair : sorted_aliases) {
                    std::cout << pair.first << "='" << pair.second << "'\n";
                }
                return false;
            }
            
            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& arg = args[i];
                size_t eq_pos = arg.find('=');
                
                if (eq_pos != std::string::npos) {
                    // Setting an alias: alias name=value
                    std::string name = arg.substr(0, eq_pos);
                    std::string value = arg.substr(eq_pos + 1);
                    
                    // Remove quotes if present
                    if (!value.empty() && 
                        ((value.front() == '"' && value.back() == '"') ||
                         (value.front() == '\'' && value.back() == '\''))) {
                        value = value.substr(1, value.length() - 2);
                    }
                    
                    if (!name.empty()) {
                        alias_manager.set_alias(name, value);
                    }
                } else {
                    // Show specific alias
                    if (alias_manager.has_alias(arg)) {
                        std::cout << arg << "='" << alias_manager.get_alias(arg) << "'\n";
                    } else {
                        std::cerr << arg << ": not found\n";
                    }
                }
            }
            
            return false;
        }
    },
    {
        "unalias", [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                std::cerr << "unalias: usage: unalias name [name ...]\n";
                return false;
            }
            
            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& name = args[i];
                if (!alias_manager.remove_alias(name)) {
                    std::cerr << "unalias: " << name << ": not found\n";
                }
            }
            
            return false;
        }
    }
};
