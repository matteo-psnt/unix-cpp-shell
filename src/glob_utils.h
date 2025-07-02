#pragma once
#include <string>
#include <vector>

/**
 * Expand filesystem globbing patterns in a list of tokens.
 * Patterns containing '*' or '?' are expanded to matching filenames.
 * Returns a new vector with patterns replaced by their matches (sorted).
 * If no matches are found, the literal pattern is preserved.
 * 
 * @param tokens The input tokens that may contain glob patterns
 * @return A new vector with glob patterns expanded
 */
std::vector<std::string> expand_glob_patterns(const std::vector<std::string>& tokens);

/**
 * Check if a string contains glob pattern characters (* or ?)
 * 
 * @param str The string to check
 * @return true if the string contains glob patterns, false otherwise
 */
bool contains_glob_pattern(const std::string& str);

/**
 * Expand a single glob pattern to matching filenames.
 * 
 * @param pattern The glob pattern to expand
 * @return A vector of matching filenames (sorted), or empty if no matches
 */
std::vector<std::string> expand_single_pattern(const std::string& pattern);

/**
 * Check if a filename matches a glob pattern.
 * Supports * (matches any sequence) and ? (matches single character).
 * 
 * @param filename The filename to test
 * @param pattern The glob pattern
 * @return true if the filename matches the pattern
 */
bool matches_pattern(const std::string& filename, const std::string& pattern);
