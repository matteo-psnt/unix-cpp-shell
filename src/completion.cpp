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

        // Only the first word gets command completion, all others get path completion
        bool is_first_word = true;
        if (rl_point > 0) {
            for (int i = rl_point; i >= 0; --i) {
                if (!isspace(rl_line_buffer[i])) {
                    is_first_word = false;
                    break;
                }
                if (rl_line_buffer[i] == ' ') break;
            }
        }

        if (is_first_word) {
            // Add built-in commands
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
        } else {
            // Always do path/file completion for all non-first words
            fs::path path_prefix(prefix);
            std::string base = path_prefix.filename().string();
            fs::path dir = path_prefix.parent_path();
            // If prefix starts with ~, recommend from $HOME but do not expand in the completion
            std::string dir_str = dir.string();
            bool tilde_prefix = false;
            if (!dir_str.empty() && dir_str[0] == '~') {
                const char* home = std::getenv("HOME");
                if (home) {
                    dir_str = std::string(home) + dir_str.substr(1);
                    dir = fs::path(dir_str);
                    tilde_prefix = true;
                }
            } else if (prefix.size() > 0 && prefix[0] == '~') {
                const char* home = std::getenv("HOME");
                if (home) {
                    dir = fs::path(home);
                    base = prefix.substr(1);
                    tilde_prefix = true;
                }
            }
            if (dir.empty()) dir = ".";
            std::error_code ec;
            if (fs::exists(dir, ec) && fs::is_directory(dir, ec)) {
                for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
                    std::string filename = entry.path().filename().string();
                    if (filename.starts_with(base)) {
                        // Hide dotfiles unless base starts with '.'
                        if (filename[0] == '.' && (base.empty() || base[0] != '.')) {
                            continue;
                        }
                        std::string completion;
                        if (tilde_prefix) {
                            // Recommend as ~/foo instead of /Users/you/foo
                            completion = "~";
                            if (!dir_str.empty() && dir_str != std::string(getenv("HOME"))) {
                                // If subdir under home, append subdir
                                std::string rel = fs::relative(entry.path().parent_path(), getenv("HOME")).string();
                                if (!rel.empty() && rel != ".") completion += "/" + rel;
                            }
                            completion += "/" + filename;
                        } else {
                            completion = (dir == "." ? filename : (dir / filename).string());
                        }
                        if (entry.is_directory(ec)) completion += "/";
                        unique_matches.insert(completion);
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
