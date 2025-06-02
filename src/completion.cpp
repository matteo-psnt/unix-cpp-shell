#include "completion.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <optional>
#include <readline/readline.h>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include "command_table.h"

namespace fs = std::filesystem;

// Helper: returns true if the user is completing the first word on the line
static bool is_first_word() {
    if (!rl_line_buffer || rl_point == 0) return true;
    for (int i = rl_point - 1; i >= 0; --i) {
        if (!isspace(rl_line_buffer[i])) return false;
        if (rl_line_buffer[i] == ' ') break;
    }
    return true;
}

// Add all executable files from $PATH that start with the given prefix
void add_path_executables_matching_prefix(const std::string& prefix, std::set<std::string>& out) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) return;

    std::istringstream path_stream(path_env);
    std::string dir;
    std::error_code ec;

    while (std::getline(path_stream, dir, ':')) {
        if (dir.empty()) dir = ".";
        fs::path dir_path(dir);
        if (!fs::exists(dir_path, ec) || !fs::is_directory(dir_path, ec)) continue;

        for (const auto& entry : fs::directory_iterator(dir_path, fs::directory_options::skip_permission_denied, ec)) {
            const auto& path = entry.path();
            const std::string filename = path.filename().string();

            if (entry.is_regular_file(ec) && filename.starts_with(prefix) && access(path.c_str(), X_OK) == 0) {
                out.insert(filename);
            }
        }
    }
}

// Add path/file completions based on prefix (supporting ~ expansion and
// directories)
void add_file_completions(const std::string& prefix, std::set<std::string>& out) {
    std::error_code ec;
    fs::path path_prefix(prefix);
    std::string base = path_prefix.filename().string();
    fs::path dir = path_prefix.parent_path();
    if (dir.empty()) dir = ".";

    const char* home = std::getenv("HOME");
    bool tilde_expanded = false;
    std::string virtual_prefix(prefix);

    if (!prefix.empty() && prefix[0] == '~' && home) {
        tilde_expanded = true;
        if (prefix.size() == 1 || prefix[1] == '/') {
            dir = fs::path(std::string(home) + std::string(prefix.substr(1)));
            base = dir.filename().string();
            dir = dir.parent_path();
        }
    }

    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

    for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
        std::string filename = entry.path().filename().string();
        if (!filename.starts_with(base)) continue;
        if (filename[0] == '.' && (base.empty() || base[0] != '.')) continue;

        std::string result;
        if (tilde_expanded && home) {
            result = "~/" + fs::relative(entry.path(), home).string();
        } else {
            result = (dir == "." ? filename : (dir / filename).string());
        }

        if (entry.is_directory(ec)) result += "/";
        out.insert(result);
    }
}

char* command_generator(const char* text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;

        std::set<std::string> unique_matches;
        std::string prefix(text);

        if (is_first_word()) {
            for (const auto& [cmd, _] : command_table) {
                if (cmd.rfind(prefix, 0) == 0) { // starts_with alternative
                    unique_matches.insert(cmd);
                }
            }
            add_path_executables_matching_prefix(prefix, unique_matches);
        } else {
            add_file_completions(prefix, unique_matches);
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
