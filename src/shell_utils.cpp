#include "shell_utils.h"
#include "command_table.h"
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>

std::string trim_whitespace(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return "";
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

std::vector<std::string> tokenize_input(const std::string& input) {
    std::vector<std::string> tokens;
    size_t i = 0;
    while (i < input.size()) {
        // Skip whitespace between tokens
        while (i < input.size() && std::isspace(input[i])) ++i;
        if (i >= input.size()) break;

        std::string token;
        while (i < input.size() && !std::isspace(input[i])) {
            if (input[i] == '\'') {
                // Start of single-quoted segment
                size_t end = input.find('\'', i + 1);
                if (end == std::string::npos) {
                    // No closing quote, take rest of line as literal
                    token += input.substr(i + 1);
                    i = input.size();
                    break;
                } else {
                    token += input.substr(i + 1, end - i - 1);
                    i = end + 1;
                }
            } else {
                // Unquoted segment
                size_t start = i;
                while (i < input.size() && !std::isspace(input[i]) && input[i] != '\'') ++i;
                token += input.substr(start, i - start);
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string find_executable(const std::string &cmd_name) {
    namespace fs = std::filesystem;
    if (cmd_name.find('/') != std::string::npos) {
        fs::path cmd_path(cmd_name);
        try {
            if (fs::exists(cmd_path) && fs::is_regular_file(cmd_path) && (access(cmd_path.c_str(), X_OK) == 0)) {
                return fs::canonical(cmd_path).string();
            }
        } catch (const fs::filesystem_error&) {
            return "";
        }
        return "";
    }
    const char *path_env_p = std::getenv("PATH");
    if (!path_env_p) {
        return "";
    }
    std::string path_env_str = path_env_p;
    std::istringstream path_stream(path_env_str);
    std::string dir_str;
    while (std::getline(path_stream, dir_str, ':')) {
        if (dir_str.empty()) dir_str = ".";
        try {
            fs::path dir_path(dir_str);
            if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) continue;
            fs::path full_path = dir_path / cmd_name;
            if (fs::exists(full_path) && fs::is_regular_file(full_path) && (access(full_path.c_str(), X_OK) == 0)) {
                return fs::canonical(full_path).string();
            }
        } catch (const fs::filesystem_error&) {
            continue;
        }
    }
    return "";
}

void run_external_command(const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        std::cerr << "Error: No command provided for external execution." << std::endl;
        return;
    }
    const std::string exec_path_str = find_executable(tokens[0]);
    if (exec_path_str.empty()) {
        std::cerr << tokens[0] << ": command not found" << std::endl;
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        std::vector<char *> argv_c;
        argv_c.reserve(tokens.size() + 1);
        for (const auto &token : tokens) {
            argv_c.push_back(const_cast<char *>(token.c_str()));
        }
        argv_c.push_back(nullptr);
        execv(exec_path_str.c_str(), argv_c.data());
        perror(("execv failed for " + tokens[0]).c_str());
        std::exit(EXIT_FAILURE);
    } else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}

bool execute_command(const std::vector<std::string> &tokens) {
    if (tokens.empty()) return false;
    const std::string& command_name = tokens[0];
    auto it = command_table.find(command_name);
    if (it != command_table.end()) {
        return it->second(tokens);
    } else {
        run_external_command(tokens);
        return false;
    }
}
