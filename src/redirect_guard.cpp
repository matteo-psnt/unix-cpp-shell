#include "redirect_guard.h"
#include "command_parser.h"  // For RedirectType enum
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

RedirectGuard::RedirectGuard(const std::string& file, RedirectType type) : type_(type) {
    if (file.empty() || type == RedirectType::None) return;

    out_fd_ = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd_ < 0) {
        perror("open for redirection");
        return;
    }

    if (type == RedirectType::Stdout || type == RedirectType::Both) {
        saved_stdout_ = dup(STDOUT_FILENO);
        dup2(out_fd_, STDOUT_FILENO);
    }
    if (type == RedirectType::Stderr || type == RedirectType::Both) {
        saved_stderr_ = dup(STDERR_FILENO);
        dup2(out_fd_, STDERR_FILENO);
    }
}

RedirectGuard::~RedirectGuard() {
    if (out_fd_ >= 0) {
        fflush(stdout);
        fflush(stderr);
        if (saved_stdout_ != -1) dup2(saved_stdout_, STDOUT_FILENO);
        if (saved_stderr_ != -1) dup2(saved_stderr_, STDERR_FILENO);
        close(out_fd_);
        if (saved_stdout_ != -1) close(saved_stdout_);
        if (saved_stderr_ != -1) close(saved_stderr_);
    }
}
