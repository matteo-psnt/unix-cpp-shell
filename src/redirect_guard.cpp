#include "redirect_guard.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include "command_parser.h" // For RedirectType enum

RedirectGuard::RedirectGuard(const std::string& file, RedirectType type) : type_(type) {
    if (file.empty() || type == RedirectType::None) return;

    int flags = O_WRONLY | O_CREAT;
    if (type == RedirectType::StdoutAppend || type == RedirectType::BothAppend || type == RedirectType::StderrAppend) {
        flags |= O_APPEND;
    } else if (type != RedirectType::Stdin) {
        flags |= O_TRUNC;
    }

    if (type == RedirectType::Stdin) {
        out_fd_ = open(file.c_str(), O_RDONLY);
        if (out_fd_ < 0) {
            perror("open for input redirection");
            return;
        }
        saved_stdin_ = dup(STDIN_FILENO);
        dup2(out_fd_, STDIN_FILENO);
    } else {
        out_fd_ = open(file.c_str(), flags, 0666);
        if (out_fd_ < 0) {
            perror("open for redirection");
            return;
        }
        if (type == RedirectType::Stdout || type == RedirectType::StdoutAppend || type == RedirectType::Both ||
            type == RedirectType::BothAppend) {
            saved_stdout_ = dup(STDOUT_FILENO);
            dup2(out_fd_, STDOUT_FILENO);
        }
        if (type == RedirectType::Stderr || type == RedirectType::Both || type == RedirectType::BothAppend ||
            type == RedirectType::StderrAppend) {
            saved_stderr_ = dup(STDERR_FILENO);
            dup2(out_fd_, STDERR_FILENO);
        }
    }
}

RedirectGuard::~RedirectGuard() {
    if (out_fd_ >= 0) {
        fflush(stdout);
        fflush(stderr);
        if (saved_stdout_ != -1) dup2(saved_stdout_, STDOUT_FILENO);
        if (saved_stderr_ != -1) dup2(saved_stderr_, STDERR_FILENO);
        if (saved_stdin_ != -1) dup2(saved_stdin_, STDIN_FILENO);
        close(out_fd_);
        if (saved_stdout_ != -1) close(saved_stdout_);
        if (saved_stderr_ != -1) close(saved_stderr_);
        if (saved_stdin_ != -1) close(saved_stdin_);
    }
}
