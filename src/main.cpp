#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell_utils.h"
#include "command_table.h"
#include "completion.h"

int main() {
    // Configure readline to use our custom completer
    rl_attempted_completion_function = shell_completer;

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        char* line_c_str = readline("$ ");
        if (!line_c_str) {
            std::cout << "exit" << std::endl;
            break;
        }
        std::string input_line(line_c_str);
        free(line_c_str);

        std::string trimmed_input = trim_whitespace(input_line);
        if (trimmed_input.empty()) {
            continue;
        }
        add_history(trimmed_input.c_str());

        std::vector<std::string> tokens = tokenize_input(trimmed_input);
        if (tokens.empty()) {
            continue;
        }
        if (execute_command(tokens)) {
            break;
        }
    }
    return 0;
}