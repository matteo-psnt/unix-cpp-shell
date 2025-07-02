#include "glob_utils.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

std::vector<std::string> expand_glob_patterns(const std::vector<std::string>& tokens) {
    std::vector<std::string> expanded_tokens;
    
    for (const std::string& token : tokens) {
        if (contains_glob_pattern(token)) {
            std::vector<std::string> matches = expand_single_pattern(token);
            if (!matches.empty()) {
                // Add all matches to the result
                expanded_tokens.insert(expanded_tokens.end(), matches.begin(), matches.end());
            } else {
                // No matches found, keep the literal pattern
                expanded_tokens.push_back(token);
            }
        } else {
            // No glob pattern, keep as-is
            expanded_tokens.push_back(token);
        }
    }
    
    return expanded_tokens;
}

bool contains_glob_pattern(const std::string& str) {
    return str.find('*') != std::string::npos || str.find('?') != std::string::npos;
}

std::vector<std::string> expand_single_pattern(const std::string& pattern) {
    std::vector<std::string> matches;
    namespace fs = std::filesystem;
    
    try {
        // Handle different cases based on pattern structure
        std::string dir_path = ".";
        std::string filename_pattern = pattern;
        
        // Check if pattern contains directory separators
        size_t last_slash = pattern.find_last_of('/');
        if (last_slash != std::string::npos) {
            dir_path = pattern.substr(0, last_slash);
            filename_pattern = pattern.substr(last_slash + 1);
            
            // Handle empty directory path (e.g., "/pattern")
            if (dir_path.empty()) {
                dir_path = "/";
            }
        }
        
        // Scan the directory for matches
        if (fs::exists(dir_path) && fs::is_directory(dir_path)) {
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                if (entry.is_regular_file() || entry.is_directory()) {
                    std::string filename = entry.path().filename().string();
                    
                    // Skip hidden files unless pattern explicitly starts with '.'
                    if (filename[0] == '.' && filename_pattern[0] != '.') {
                        continue;
                    }
                    
                    if (matches_pattern(filename, filename_pattern)) {
                        std::string full_path;
                        if (dir_path == ".") {
                            full_path = filename;
                        } else if (dir_path == "/") {
                            full_path = "/" + filename;
                        } else {
                            full_path = dir_path + "/" + filename;
                        }
                        matches.push_back(full_path);
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        // If filesystem access fails, return empty vector
        // This will cause the literal pattern to be preserved
        return {};
    }
    
    // Sort matches for consistent output
    std::sort(matches.begin(), matches.end());
    
    return matches;
}

bool matches_pattern(const std::string& filename, const std::string& pattern) {
    // Use dynamic programming approach for pattern matching
    size_t f_len = filename.length();
    size_t p_len = pattern.length();
    
    // DP table: dp[i][j] = true if filename[0...i-1] matches pattern[0...j-1]
    std::vector<std::vector<bool>> dp(f_len + 1, std::vector<bool>(p_len + 1, false));
    
    // Empty pattern matches empty filename
    dp[0][0] = true;
    
    // Handle patterns starting with '*'
    for (size_t j = 1; j <= p_len; ++j) {
        if (pattern[j-1] == '*') {
            dp[0][j] = dp[0][j-1];
        }
    }
    
    for (size_t i = 1; i <= f_len; ++i) {
        for (size_t j = 1; j <= p_len; ++j) {
            if (pattern[j-1] == '*') {
                // '*' can match empty string or any character(s)
                dp[i][j] = dp[i][j-1] || dp[i-1][j] || dp[i-1][j-1];
            } else if (pattern[j-1] == '?' || pattern[j-1] == filename[i-1]) {
                // '?' matches any single character, or exact character match
                dp[i][j] = dp[i-1][j-1];
            }
            // else: characters don't match and pattern is not wildcard
            // dp[i][j] remains false
        }
    }
    
    return dp[f_len][p_len];
}
