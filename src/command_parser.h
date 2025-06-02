#pragma once
#include <string>
#include <vector>

enum class RedirectType {
  None,
  Stdout,
  Stderr,
  Both,
  StdoutAppend,
  StderrAppend,
  Stdin,
  BothAppend
};

struct ParsedCommand {
    std::vector<std::vector<std::string>> pipeline; // Each command in the pipeline
    std::string redirect_file;
    RedirectType redirect_type = RedirectType::None;
};

ParsedCommand parse_redirection(std::vector<std::string> tokens);
