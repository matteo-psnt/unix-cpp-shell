#include "pipe_utils.h"
#include "shell_utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

void run_piped_commands(const ParsedCommand& cmd) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]); // close read
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        run_external_command(cmd.tokens);
        std::exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]); // close write
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        run_external_command(cmd.piped_tokens);
        std::exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}
