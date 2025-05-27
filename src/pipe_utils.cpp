#include "pipe_utils.h"
#include "shell_utils.h"
#include "redirect_guard.h"
#include "command_table.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <array>


void run_pipeline(const ParsedCommand& cmd) {
    size_t n = cmd.pipeline.size();
    std::vector<std::array<int, 2>> pipes(n > 1 ? n - 1 : 0);

    for (size_t i = 0; i < n; ++i) {
        if (i < n - 1) pipe(pipes[i].data());
        pid_t pid = fork();
        if (pid == 0) {
            // Set up input from previous pipe if not first command
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            // Set up output to next pipe if not last command
            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            // Close all pipe fds in child
            for (size_t j = 0; j < pipes.size(); ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            // Only the last command gets redirection
            const auto& tokens = cmd.pipeline[i];
            if (i == n - 1 && cmd.redirect_type != RedirectType::None) {
                RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
                auto it = (!tokens.empty()) ? command_table.find(tokens[0]) : command_table.end();
                if (it != command_table.end()) {
                    it->second(tokens);
                } else {
                    run_external_command(tokens);
                }
            } else {
                auto it = (!tokens.empty()) ? command_table.find(tokens[0]) : command_table.end();
                if (it != command_table.end()) {
                    it->second(tokens);
                } else {
                    run_external_command(tokens);
                }
            }
            std::exit(0);
        }
        // Parent closes pipe ends it doesn't need
        if (i > 0) {
            close(pipes[i-1][0]);
            close(pipes[i-1][1]);
        }
    }
    // Wait for all children
    for (size_t i = 0; i < n; ++i) wait(nullptr);
}
