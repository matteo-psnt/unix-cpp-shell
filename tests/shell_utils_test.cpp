#include <sstream>
#include <iostream>
#include <gtest/gtest.h>
#include "shell_utils.h"

TEST(trim_whitespace, removes_leading_and_trailing_spaces) {
    EXPECT_EQ(trim_whitespace("  hello  "), "hello");
    EXPECT_EQ(trim_whitespace("\thello\n"), "hello");
    EXPECT_EQ(trim_whitespace("hello"), "hello");
    EXPECT_EQ(trim_whitespace("   "), "");
}

TEST(find_executable, finds_bin_ls) {
    std::string path = find_executable("ls");
    EXPECT_FALSE(path.empty());
}

TEST(find_executable, returns_empty_for_nonexistent) {
    std::string path = find_executable("definitelynotacommand12345");
    EXPECT_TRUE(path.empty());
}

TEST(find_executable, finds_with_path) {
    // /bin/ls should exist on macOS
    std::string path = find_executable("/bin/ls");
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path, "/bin/ls");
}

TEST(find_executable, returns_empty_for_nonexistent_path) {
    std::string path = find_executable("/definitely/does/not/exist");
    EXPECT_TRUE(path.empty());
}

TEST(execute_command, built_in_echo) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"echo", "hello", "world"};
    bool should_exit = execute_command(tokens);
    std::cout.rdbuf(old);
    EXPECT_FALSE(should_exit);
    std::string output = buffer.str();
    EXPECT_NE(output.find("hello world"), std::string::npos);
}

TEST(execute_command, non_existent_command) {
    std::stringstream err_buffer;
    std::streambuf* old_err = std::cerr.rdbuf(err_buffer.rdbuf());
    std::vector<std::string> tokens = {"definitelynotacommand12345"};
    bool should_exit = execute_command(tokens);
    std::cerr.rdbuf(old_err);
    EXPECT_FALSE(should_exit);
    std::string err_output = err_buffer.str();
    EXPECT_NE(err_output.find("command not found"), std::string::npos);
}
