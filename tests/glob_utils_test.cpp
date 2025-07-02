#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "glob_utils.h"

namespace fs = std::filesystem;

class GlobUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for testing
        test_dir = fs::temp_directory_path() / "glob_test";
        fs::create_directory(test_dir);
        
        // Create test files
        create_test_file("file1.txt");
        create_test_file("file2.txt");
        create_test_file("test.cpp");
        create_test_file("test.h");
        create_test_file("main.cpp");
        create_test_file("readme.md");
        create_test_file("file1.bak");
        create_test_file("file22.txt");
        create_test_file(".hidden");
        create_test_file(".config");
        
        // Create subdirectory with files
        fs::create_directory(test_dir / "subdir");
        create_test_file("subdir/sub1.txt");
        create_test_file("subdir/sub2.cpp");
        
        // Change to test directory
        original_cwd = fs::current_path();
        fs::current_path(test_dir);
    }
    
    void TearDown() override {
        // Restore original directory and clean up
        fs::current_path(original_cwd);
        fs::remove_all(test_dir);
    }
    
    void create_test_file(const std::string& path) {
        std::ofstream file(test_dir / path);
        file << "test content";
        file.close();
    }
    
    fs::path test_dir;
    fs::path original_cwd;
};

// Test pattern detection
TEST_F(GlobUtilsTest, ContainsGlobPattern) {
    EXPECT_TRUE(contains_glob_pattern("*.txt"));
    EXPECT_TRUE(contains_glob_pattern("file?.txt"));
    EXPECT_TRUE(contains_glob_pattern("*"));
    EXPECT_TRUE(contains_glob_pattern("?"));
    EXPECT_TRUE(contains_glob_pattern("file*.?"));
    
    EXPECT_FALSE(contains_glob_pattern("filename.txt"));
    EXPECT_FALSE(contains_glob_pattern("test"));
    EXPECT_FALSE(contains_glob_pattern(""));
    EXPECT_FALSE(contains_glob_pattern("path/to/file"));
}

// Test basic pattern matching
TEST_F(GlobUtilsTest, MatchesPattern) {
    // Test * wildcard
    EXPECT_TRUE(matches_pattern("file1.txt", "*.txt"));
    EXPECT_TRUE(matches_pattern("file2.txt", "*.txt"));
    EXPECT_TRUE(matches_pattern("test.txt", "*.txt"));
    EXPECT_FALSE(matches_pattern("test.cpp", "*.txt"));
    
    // Test ? wildcard
    EXPECT_TRUE(matches_pattern("file1.txt", "file?.txt"));
    EXPECT_TRUE(matches_pattern("file2.txt", "file?.txt"));
    EXPECT_FALSE(matches_pattern("file22.txt", "file?.txt"));
    EXPECT_FALSE(matches_pattern("test.txt", "file?.txt"));
    
    // Test combined wildcards
    EXPECT_TRUE(matches_pattern("test.cpp", "*.?pp"));
    EXPECT_TRUE(matches_pattern("main.cpp", "*.?pp"));
    EXPECT_FALSE(matches_pattern("readme.md", "*.?pp"));
    
    // Test exact match
    EXPECT_TRUE(matches_pattern("exact", "exact"));
    EXPECT_FALSE(matches_pattern("exact", "other"));
}

// Test single pattern expansion
TEST_F(GlobUtilsTest, ExpandSinglePatternMultipleMatches) {
    auto matches = expand_single_pattern("*.txt");
    
    // Should find multiple .txt files, sorted
    EXPECT_EQ(matches.size(), 3);
    EXPECT_EQ(matches[0], "file1.txt");
    EXPECT_EQ(matches[1], "file2.txt");
    EXPECT_EQ(matches[2], "file22.txt");
}

TEST_F(GlobUtilsTest, ExpandSinglePatternQuestionMark) {
    auto matches = expand_single_pattern("file?.txt");
    
    // Should find file1.txt and file2.txt, but not file22.txt
    EXPECT_EQ(matches.size(), 2);
    EXPECT_EQ(matches[0], "file1.txt");
    EXPECT_EQ(matches[1], "file2.txt");
}

TEST_F(GlobUtilsTest, ExpandSinglePatternNoMatches) {
    auto matches = expand_single_pattern("*.xyz");
    
    // Should find no matches
    EXPECT_TRUE(matches.empty());
}

TEST_F(GlobUtilsTest, ExpandSinglePatternCppFiles) {
    auto matches = expand_single_pattern("*.cpp");
    
    // Should find .cpp files
    EXPECT_EQ(matches.size(), 2);
    EXPECT_EQ(matches[0], "main.cpp");
    EXPECT_EQ(matches[1], "test.cpp");
}

// Test hidden file behavior
TEST_F(GlobUtilsTest, HiddenFilesBehavior) {
    // Pattern not starting with . should not match hidden files
    auto matches = expand_single_pattern("*");
    bool found_hidden = false;
    for (const auto& match : matches) {
        if (match[0] == '.') {
            found_hidden = true;
            break;
        }
    }
    EXPECT_FALSE(found_hidden);
    
    // Pattern starting with . should match hidden files
    auto hidden_matches = expand_single_pattern(".*");
    EXPECT_GE(hidden_matches.size(), 2); // Should find .hidden and .config
    
    // Verify specific hidden files are found
    bool found_dot_hidden = false, found_dot_config = false;
    for (const auto& match : hidden_matches) {
        if (match == ".hidden") found_dot_hidden = true;
        if (match == ".config") found_dot_config = true;
    }
    EXPECT_TRUE(found_dot_hidden);
    EXPECT_TRUE(found_dot_config);
}

// Test directory patterns
TEST_F(GlobUtilsTest, DirectoryPatterns) {
    auto matches = expand_single_pattern("subdir/*.txt");
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], "subdir/sub1.txt");
}

// Test full glob expansion
TEST_F(GlobUtilsTest, ExpandGlobPatternsMultipleTokens) {
    std::vector<std::string> tokens = {"ls", "*.txt", "file?.cpp"};
    auto expanded = expand_glob_patterns(tokens);
    
    // Should expand to: ls file1.txt file2.txt file22.txt (no .cpp files match file?.cpp)
    EXPECT_EQ(expanded.size(), 5); // ls + 3 txt files + literal file?.cpp (no matches)
    EXPECT_EQ(expanded[0], "ls");
    EXPECT_EQ(expanded[1], "file1.txt");
    EXPECT_EQ(expanded[2], "file2.txt");
    EXPECT_EQ(expanded[3], "file22.txt");
    EXPECT_EQ(expanded[4], "file?.cpp"); // No matches, keep literal
}

TEST_F(GlobUtilsTest, ExpandGlobPatternsNoGlobs) {
    std::vector<std::string> tokens = {"echo", "hello", "world"};
    auto expanded = expand_glob_patterns(tokens);
    
    // Should remain unchanged
    EXPECT_EQ(expanded, tokens);
}

TEST_F(GlobUtilsTest, ExpandGlobPatternsNoMatches) {
    std::vector<std::string> tokens = {"rm", "*.nonexistent"};
    auto expanded = expand_glob_patterns(tokens);
    
    // Should keep literal pattern when no matches
    EXPECT_EQ(expanded.size(), 2);
    EXPECT_EQ(expanded[0], "rm");
    EXPECT_EQ(expanded[1], "*.nonexistent");
}

// Test edge cases
TEST_F(GlobUtilsTest, EmptyTokenList) {
    std::vector<std::string> tokens;
    auto expanded = expand_glob_patterns(tokens);
    
    EXPECT_TRUE(expanded.empty());
}

TEST_F(GlobUtilsTest, ComplexPatterns) {
    // Test pattern with both * and ?
    auto matches = expand_single_pattern("*?.txt");
    
    // Should match files with at least one character before .txt
    EXPECT_EQ(matches.size(), 3);
    EXPECT_EQ(matches[0], "file1.txt");
    EXPECT_EQ(matches[1], "file2.txt");
    EXPECT_EQ(matches[2], "file22.txt");
}
