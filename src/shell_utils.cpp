#include "shell_utils.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <sys/wait.h>
#include <vector>
#include "command_table.h"
#include "command_parser.h"
#include "redirect_guard.h"
#include "pipe_utils.h"
#include "glob_utils.h"
#include "alias_manager.h"
#include <cstdio>

static std::string run_subcommand(const std::string& cmd) {
    std::string output;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return output;
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) {
        output += buf;
    }
    pclose(pipe);
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
        output.pop_back();
    }
    return output;
}

std::string trim_whitespace(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

std::vector<std::string> tokenize_input(const std::string& input) {
    std::vector<std::string> tokens;
    std::string token;

    enum class State { Normal, Single, Double } state = State::Normal;
    bool token_from_single_quotes = false;
    
    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        switch (state) {
            case State::Normal:
                if (c == '$' && i + 1 < input.size() && input[i+1] == '(') {
                    if (!token.empty()) {
                        if (!token_from_single_quotes && token[0] == '$' && token.size() > 1) {
                            const char* val = std::getenv(token.c_str() + 1);
                            token = val ? val : "";
                        }
                        tokens.push_back(token);
                        token.clear();
                        token_from_single_quotes = false;
                    }
                    i += 2;
                    int depth = 1;
                    std::string subcmd;
                    for (; i < input.size() && depth > 0; ++i) {
                        if (input[i] == '(') depth++;
                        else if (input[i] == ')') depth--;
                        if (depth > 0) subcmd += input[i];
                    }
                    std::string result = run_subcommand(subcmd);
                    std::istringstream iss(result);
                    std::string word;
                    while (iss >> word) tokens.push_back(word);
                    --i;
                } else if (std::isspace(static_cast<unsigned char>(c))) {
                    if (!token.empty()) {
                        if (!token_from_single_quotes && token[0] == '$' && token.size() > 1) {
                            const char* val = std::getenv(token.c_str() + 1);
                            token = val ? val : "";
                        }
                        tokens.push_back(token);
                        token.clear();
                        token_from_single_quotes = false;
                    }
                } else if (c == '\'') {
                    state = State::Single;
                    token_from_single_quotes = true;
                } else if (c == '"') {
                    state = State::Double;
                } else if (c == '\\' && i + 1 < input.size()) {
                    token += input[i + 1];
                    ++i;
                } else {
                    token += c;
                }
                break;

            case State::Single:
                if (c == '\\' && i + 1 < input.size() && input[i + 1] == '\'') {
                    token += '\'';
                    ++i;
                } else if (c == '\'') {
                    state = State::Normal;
                } else {
                    token += c;
                }
                break;

            case State::Double:
                if (c == '$' && i + 1 < input.size() && input[i+1] == '(') {
                    i += 2;
                    int depth = 1;
                    std::string subcmd;
                    for (; i < input.size() && depth > 0; ++i) {
                        if (input[i] == '(') depth++;
                        else if (input[i] == ')') depth--;
                        if (depth > 0) subcmd += input[i];
                    }
                    std::string result = run_subcommand(subcmd);
                    token += result;
                    --i;
                } else if (c == '\\' && i + 1 < input.size()) {
                    char next = input[i + 1];
                    if (next == '\\' || next == '"' || next == '$') {
                        token += next;
                        ++i;
                    } else if (next == 'n') {
                        token += '\n';
                        ++i;
                    } else if (next == '\n') {
                        token += '\n';
                        ++i;
                    } else {
                        token += '\\';
                    }
                } else if (c == '"') {
                    state = State::Normal;
                } else if (c == '$' && i + 1 < input.size()) {
                    // Handle variable expansion in double quotes
                    ++i;
                    std::string var_name;
                    while (i < input.size() && (std::isalnum(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
                        var_name += input[i];
                        ++i;
                    }
                    --i; // Back up one since the loop will increment
                    
                    if (!var_name.empty()) {
                        const char* val = std::getenv(var_name.c_str());
                        token += val ? val : "";
                    } else {
                        token += '$';
                    }
                } else {
                    token += c;
                }
                break;
        }
    }

    if (!token.empty()) {
        if (!token_from_single_quotes && token[0] == '$' && token.size() > 1) {
            const char* val = std::getenv(token.c_str() + 1);
            token = val ? val : "";
        }
        tokens.push_back(token);
    }

    return tokens;
}

std::string find_executable(const std::string& cmd_name) {
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
    const char* path_env_p = std::getenv("PATH");
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

void run_external_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        std::cerr << "Error: No command provided for external execution." << std::endl;
        return;
    }
    std::vector<std::string> expanded_tokens;
    for (const auto& t : tokens) {
        if (t.rfind("$(", 0) == 0 && t.size() > 2 && t.back() == ')') {
            std::string subcmd = t.substr(2, t.size() - 3);
            std::string result = run_subcommand(subcmd);
            std::istringstream iss(result);
            std::string word;
            while (iss >> word) expanded_tokens.push_back(word);
        } else {
            expanded_tokens.push_back(t);
        }
    }
    const std::string exec_path_str = find_executable(expanded_tokens[0]);
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
        std::vector<char*> argv_c;
        argv_c.reserve(expanded_tokens.size() + 1);
        for (const auto& token : expanded_tokens) {
            argv_c.push_back(const_cast<char*>(token.c_str()));
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
        // Store exit status for logical operations (could be enhanced later)
    }
}

bool execute_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return false;
    
    std::vector<std::string> expanded_tokens;
    try {
        expanded_tokens = alias_manager.expand_aliases(tokens);
    } catch (const std::runtime_error& e) {
        // Handle alias recursion
        std::cerr << e.what() << std::endl;
        return false;
    }
    
    const std::string& command_name = expanded_tokens[0];
    auto it = command_table.find(command_name);
    if (it != command_table.end()) {
        bool result = it->second(expanded_tokens);
        // Ensure output is flushed after built-in commands
        std::cout.flush();
        std::cerr.flush();
        return result;
    } else {
        run_external_command(expanded_tokens);
        // Ensure output is flushed after external commands
        std::cout.flush();
        std::cerr.flush();
        return false;
    }
}

std::vector<CommandSequence> parse_command_sequence(const std::string& input) {
    std::vector<CommandSequence> sequences;
    std::string current_command;
    std::string current_operator = "";
    
    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        
        if (c == ';') {
            if (!current_command.empty()) {
                CommandSequence seq;
                seq.tokens = tokenize_input(current_command);
                // Apply glob expansion after tokenization
                seq.tokens = expand_glob_patterns(seq.tokens);
                seq.operator_type = current_operator;
                sequences.push_back(seq);
                current_command.clear();
                current_operator = ";";
            }
        } else if (c == '&' && i + 1 < input.size() && input[i + 1] == '&') {
            if (!current_command.empty()) {
                CommandSequence seq;
                seq.tokens = tokenize_input(current_command);
                // Apply glob expansion after tokenization
                seq.tokens = expand_glob_patterns(seq.tokens);
                seq.operator_type = current_operator;
                sequences.push_back(seq);
                current_command.clear();
                current_operator = "&&";
            }
            ++i; // Skip the second &
        } else if (c == '|' && i + 1 < input.size() && input[i + 1] == '|') {
            if (!current_command.empty()) {
                CommandSequence seq;
                seq.tokens = tokenize_input(current_command);
                // Apply glob expansion after tokenization
                seq.tokens = expand_glob_patterns(seq.tokens);
                seq.operator_type = current_operator;
                sequences.push_back(seq);
                current_command.clear();
                current_operator = "||";
            }
            ++i; // Skip the second |
        } else {
            current_command += c;
        }
    }
    
    // Add the last command
    if (!current_command.empty()) {
        CommandSequence seq;
        seq.tokens = tokenize_input(current_command);
        // Apply glob expansion after tokenization
        seq.tokens = expand_glob_patterns(seq.tokens);
        seq.operator_type = current_operator;
        sequences.push_back(seq);
    }
    
    return sequences;
}

bool execute_command_sequence(const std::vector<CommandSequence>& sequences) {
    bool last_command_success = true;
    bool should_exit = false;
    
    for (const auto& seq : sequences) {
        bool should_execute = true;
        
        if (seq.operator_type == "&&" && !last_command_success) {
            should_execute = false;
        } else if (seq.operator_type == "||" && last_command_success) {
            should_execute = false;
        }
        
        if (should_execute && !seq.tokens.empty()) {
            ParsedCommand cmd = parse_redirection(seq.tokens);
            
            if (cmd.pipeline.size() > 1) {
                run_pipeline(cmd);
                last_command_success = true; // Assume success for now
            } else {
                const auto& command = cmd.pipeline.empty() ? std::vector<std::string>{} : cmd.pipeline[0];
                
                if (cmd.redirect_type != RedirectType::None) {
                    RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
                    should_exit = execute_command(command);
                } else {
                    should_exit = execute_command(command);
                }
                
                // Determine command success based on command type
                if (!command.empty()) {
                    if (command[0] == "false") {
                        last_command_success = false;
                    } else if (command[0] == "true") {
                        last_command_success = true;
                    } else if (command[0] == "exit") {
                        last_command_success = true; // exit is considered successful
                    } else {
                        // For other built-ins and external commands, success is the default
                        // unless they explicitly fail
                        last_command_success = true;
                    }
                } else {
                    last_command_success = true;
                }
                
                if (should_exit) {
                    break;
                }
            }
        }
    }
    
    return should_exit;
}
