#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

class AliasManager {
private:
    std::unordered_map<std::string, std::string> aliases;

public:
    // Add or update an alias
    void set_alias(const std::string& name, const std::string& value);
    
    // Remove an alias
    bool remove_alias(const std::string& name);
    
    // Get alias value if it exists
    std::string get_alias(const std::string& name) const;
    
    // Check if an alias exists
    bool has_alias(const std::string& name) const;
    
    // Get all aliases
    const std::unordered_map<std::string, std::string>& get_all_aliases() const;
    
    // Expand aliases in a command token list
    std::vector<std::string> expand_aliases(const std::vector<std::string>& tokens) const;
    std::vector<std::string> expand_aliases_with_recursion_check(const std::vector<std::string>& tokens, std::set<std::string>& expanded_aliases) const;
    
    // Load aliases from config file
    bool load_aliases_from_file(const std::string& filename);
    
    // Save aliases to config file
    bool save_aliases_to_file(const std::string& filename) const;
};

// Global alias manager instance
extern AliasManager alias_manager;
