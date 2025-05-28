#include <iostream>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell_utils.h"
#include "completion.h"
#include "command_parser.h"
#include "redirect_guard.h"
#include "pipe_utils.h"

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
            size_t last_slash = full_path.find_last_of("/");
            std::string folder = (last_slash == std::string::npos) ? full_path : full_path.substr(last_slash + 1);
            prompt = folder + " $ ";
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

        std::vector<std::string> tokens = tokenize_input(trimmed_input);
        if (tokens.empty()) {
            continue;
        }

        ParsedCommand cmd = parse_redirection(std::move(tokens));

        if (cmd.pipeline.size() > 1) {
            run_pipeline(cmd);
            continue;
        }

        bool should_exit = false;
        const auto& command = cmd.pipeline.empty() ? std::vector<std::string>{} : cmd.pipeline[0];

        if (cmd.redirect_type != RedirectType::None) {
            RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
            should_exit = execute_command(command);
        } else {
            should_exit = execute_command(command);
        }

        if (should_exit) {
            break;
        }
    }
    return 0;
}