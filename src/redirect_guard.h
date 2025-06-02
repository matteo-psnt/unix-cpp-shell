
#pragma once
#include <string>

enum class RedirectType;

class RedirectGuard {
  public:
    RedirectGuard(const std::string& file, RedirectType type);
    ~RedirectGuard();

  private:
    int saved_stdout_ = -1;
    int saved_stderr_ = -1;
    int saved_stdin_ = -1;
    int out_fd_ = -1;
    RedirectType type_;
};
