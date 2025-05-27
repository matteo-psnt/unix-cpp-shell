#pragma once
#include <string>
#include <vector>

enum class RedirectType { None, Stdout, Stderr, Both };

struct ParsedCommand {
    std::vector<std::string> tokens;
    std::string redirect_file;
    RedirectType redirect_type = RedirectType::None;
};

ParsedCommand parse_redirection(std::vector<std::string> tokens);
