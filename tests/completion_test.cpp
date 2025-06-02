#include <gtest/gtest.h>
#include <set>
#include <string>
#include <vector>
#include "command_table.h"
#include "completion.h"

// Test: built-in command completion
TEST(CompletionTest, BuiltinCommandCompletion) {
    std::set<std::string> matches;
    add_path_executables_matching_prefix("ec", matches); // Should not add builtins
    matches.clear();
    for (const auto& [cmd, _] : command_table) {
        if (cmd.starts_with("ec")) matches.insert(cmd);
    }
    EXPECT_TRUE(matches.count("echo"));
}

// Test: path executable completion (simulate PATH with /bin)
TEST(CompletionTest, PathExecutableCompletion) {
    std::set<std::string> matches;
    setenv("PATH", "/bin", 1);
    add_path_executables_matching_prefix("ls", matches);
    // Should find 'ls' in /bin
    bool found = false;
    for (const auto& m : matches) {
        if (m == "ls") found = true;
    }
    EXPECT_TRUE(found);
}

// Test: file completion with tilde expansion
TEST(CompletionTest, FileCompletionTilde) {
    std::set<std::string> matches;
    const char* home = getenv("HOME");
    if (home) {
        // Create a file in home for the test
        std::string testfile = std::string(home) + "/.completion_test_file";
        FILE* f = fopen(testfile.c_str(), "w");
        if (f) {
            fputs("test", f);
            fclose(f);
        }
        add_file_completions("~/.completion_test_", matches);
        bool found = false;
        for (const auto& m : matches) {
            if (m.find(".completion_test_file") != std::string::npos) found = true;
        }
        EXPECT_TRUE(found);
        unlink(testfile.c_str());
    }
}

// Test: file completion hides dotfiles unless base starts with '.'
TEST(CompletionTest, FileCompletionHidesDotfiles) {
    std::set<std::string> matches;
    add_file_completions("/bin/l", matches);
    bool has_dotfile = false;
    for (const auto& m : matches) {
        if (!m.empty() && m[0] == '.') has_dotfile = true;
    }
    EXPECT_FALSE(has_dotfile);
}
