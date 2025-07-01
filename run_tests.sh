#!/bin/sh
# Run all CTest-based unit tests and integration tests for the unix-shell

# Ensure we are in the project root (the directory containing this script)
cd "$(dirname "$0")" || exit 1

# Build the project (if not already built)
cmake -S . -B build && cmake --build build || exit 1

echo "Running unit tests..."
cd build && ctest --output-on-failure --label-exclude integration

echo ""
echo "Running integration tests..."
echo ""

# Build integration test target and run integration tests
cmake --build . --target shell_integration_tests
ctest --output-on-failure --label-regex integration
