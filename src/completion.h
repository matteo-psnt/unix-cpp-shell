#pragma once
#include <set>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif
char** shell_completer(const char* text, int start, int end);
#ifdef __cplusplus
}
#endif

// For testing
void add_path_executables_matching_prefix(const std::string& prefix, std::set<std::string>& out);
void add_file_completions(const std::string& prefix, std::set<std::string>& out);
