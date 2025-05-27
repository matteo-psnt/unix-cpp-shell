#pragma once
#include <string>
#include <vector>


enum class RedirectType {
    None,
    Stdout,
    Stderr,
    Both,
    StdoutAppend,
    Stdin,
    BothAppend
};


struct ParsedCommand {
    std::vector<std::string> tokens;
    std::string redirect_file;
    RedirectType redirect_type = RedirectType::None;
    std::vector<std::string> piped_tokens;
};

ParsedCommand parse_redirection(std::vector<std::string> tokens);
