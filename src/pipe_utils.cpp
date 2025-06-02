#include "pipe_utils.h"
#include "shell_utils.h"
#include "redirect_guard.h"
#include "command_table.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <array>


bool (*execute_command_ptr)(const std::vector<std::string>&) = execute_command;

void run_pipeline(const ParsedCommand& cmd) {
    size_t n = cmd.pipeline.size();
    if (n == 0) return;
    if (n == 1) {
        if (cmd.redirect_type != RedirectType::None) {
            RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
            execute_command_ptr(cmd.pipeline[0]);
        } else {
            execute_command_ptr(cmd.pipeline[0]);
        }
        return;
    }
    std::vector<std::array<int, 2>> pipes(n - 1);
    for (size_t i = 0; i < n - 1; ++i) {
        if (pipe(pipes[i].data()) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    std::vector<pid_t> pids;
    for (size_t i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // stdin from previous pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            // stdout to next pipe
            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            // close all pipes in child
            for (auto& p : pipes) {
                close(p[0]);
                close(p[1]);
            }
            // Only the last command gets redirection
            if (i == n - 1 && cmd.redirect_type != RedirectType::None) {
                RedirectGuard guard(cmd.redirect_file, cmd.redirect_type);
                execute_command_ptr(cmd.pipeline[i]);
            } else {
                execute_command_ptr(cmd.pipeline[i]);
            }
            exit(0);
        } else if (pid > 0) {
            pids.push_back(pid);
        } else {
            perror("fork failed");
        }
    }
    // Parent: close all pipe fds
    for (auto& p : pipes) {
        close(p[0]);
        close(p[1]);
    }
    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
}
