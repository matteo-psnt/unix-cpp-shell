#pragma once
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

std::string trim_whitespace(const std::string& str);
std::string find_executable(const std::string &cmd_name);
void run_external_command(const std::vector<std::string> &tokens);
bool execute_command(const std::vector<std::string> &tokens);
std::vector<std::string> tokenize_input(const std::string& input);


template <typename Func>
auto with_stdout_redirected(const std::string& file, Func func) -> decltype(func()) {
    int saved_stdout = -1, out_fd = -1;
    if (!file.empty()) {
        out_fd = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (out_fd < 0) {
            perror("open for redirection");
        } else {
            saved_stdout = dup(STDOUT_FILENO);
            dup2(out_fd, STDOUT_FILENO);
        }
    }
    auto result = func();
    if (out_fd >= 0) {
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(out_fd);
        close(saved_stdout);
    }
    return result;
}