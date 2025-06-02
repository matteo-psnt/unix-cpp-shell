#include "command_parser.h"

ParsedCommand parse_redirection(std::vector<std::string> tokens) {
    ParsedCommand result;
    std::vector<std::string> current_cmd;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token == "|") {
            // Start a new command in the pipeline
            result.pipeline.push_back(current_cmd);
            current_cmd.clear();
        } else if (
            token == "&>>" || token == "&>"  ||
            token == ">>"  || token == "1>>" ||
            token == "2>>" || token == ">"   || token == "1>" ||
            token == "2>"  || token == "<"
        ) {
            // Redirection applies to the last command in the pipeline
            if (token == "&>>")
                result.redirect_type = RedirectType::BothAppend;
            else if (token == "&>")
                result.redirect_type = RedirectType::Both;
            else if (token == ">>" || token == "1>>")
                result.redirect_type = RedirectType::StdoutAppend;
            else if (token == "2>>")
                result.redirect_type = RedirectType::StderrAppend;
            else if (token == ">" || token == "1>")
                result.redirect_type = RedirectType::Stdout;
            else if (token == "2>")
                result.redirect_type = RedirectType::Stderr;
            else if (token == "<")
                result.redirect_type = RedirectType::Stdin;

            if (i + 1 < tokens.size()) {
                result.redirect_file = tokens[i + 1];
                ++i; // Skip the filename token
            }
        } else {
            current_cmd.push_back(token);
        }
    }

    if (!current_cmd.empty()) {
        result.pipeline.push_back(current_cmd);
    }

    return result;
}
