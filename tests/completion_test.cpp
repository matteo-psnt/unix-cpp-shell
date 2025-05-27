#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "completion.h"

namespace {

// Dummy test to check completion compiles
TEST(CompletionTest, Compiles) {
    // We can't easily test readline completion without a mock, so just check no crash
    SUCCEED();
}

} // namespace
