#include "completion.h"
#include "command_table.h"
#include <readline/readline.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <unistd.h>

char* command_generator(const char* text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index;
    namespace fs = std::filesystem;
    if (state == 0) {
        matches.clear();
        match_index = 0;
        std::string prefix = text;
        std::set<std::string> unique_matches;
        for (const auto& pair : command_table) {
            const std::string& cmd_name = pair.first;
            if (cmd_name.starts_with(prefix)) {
                unique_matches.insert(cmd_name);
            }
        }
        // Add executables from PATH
        if (const char* path_env_p = std::getenv("PATH")) {
            std::istringstream path_stream(path_env_p);
            std::string dir_str;
            while (std::getline(path_stream, dir_str, ':')) {
                if (dir_str.empty()) dir_str = ".";
                std::error_code ec;
                fs::path dir_path(dir_str);
                if (!fs::exists(dir_path, ec) || !fs::is_directory(dir_path, ec)) continue;
                for (const auto& entry : fs::directory_iterator(dir_path, fs::directory_options::skip_permission_denied, ec)) {
                    if (entry.is_regular_file(ec)) {
                        std::string filename = entry.path().filename().string();
                        if (filename.starts_with(prefix)) {
                            if (access(entry.path().c_str(), X_OK) == 0) {
                                unique_matches.insert(filename);
                            }
                        }
                    }
                }
            }
        }
        matches.assign(unique_matches.begin(), unique_matches.end());
    }
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    }
    return nullptr;
}

char** shell_completer(const char* text, int /*start*/, int /*end*/) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}
