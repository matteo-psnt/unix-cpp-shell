#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <filesystem>

std::string find_executable(const std::string &cmd) {
    if (cmd.find('/') != std::string::npos) {
        if (access(cmd.c_str(), X_OK) == 0) return cmd;
        return "";
    }

    const char *path_env = std::getenv("PATH");
    if (!path_env) return "";

    std::string path_var = path_env;
    std::istringstream iss(path_var);
    std::string dir;
    while (std::getline(iss, dir, ':')) {
        std::string full_path = dir + "/" + cmd;
        if (access(full_path.c_str(), X_OK) == 0) {
            return full_path;
        }
    }
    return "";
}

void run_external_command(const std::vector<std::string> &tokens) {
    const std::string exec_path = find_executable(tokens[0]);
    if (exec_path.empty()) {
        std::cout << tokens[0] << ": command not found" << std::endl;
        return;
    }

    const pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::vector<char *> args;
        args.reserve(tokens.size());
        for (const auto &arg : tokens) {
            args.push_back(const_cast<char *>(arg.c_str()));
        }
        args.push_back(nullptr); // Null-terminate

        execv(exec_path.c_str(), args.data());
        perror("execv failed"); // If execv returns, it's an error
        std::exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}


using CommandHandler = std::function<bool(const std::vector<std::string> &)>;

const std::unordered_map<std::string, CommandHandler> command_table = {
    {
        "exit", [](const std::vector<std::string> &args) {
            return true;
        }
    },
    {
        "echo", [](const std::vector<std::string> &args) {
            for (size_t i = 1; i < args.size(); ++i) std::cout << args[i] << " ";
            std::cout << std::endl;
            return false;
        }
    },
    {
        "type", [](const std::vector<std::string> &args) {
            if (args.size() < 2) {
                std::cerr << "type: missing argument" << std::endl;
            } else {
                const std::string &cmd = args[1];
                if (command_table.contains(cmd)) {
                    std::cout << cmd << " is a shell builtin" << std::endl;
                } else if (const std::string cmd_path = find_executable(cmd); !cmd_path.empty()) {
                    std::cout << cmd << " is " << cmd_path << std::endl;
                } else {
                    std::cout << cmd << ": not found" << std::endl;
                }
            }
            return false;
        }
    },
};


bool execute_command(const std::string &input, const std::vector<std::string> &tokens) {
    if (tokens.empty()) return false;

    auto it = command_table.find(tokens[0]);
    if (it != command_table.end()) {
        return it->second(tokens);
    } else {
        run_external_command(tokens);
        return false;
    }
}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);

        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        if (execute_command(input, tokens)) break;
    }
    return 0;
}
