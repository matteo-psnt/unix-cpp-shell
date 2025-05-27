#!/bin/zsh
# Run all CTest-based unit tests for the shell-cpp project


# Ensure we are in the project root (the directory containing this script)
cd "$(dirname "$0")" || exit 1

# Build the project (if not already built)
cmake -S . -B build && cmake --build build || exit 1

# Run tests with output on failure
cd build && ctest --output-on-failure
