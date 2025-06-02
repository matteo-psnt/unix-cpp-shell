#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include "redirect_guard.h"
#include "command_parser.h"

namespace {

std::string read_file(const std::string& path) {
    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return content;
}

} // namespace

TEST(RedirectGuardTest, StdoutRedirectionWritesToFile) {
    const char* filename = "redirect_test_stdout.txt";
    {
        RedirectGuard guard(filename, RedirectType::Stdout);
        printf("hello stdout\n");
        fflush(stdout);
    }
    std::string content = read_file(filename);
    EXPECT_NE(content.find("hello stdout"), std::string::npos);
    unlink(filename);
}

TEST(RedirectGuardTest, StderrRedirectionWritesToFile) {
    const char* filename = "redirect_test_stderr.txt";
    {
        RedirectGuard guard(filename, RedirectType::Stderr);
        fprintf(stderr, "hello stderr\n");
        fflush(stderr);
    }
    std::string content = read_file(filename);
    EXPECT_NE(content.find("hello stderr"), std::string::npos);
    unlink(filename);
}

TEST(RedirectGuardTest, StdinRedirectionReadsFromFile) {
    const char* filename = "redirect_test_stdin.txt";
    std::ofstream f(filename);
    f << "input line" << std::endl;
    f.close();
    char buf[32] = {0};
    {
        RedirectGuard guard(filename, RedirectType::Stdin);
        fgets(buf, sizeof(buf), stdin);
    }
    EXPECT_STREQ(buf, "input line\n");
    unlink(filename);
}

TEST(RedirectGuardTest, NoRedirectionDoesNothing) {
    RedirectGuard guard("", RedirectType::None);
    // Should not crash or change fds
    SUCCEED();
}

TEST(RedirectGuardTest, InvalidFileDoesNotCrash) {
    RedirectGuard guard("/this/file/does/not/exist", RedirectType::Stdout);
    // Should not crash
    SUCCEED();
}
