#include "alias_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "shell_utils.h"

// Global alias manager instance
AliasManager alias_manager;

void AliasManager::set_alias(const std::string& name, const std::string& value) {
    // Don't allow empty alias names
    if (name.empty()) {
        return;
    }
    
    // Store the alias
    aliases[name] = value;
}

bool AliasManager::remove_alias(const std::string& name) {
    auto it = aliases.find(name);
    if (it != aliases.end()) {
        aliases.erase(it);
        return true;
    }
    return false;
}

std::string AliasManager::get_alias(const std::string& name) const {
    auto it = aliases.find(name);
    return (it != aliases.end()) ? it->second : "";
}

bool AliasManager::has_alias(const std::string& name) const {
    return aliases.find(name) != aliases.end();
}

const std::unordered_map<std::string, std::string>& AliasManager::get_all_aliases() const {
    return aliases;
}

std::vector<std::string> AliasManager::expand_aliases(const std::vector<std::string>& tokens) const {
    std::set<std::string> expanded_aliases;
    return expand_aliases_with_recursion_check(tokens, expanded_aliases);
}

std::vector<std::string> AliasManager::expand_aliases_with_recursion_check(const std::vector<std::string>& tokens, std::set<std::string>& expanded_aliases) const {
    if (tokens.empty()) {
        return tokens;
    }
    
    // Check if the first token is an alias
    const std::string& first_token = tokens[0];
    if (!has_alias(first_token)) {
        return tokens;
    }
    
    // Check for recursion
    if (expanded_aliases.find(first_token) != expanded_aliases.end()) {
        // Recursion detected - throw error with format consistent with other shell errors
        throw std::runtime_error(first_token + ": alias loop detected");
    }
    
    // Add this alias to the expansion set
    expanded_aliases.insert(first_token);
    
    // Get the alias value
    std::string alias_value = get_alias(first_token);
    
    // Tokenize the alias value
    std::vector<std::string> alias_tokens = tokenize_input(alias_value);
    
    // Recursively expand the first token of the alias if it's also an alias
    if (!alias_tokens.empty()) {
        alias_tokens = expand_aliases_with_recursion_check(alias_tokens, expanded_aliases);
    }
    
    // Create the expanded command: alias tokens + remaining original tokens (skipping first)
    std::vector<std::string> expanded_tokens = alias_tokens;
    
    // Add the remaining original tokens (excluding the alias name)
    for (size_t i = 1; i < tokens.size(); ++i) {
        expanded_tokens.push_back(tokens[i]);
    }
    
    // Remove this alias from the expansion set (backtrack)
    expanded_aliases.erase(first_token);
    
    return expanded_tokens;
}

bool AliasManager::load_aliases_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        line = trim_whitespace(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse alias definition: alias_name=alias_value
        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string name = trim_whitespace(line.substr(0, eq_pos));
            std::string value = trim_whitespace(line.substr(eq_pos + 1));
            
            // Remove quotes if present
            if (!value.empty() && 
                ((value.front() == '"' && value.back() == '"') ||
                 (value.front() == '\'' && value.back() == '\''))) {
                value = value.substr(1, value.length() - 2);
            }
            
            if (!name.empty()) {
                set_alias(name, value);
            }
        }
    }
    
    return true;
}

bool AliasManager::save_aliases_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Shell aliases - auto-generated file\n\n";
    
    for (const auto& pair : aliases) {
        // Quote the value to handle spaces and special characters
        file << pair.first << "='" << pair.second << "'\n";
    }
    
    return true;
}
