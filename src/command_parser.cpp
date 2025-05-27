#include "command_parser.h"

ParsedCommand parse_redirection(std::vector<std::string> tokens) {
    ParsedCommand result;
    result.tokens = std::move(tokens);

    for (size_t i = 0; i + 1 < result.tokens.size(); ++i) {
        const std::string& op = result.tokens[i];
        if (op == "&>>") {
            result.redirect_type = RedirectType::BothAppend;
        } else if (op == "&>") {
            result.redirect_type = RedirectType::Both;
        } else if (op == ">>") {
            result.redirect_type = RedirectType::StdoutAppend;
        } else if (op == ">" || op == "1>") {
            result.redirect_type = RedirectType::Stdout;
        } else if (op == "2>") {
            result.redirect_type = RedirectType::Stderr;
        } else if (op == "<") {
            result.redirect_type = RedirectType::Stdin;
        } else if (op == "|") {
            result.piped_tokens.assign(result.tokens.begin() + i + 1, result.tokens.end());
            result.tokens.resize(i);
            break;
        } else {
            continue;
        }

        result.redirect_file = result.tokens[i + 1];
        result.tokens.erase(result.tokens.begin() + i, result.tokens.begin() + i + 2);
        break;
    }

    return result;
}
