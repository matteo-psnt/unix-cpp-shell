#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <filesystem>
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
    std::string expected_tmp = std::filesystem::canonical("/tmp").string();
    EXPECT_EQ(std::string(new_cwd), expected_tmp);
    // Change back to original directory for test isolation
    std::vector<std::string> back = {"cd", old_dir};
    execute_command(back);
    ASSERT_TRUE(getcwd(new_cwd, sizeof(new_cwd)) != nullptr);
    EXPECT_EQ(std::string(new_cwd), old_dir);
}

TEST(CommandTableTest, CdDashGoesToPreviousDirectory) {
    char cwd[4096];
    ASSERT_TRUE(getcwd(cwd, sizeof(cwd)) != nullptr);
    std::string start_dir = cwd;
    // Go to /tmp
    std::vector<std::string> to_tmp = {"cd", "/tmp"};
    execute_command(to_tmp);
    char tmp_cwd[4096];
    ASSERT_TRUE(getcwd(tmp_cwd, sizeof(tmp_cwd)) != nullptr);
    std::string tmp_dir = tmp_cwd;
    std::string expected_tmp = std::filesystem::canonical("/tmp").string();
    EXPECT_EQ(tmp_dir, expected_tmp);
    // Go back to start_dir using cd -
    std::vector<std::string> cd_dash = {"cd", "-"};
    execute_command(cd_dash);
    char back_cwd[4096];
    ASSERT_TRUE(getcwd(back_cwd, sizeof(back_cwd)) != nullptr);
    EXPECT_EQ(std::string(back_cwd), start_dir);
    // Go to /tmp again using cd -
    execute_command(cd_dash);
    ASSERT_TRUE(getcwd(back_cwd, sizeof(back_cwd)) != nullptr);
    EXPECT_EQ(std::string(back_cwd), expected_tmp);
    // Restore original directory
    std::vector<std::string> restore = {"cd", start_dir};
    execute_command(restore);
    ASSERT_TRUE(getcwd(back_cwd, sizeof(back_cwd)) != nullptr);
    EXPECT_EQ(std::string(back_cwd), start_dir);
}
