#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <string>
#include <vector>
#include "command_parser.h"
#include "completion.h"
#include "pipe_utils.h"
#include "redirect_guard.h"
#include "shell_utils.h"

int main() {
    // Configure readline to use our custom completer
    rl_attempted_completion_function = shell_completer;

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // Main shell loop: read, parse, and execute commands
    while (true) {
        // Show only the current folder name in the prompt
        char cwd[4096];
        std::string prompt = "$ ";
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::string full_path = cwd;
            const char* home = getenv("HOME");
            if (home && full_path == home) {
                prompt = "~ $ ";
            } else {
                size_t last_slash = full_path.find_last_of("/");
                std::string folder = (last_slash == std::string::npos) ? full_path : full_path.substr(last_slash + 1);
                prompt = folder + " $ ";
            }
        }
        char* line_c_str = readline(prompt.c_str());
        if (!line_c_str) {
            std::cout << "exit" << std::endl;
            break;
        }
        std::string input_line(line_c_str);
        free(line_c_str);

        std::string trimmed_input = trim_whitespace(input_line);
        if (trimmed_input.empty()) {
            continue;
        }

        add_history(trimmed_input.c_str());

        // Parse and execute command sequence (handles ;, &&, ||)
        std::vector<CommandSequence> sequences = parse_command_sequence(trimmed_input);
        bool should_exit = execute_command_sequence(sequences);

        if (should_exit) {
            break;
        }
    }
    return 0;
}