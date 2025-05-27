#include <iostream>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell_utils.h"
#include "completion.h"
#include "command_table.h"
#include "command_parser.h"
#include "redirect_guard.h"

int main() {
    // Configure readline to use our custom completer
    rl_attempted_completion_function = shell_completer;

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        char* line_c_str = readline("$ ");
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

        // Pipe support
        if (!cmd.piped_tokens.empty()) {
            int pipefd[2];
            pipe(pipefd);

            pid_t pid1 = fork();
            if (pid1 == 0) {
                close(pipefd[0]); // close read
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                run_external_command(cmd.tokens);
                exit(0);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                close(pipefd[1]); // close write
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                run_external_command(cmd.piped_tokens);
                exit(0);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, nullptr, 0);
            waitpid(pid2, nullptr, 0);
            continue;
        }

        auto run_cmd = [&]() {
            return execute_command(cmd.tokens);
        };

        bool should_exit;
        if (cmd.redirect_type != RedirectType::None) {
            RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
            should_exit = run_cmd();
        } else {
            should_exit = run_cmd();
        }
        if (should_exit) {
            break;
        }
    }
    return 0;
}