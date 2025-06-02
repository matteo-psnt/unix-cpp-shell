#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "command_table.h"
#include "shell_utils.h"

TEST(CommandTableTest, HasExitAndEcho) {
    EXPECT_TRUE(command_table.count("exit"));
    EXPECT_TRUE(command_table.count("echo"));
    EXPECT_TRUE(command_table.count("type"));
    EXPECT_TRUE(command_table.count("pwd"));
    EXPECT_TRUE(command_table.count("cd"));
}

TEST(CommandTableTest, TypeBuiltin) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "echo"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find("is a shell builtin"), std::string::npos);
}

TEST(CommandTableTest, TypeExternal) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "ls"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find("is /"), std::string::npos); // Should print path
}

TEST(CommandTableTest, TypeNonexistent) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "notacommand12345"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find(": not found"), std::string::npos);
}

TEST(CommandTableTest, PwdPrintsCurrentDirectory) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"pwd"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    char cwd[4096];
    ASSERT_TRUE(getcwd(cwd, sizeof(cwd)) != nullptr);
    // Output should contain the current directory
    EXPECT_NE(output.find(cwd), std::string::npos);
}

TEST(CommandTableTest, CdChangesDirectory) {
    char cwd[4096];
    ASSERT_TRUE(getcwd(cwd, sizeof(cwd)) != nullptr);
    std::string old_dir = cwd;
    // Use /tmp as a directory that should exist
    std::vector<std::string> tokens = {"cd", "/tmp"};
    execute_command(tokens);
    char new_cwd[4096];
    ASSERT_TRUE(getcwd(new_cwd, sizeof(new_cwd)) != nullptr);
    EXPECT_EQ(std::string(new_cwd), "/private/tmp");
    // Change back to original directory for test isolation
    std::vector<std::string> back = {"cd", old_dir};
    execute_command(back);
    ASSERT_TRUE(getcwd(new_cwd, sizeof(new_cwd)) != nullptr);
    EXPECT_EQ(std::string(new_cwd), old_dir);
}
