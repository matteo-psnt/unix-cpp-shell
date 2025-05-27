#include "gtest/gtest.h"
#include "command_parser.h"
#include <vector>
#include <string>

TEST(CommandParserTest, StdoutRedirection) {
    std::vector<std::string> tokens = {"echo", "foo", ">", "file.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"echo", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stdout);
    EXPECT_EQ(cmd.redirect_file, "file.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, StdoutAppendRedirection) {
    std::vector<std::string> tokens = {"echo", "foo", ">>", "file.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"echo", "foo"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::StdoutAppend);
    EXPECT_EQ(cmd.redirect_file, "file.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, StderrRedirection) {
    std::vector<std::string> tokens = {"ls", "2>", "err.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stderr);
    EXPECT_EQ(cmd.redirect_file, "err.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, BothRedirection) {
    std::vector<std::string> tokens = {"ls", "&>", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Both);
    EXPECT_EQ(cmd.redirect_file, "out.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, BothAppendRedirection) {
    std::vector<std::string> tokens = {"ls", "&>>", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::BothAppend);
    EXPECT_EQ(cmd.redirect_file, "out.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, StdinRedirection) {
    std::vector<std::string> tokens = {"cat", "<", "input.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"cat"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::Stdin);
    EXPECT_EQ(cmd.redirect_file, "input.txt");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}

TEST(CommandParserTest, PipeOnly) {
    std::vector<std::string> tokens = {"ls", "|", "grep", "foo"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::None);
    EXPECT_EQ(cmd.redirect_file, "");
    EXPECT_EQ(cmd.piped_tokens, std::vector<std::string>({"grep", "foo"}));
}

TEST(CommandParserTest, PipeWithRedirection) {
    std::vector<std::string> tokens = {"ls", "|", "grep", "foo", ">", "out.txt"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"ls"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::None);
    EXPECT_EQ(cmd.redirect_file, "");
    EXPECT_EQ(cmd.piped_tokens, std::vector<std::string>({"grep", "foo", ">", "out.txt"}));
}

TEST(CommandParserTest, NoRedirectionOrPipe) {
    std::vector<std::string> tokens = {"echo", "hello"};
    ParsedCommand cmd = parse_redirection(tokens);
    EXPECT_EQ(cmd.tokens, std::vector<std::string>({"echo", "hello"}));
    EXPECT_EQ(cmd.redirect_type, RedirectType::None);
    EXPECT_EQ(cmd.redirect_file, "");
    EXPECT_TRUE(cmd.piped_tokens.empty());
}
