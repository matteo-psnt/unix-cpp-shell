#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "command_parser.h"

TEST(CommandParserTest, StdoutRedirection) {
    std::vector<std::string> tokens = {"echo", "foo", ">", "file.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"echo", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stdout);
    EXPECT_EQ(cmd.redirect_file, "file.txt");
}

TEST(CommandParserTest, StdoutAppendRedirection) {
    std::vector<std::string> tokens = {"echo", "foo", ">>", "file.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"echo", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::StdoutAppend);
    EXPECT_EQ(cmd.redirect_file, "file.txt");
}

TEST(CommandParserTest, StderrRedirection) {
    std::vector<std::string> tokens = {"ls", "2>", "err.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stderr);
    EXPECT_EQ(cmd.redirect_file, "err.txt");
}

TEST(CommandParserTest, StderrAppendRedirection) {
    std::vector<std::string> tokens = {"ls", "2>>", "err.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::StderrAppend);
    EXPECT_EQ(cmd.redirect_file, "err.txt");
}

TEST(CommandParserTest, BothRedirection) {
    std::vector<std::string> tokens = {"ls", "&>", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Both);
    EXPECT_EQ(cmd.redirect_file, "out.txt");
}

TEST(CommandParserTest, BothAppendRedirection) {
    std::vector<std::string> tokens = {"ls", "&>>", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::BothAppend);
    EXPECT_EQ(cmd.redirect_file, "out.txt");
}

TEST(CommandParserTest, StdinRedirection) {
    std::vector<std::string> tokens = {"cat", "<", "input.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"cat"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stdin);
    EXPECT_EQ(cmd.redirect_file, "input.txt");
}

TEST(CommandParserTest, PipeOnly) {
    std::vector<std::string> tokens = {"ls", "|", "grep", "foo"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 2);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.pipeline[1], std::vector<std::string>({"grep", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::None);
    EXPECT_EQ(cmd.redirect_file, "");
}

TEST(CommandParserTest, PipeWithRedirection) {
    std::vector<std::string> tokens = {"ls", "|", "grep", "foo", ">", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 2);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.pipeline[1], std::vector<std::string>({"grep", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stdout);
    EXPECT_EQ(cmd.redirect_file, "out.txt");
}

TEST(CommandParserTest, NoRedirectionOrPipe) {
    std::vector<std::string> tokens = {"echo", "hello"};
    ParsedCommand cmd = parse_redirection(tokens);
    ASSERT_EQ(cmd.pipeline.size(), 1);
    EXPECT_EQ(cmd.pipeline[0], std::vector<std::string>({"echo", "hello"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::None);
    EXPECT_EQ(cmd.redirect_file, "");
}
