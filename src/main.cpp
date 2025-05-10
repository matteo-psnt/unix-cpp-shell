#include <cstdlib>       // getenv, exit, EXIT_FAILURE
#include <unistd.h>      // access, fork, execv, X_OK
#include <sys/wait.h>    // waitpid
#include <iostream>      // cout, cerr, endl, unitbuf
#include <vector>        // vector
#include <string>        // string
#include <sstream>       // istringstream
#include <unordered_map> // unordered_map
#include <functional>    // function
#include <filesystem>    // C++17 filesystem library
#include <readline/readline.h> // readline
#include <readline/history.h>  // add_history
#include <cstring>       // strdup
#include <set>           // set (for unique, sorted completions)
#include <algorithm>     // find_first_not_of, find_last_not_of (for trimming)

// Finds an executable in PATH or by direct path.
// Returns the full path to the executable or an empty string if not found/executable.
std::string find_executable(const std::string &cmd_name) {
    namespace fs = std::filesystem;

    // If cmd_name contains a slash, treat it as a path (absolute or relative)
    if (cmd_name.find('/') != std::string::npos) {
        fs::path cmd_path(cmd_name);
        try {
            if (fs::exists(cmd_path) && fs::is_regular_file(cmd_path) && (access(cmd_path.c_str(), X_OK) == 0)) {
                return fs::canonical(cmd_path).string(); // Return canonical path
            }
        } catch (const fs::filesystem_error& e) {
            // std::cerr << "Filesystem error checking " << cmd_name << ": " << e.what() << std::endl;
            return ""; // Error accessing, treat as not found
        }
        return ""; // Not found or not executable
    }

    // Otherwise, search in PATH environment variable
    const char *path_env_p = std::getenv("PATH");
    if (!path_env_p) {
        // std::cerr << "Warning: PATH environment variable not set." << std::endl; // Optional warning
        return "";
    }

    std::string path_env_str = path_env_p;
    std::istringstream path_stream(path_env_str);
    std::string dir_str;

    while (std::getline(path_stream, dir_str, ':')) {
        if (dir_str.empty()) { // Handle cases like "::" or leading/trailing ":"
            dir_str = "."; // Interpret empty path component as current directory
        }
        try {
            fs::path dir_path(dir_str);
            if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
                continue;
            }
            fs::path full_path = dir_path / cmd_name;

            if (fs::exists(full_path) && fs::is_regular_file(full_path) && (access(full_path.c_str(), X_OK) == 0)) {
                return fs::canonical(full_path).string(); // Return canonical path
            }
        } catch (const fs::filesystem_error& e) {
            // std::cerr << "Filesystem error accessing " << dir_str << "/" << cmd_name << ": " << e.what() << std::endl;
            // Ignore directories or files that cause issues (e.g., permissions, broken symlinks)
            continue;
        }
    }
    return ""; // Not found in PATH
}

// Runs an external command.
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

    if (pid == -1) { // Fork failed
        perror("fork failed");
        return;
    }

    if (pid == 0) { // Child process
        std::vector<char *> argv_c;
        argv_c.reserve(tokens.size() + 1);
        for (const auto &token : tokens) {
            argv_c.push_back(const_cast<char *>(token.c_str()));
        }
        argv_c.push_back(nullptr); // execv expects a null-terminated array

        execv(exec_path_str.c_str(), argv_c.data());
        // If execv returns, an error occurred
        perror(("execv failed for " + tokens[0]).c_str());
        std::exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }
        // Optionally, could inspect 'status' for exit code, signals, etc.
    }
}

// Type for command handlers: takes arguments, returns true if shell should exit.
using CommandHandler = std::function<bool(const std::vector<std::string> &)>;

// Table of built-in commands.
// Needs to be defined after find_executable if used by command handlers.
// Initialized globally or in main before use.
// (Static initialization order can be tricky, but here it's straightforward)
static std::unordered_map<std::string, CommandHandler> command_table = {
    {
        "exit", [](const std::vector<std::string> &/*args*/) {
            return true; // Signal to exit the shell
        }
    },
    {
        "echo", [](const std::vector<std::string> &args) {
            for (size_t i = 1; i < args.size(); ++i) {
                std::cout << args[i] << (i == args.size() - 1 ? "" : " ");
            }
            std::cout << std::endl;
            return false; // Continue shell
        }
    },
    {
        "type", [](const std::vector<std::string> &args) {
            if (args.size() < 2) {
                std::cerr << "type: missing argument" << std::endl;
            } else {
                const std::string &cmd_to_check = args[1];
                // Note: The 'command_table.count' below refers to this very same map.
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
            return false; // Continue shell
        }
    },
    // Add other built-in commands here
};


// Readline command generator for tab completion.
char* command_generator(const char* text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index;

    namespace fs = std::filesystem;

    if (state == 0) { // This is the first call for this completion attempt
        matches.clear();
        match_index = 0;
        std::string prefix = text;
        std::set<std::string> unique_matches; // Use std::set for automatic sorting and uniqueness

        // 1. Add built-in commands
        for (const auto& pair : command_table) {
            const std::string& cmd_name = pair.first;
            // C++20: if (cmd_name.starts_with(prefix))
            if (cmd_name.rfind(prefix, 0) == 0) { // Check if cmd_name starts with prefix
                unique_matches.insert(cmd_name);
            }
        }

        // 2. Add external commands from PATH
        const char* path_env_p = std::getenv("PATH");
        if (path_env_p) {
            std::string path_env_str = path_env_p;
            std::istringstream path_stream(path_env_str);
            std::string dir_str;

            while (std::getline(path_stream, dir_str, ':')) {
                if (dir_str.empty()) {
                     dir_str = "."; // Interpret empty path component as current directory
                }
                try {
                    fs::path dir_path(dir_str);
                    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
                        continue;
                    }

                    // Iterate through files in the directory
                    for (const auto& entry : fs::directory_iterator(dir_path, fs::directory_options::skip_permission_denied)) {
                        try {
                            if (entry.is_regular_file()) {
                                std::string filename = entry.path().filename().string();
                                // C++20: if (filename.starts_with(prefix))
                                if (filename.rfind(prefix, 0) == 0) { // Check if filename starts with prefix
                                    if (access(entry.path().c_str(), X_OK) == 0) {
                                        unique_matches.insert(filename);
                                    }
                                }
                            }
                        } catch (const fs::filesystem_error& /* ignored_inner_error */) {
                            // Ignore errors for individual files (e.g., permission denied to stat)
                        }
                    }
                } catch (const fs::filesystem_error& /* ignored_outer_error */) {
                    // Ignore errors for directories in PATH (e.g., non-existent, not a directory)
                }
            }
        }
        matches.assign(unique_matches.begin(), unique_matches.end());
    }

    // Return the next match (or nullptr if no more matches)
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str()); // Readline expects strdup'd string
    }
    return nullptr;
}

// Readline completion function.
char** shell_completer(const char* text, int /*start*/, int /*end*/) {
    // Don't do filename completion if we are at the start of the line.
    // rl_completion_matches will use our generator.
    rl_attempted_completion_over = 1; // Tell readline we've handled it.
    return rl_completion_matches(text, command_generator);
}

// Executes a parsed command (either built-in or external).
// Returns true if the shell should terminate (e.g., "exit" command).
bool execute_command(const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        return false; // No command, continue shell
    }

    const std::string& command_name = tokens[0];
    auto it = command_table.find(command_name);

    if (it != command_table.end()) {
        // Built-in command
        return it->second(tokens); // Returns true if it's "exit"
    } else {
        // External command
        run_external_command(tokens);
        return false; // External commands don't terminate the shell by default
    }
}

// Helper to trim whitespace from both ends of a string
std::string trim_whitespace(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


int main() {
    // Configure readline to use our custom completer
    rl_attempted_completion_function = shell_completer;

    // Ensure cout and cerr flush immediately for interactive use
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        // readline allocates memory for line_c_str
        char* line_c_str = readline("$ ");

        if (!line_c_str) { // EOF (Ctrl+D)
            std::cout << "exit" << std::endl; // Print "exit" for consistency
            break;
        }

        std::string input_line(line_c_str);
        free(line_c_str); // Free memory allocated by readline

        std::string trimmed_input = trim_whitespace(input_line);

        if (trimmed_input.empty()) {
            continue; // If line was empty or only whitespace, show prompt again
        }

        // Add non-empty, trimmed line to history
        add_history(trimmed_input.c_str());

        // Tokenize the input line (simple whitespace splitting)
        std::istringstream token_stream(trimmed_input);
        std::vector<std::string> tokens;
        std::string token;
        while (token_stream >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) { // Should be redundant due to earlier check, but safe
            continue;
        }

        // Execute command and check if shell should terminate
        if (execute_command(tokens)) {
            break;
        }
    }

    return 0;
}