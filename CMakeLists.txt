cmake_minimum_required(VERSION 3.15)
project(shell-cpp CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
# Remove test files from main source list
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*_test.cpp$")


# Main shell executable
add_executable(shell ${SOURCE_FILES})
# Link readline library

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Test sources (only *_test.cpp in tests/)
file(GLOB TEST_SOURCES tests/*_test.cpp)

# Test executable and linking
add_executable(shell_tests ${TEST_SOURCES} src/shell_utils.cpp src/command_table.cpp src/completion.cpp src/command_parser.cpp src/pipe_utils.cpp)
target_include_directories(shell_tests PRIVATE src)
target_link_libraries(shell_tests PRIVATE gtest_main ${READLINE_LIB} ${NCURSES_LIB})

include(GoogleTest)
gtest_discover_tests(shell_tests)

# Link readline and ncurses libraries for both shell and shell_tests
find_library(READLINE_LIB readline REQUIRED)
find_library(NCURSES_LIB ncurses REQUIRED)
target_link_libraries(shell PRIVATE ${READLINE_LIB} ${NCURSES_LIB})
target_link_libraries(shell_tests PRIVATE ${READLINE_LIB} ${NCURSES_LIB})