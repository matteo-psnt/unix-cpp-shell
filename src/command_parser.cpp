#include "command_parser.h"

ParsedCommand parse_redirection(std::vector<std::string> tokens) {
    ParsedCommand result;
    std::vector<std::string> current_cmd;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& op = tokens[i];
        if (op == "|") {
            result.pipeline.push_back(current_cmd);
            current_cmd.clear();
        } else if (
            (op == "&>>") || (op == "&>") ||
            (op == ">>") || (op == "1>>") ||
            (op == "2>>") || (op == ">" ) || (op == "1>") ||
            (op == "2>") || (op == "<")
        ) {
            // Redirection only applies to the last command
            if (op == "&>>") result.redirect_type = RedirectType::BothAppend;
            else if (op == "&>") result.redirect_type = RedirectType::Both;
            else if (op == ">>" || op == "1>>") result.redirect_type = RedirectType::StdoutAppend;
            else if (op == "2>>") result.redirect_type = RedirectType::StderrAppend;
            else if (op == ">" || op == "1>") result.redirect_type = RedirectType::Stdout;
            else if (op == "2>") result.redirect_type = RedirectType::Stderr;
            else if (op == "<") result.redirect_type = RedirectType::Stdin;

            if (i + 1 < tokens.size()) {
                result.redirect_file = tokens[i + 1];
                ++i;
            }
        } else {
            current_cmd.push_back(op);
        }
    }
    if (!current_cmd.empty()) {
        result.pipeline.push_back(current_cmd);
    }
    return result;
}
