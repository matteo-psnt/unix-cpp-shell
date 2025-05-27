#include <vector>
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
    std::string path = find_executable("notacommand12345");
    EXPECT_TRUE(path.empty());
}

TEST(find_executable, finds_with_path) {
    // /bin/ls should exist on macOS
    std::string path = find_executable("/bin/ls");
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path, "/bin/ls");
}

TEST(find_executable, returns_empty_for_nonexistent_path) {
    std::string path = find_executable("/does/not/exist");
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

TEST(tokenize_input, single_and_double_quoted_and_unquoted_segments) {
    using V = std::vector<std::string>;
    // Simple single-quoted
    EXPECT_EQ(tokenize_input("echo 'hello world'"), (V{"echo", "hello world"}));
    // Multiple quoted args
    EXPECT_EQ(tokenize_input("echo 'foo' 'bar'"), (V{"echo", "foo", "bar"}));
    // Quoted and unquoted mix
    EXPECT_EQ(tokenize_input("echo foo'bar'"), (V{"echo", "foobar"}));
    EXPECT_EQ(tokenize_input("echo 'foo'bar"), (V{"echo", "foobar"}));
    // Adjacent quoted segments
    EXPECT_EQ(tokenize_input("echo 'foo''bar'"), (V{"echo", "foobar"}));
    // Quoted, unquoted, quoted
    EXPECT_EQ(tokenize_input("echo 'foo'bar'baz'"), (V{"echo", "foobarbaz"}));
    // Multiple args, some quoted, some not
    EXPECT_EQ(tokenize_input("echo 'a' b 'c'"), (V{"echo", "a", "b", "c"}));
    // Spaces between tokens
    EXPECT_EQ(tokenize_input("  echo   'hello'   world  "), (V{"echo", "hello", "world"}));
    // Empty quoted string
    EXPECT_EQ(tokenize_input("echo '' foo"), (V{"echo", "foo"}));
    // Unterminated quote
    EXPECT_EQ(tokenize_input("echo 'foo bar"), (V{"echo", "foo bar"}));
    // Only quoted
    EXPECT_EQ(tokenize_input("'foo'"), V{"foo"});
    // Only unquoted
    EXPECT_EQ(tokenize_input("foo"), V{"foo"});
    // Empty input
    EXPECT_EQ(tokenize_input(""), V{});

    // --- Double quote tests ---
    // Simple double-quoted
    EXPECT_EQ(tokenize_input("echo \"hello world\""), (V{"echo", "hello world"}));
    // Multiple double-quoted args
    EXPECT_EQ(tokenize_input("echo \"foo\" \"bar\""), (V{"echo", "foo", "bar"}));
    // Double-quoted and unquoted mix
    EXPECT_EQ(tokenize_input("echo foo\"bar\""), (V{"echo", "foobar"}));
    EXPECT_EQ(tokenize_input("echo \"foo\"bar"), (V{"echo", "foobar"}));
    // Adjacent double-quoted segments
    EXPECT_EQ(tokenize_input("echo \"foo\"\"bar\""), (V{"echo", "foobar"}));
    // Double-quoted, unquoted, double-quoted
    EXPECT_EQ(tokenize_input("echo \"foo\"bar\"baz\""), (V{"echo", "foobarbaz"}));
    // Mixed single and double quotes
    EXPECT_EQ(tokenize_input("echo 'foo'\"bar\""), (V{"echo", "foobar"}));
    EXPECT_EQ(tokenize_input("echo \"foo\"'bar'"), (V{"echo", "foobar"}));
    // Double-quoted with single quote inside
    EXPECT_EQ(tokenize_input("echo \"foo'bar'\""), (V{"echo", "foo'bar'"}));
    // Single-quoted with double quote inside
    EXPECT_EQ(tokenize_input("echo 'foo\"bar\"'"), (V{"echo", "foo\"bar\""}));
    // Double-quoted with escaped double quote
    EXPECT_EQ(tokenize_input("echo \"foo\\\"bar\""), (V{"echo", "foo\"bar"}));
    // Double-quoted with escaped backslash
    EXPECT_EQ(tokenize_input("echo \"foo\\\\bar\""), (V{"echo", "foo\\bar"}));
    // Double-quoted with escaped dollar (should keep the $)
    EXPECT_EQ(tokenize_input("echo \"foo\\$bar\""), (V{"echo", "foo$bar"}));
    // Unterminated double quote
    EXPECT_EQ(tokenize_input("echo \"foo bar"), (V{"echo", "foo bar"}));
    // Double-quoted with newline escape (should keep newline)
    EXPECT_EQ(tokenize_input("echo \"foo\\\nbar\""), (V{"echo", "foo\nbar"}));

    // --- Backslash escaping in unquoted context ---
    // Escaped space
    EXPECT_EQ(tokenize_input("echo before\\   after"), (V{"echo", "before ", "after"}));
    // Multiple escaped spaces
    EXPECT_EQ(tokenize_input("echo world\\ \\ \\ \\ \\ \\ script"), (V{"echo", "world      script"}));
    // Escaped backslash
    EXPECT_EQ(tokenize_input("echo foo\\\\bar"), (V{"echo", "foo\\bar"}));
    // Escaped quote in unquoted context
    EXPECT_EQ(tokenize_input("echo foo\\\"bar"), (V{"echo", "foo\"bar"}));
    // Escaped single quote in unquoted context
    EXPECT_EQ(tokenize_input("echo foo\\'bar"), (V{"echo", "foo\'bar"}));
}