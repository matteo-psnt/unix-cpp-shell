#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "pipe_utils.h"
#include "shell_utils.h"
#include "command_parser.h"

// Mock execute_command for testing
#include <functional>
static std::vector<std::vector<std::string>> executed_commands;
bool mock_execute_command(const std::vector<std::string>& tokens) {
    executed_commands.push_back(tokens);
    return false;
}


// Linkage for the real function pointer
extern bool (*execute_command_ptr)(const std::vector<std::string>&);
namespace {
struct ExecuteCommandMocker {
    ExecuteCommandMocker() {
        executed_commands.clear();
        execute_command_ptr = mock_execute_command;
    }
    ~ExecuteCommandMocker() {
        execute_command_ptr = execute_command;
    }
};
}

TEST(PipeUtilsTest, SingleCommandNoRedirection) {
    ExecuteCommandMocker _;
    ParsedCommand cmd;
    cmd.pipeline = { {"echo", "foo"} };
    run_pipeline(cmd);
    ASSERT_EQ(executed_commands.size(), 1);
    EXPECT_EQ(executed_commands[0], std::vector<std::string>({"echo", "foo"}));
}

TEST(PipeUtilsTest, SingleCommandWithRedirection) {
    ExecuteCommandMocker _;
    ParsedCommand cmd;
    cmd.pipeline = { {"echo", "bar"} };
    cmd.redirect_file = "test_output.txt";
    cmd.redirect_type = RedirectType::Stdout;
    run_pipeline(cmd);
    ASSERT_EQ(executed_commands.size(), 1);
    EXPECT_EQ(executed_commands[0], std::vector<std::string>({"echo", "bar"}));
    // Optionally check file existence
    std::ifstream f("test_output.txt");
    EXPECT_TRUE(f.good());
    f.close();
    unlink("test_output.txt");
}

// Helper for testing pipelines without fork
void run_pipeline_no_fork(const ParsedCommand& cmd) {
    for (const auto& tokens : cmd.pipeline) {
        execute_command_ptr(tokens);
    }
}

TEST(PipeUtilsTest, PipelineMultipleCommands) {
    ExecuteCommandMocker _;
    ParsedCommand cmd;
    cmd.pipeline = { {"echo", "foo"}, {"grep", "f"}, {"wc", "-l"} };
    run_pipeline_no_fork(cmd);
    ASSERT_EQ(executed_commands.size(), 3);
    EXPECT_EQ(executed_commands[0], std::vector<std::string>({"echo", "foo"}));
    EXPECT_EQ(executed_commands[1], std::vector<std::string>({"grep", "f"}));
    EXPECT_EQ(executed_commands[2], std::vector<std::string>({"wc", "-l"}));
}

TEST(PipeUtilsTest, EmptyPipelineDoesNothing) {
    ExecuteCommandMocker _;
    ParsedCommand cmd;
    run_pipeline(cmd);
    EXPECT_TRUE(executed_commands.empty());
}
