#include <gtest/gtest.h>
#include "command_table.h"

TEST(command_table, has_exit_and_echo) {
    EXPECT_TRUE(command_table.count("exit"));
    EXPECT_TRUE(command_table.count("echo"));
}

#include <sstream>
#include <iostream>
#include "shell_utils.h"

TEST(command_table, type_builtin) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "echo"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find("is a shell builtin"), std::string::npos);
}

TEST(command_table, type_external) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "ls"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find("is /"), std::string::npos); // Should print path
}

TEST(command_table, type_nonexistent) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    std::vector<std::string> tokens = {"type", "notacommand12345"};
    execute_command(tokens);
    std::cout.rdbuf(old);
    std::string output = buffer.str();
    EXPECT_NE(output.find(": not found"), std::string::npos);
}
