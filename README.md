# unix-shell

![shell demo](./docs/shell-demo.svg)

A minimal, educational UNIX-style shell written in modern C++ (C++23).
Supports command execution, I/O redirection (`>`, `>>`, `<`), pipelines (`|`), basic built-ins (`cd`, `pwd`, `echo`, `exit`, etc.), and tab completion.

## Features

* Execution of external commands
* Built-in commands: `cd`, `echo`, `exit`, `pwd`, `type`, `which`, `history`
* I/O redirection: `>`, `>>`, `<`, `2>`, `2>>`, `&>`, `&>>`
* Pipelining with `|`
* Quoting and escaping support
* Auto-completion
* Unit tests with GoogleTest

## Getting Started

### Prerequisites

* CMake 3.15 or higher
* C++23-compatible compiler
* vcpkg (for dependency management)

### Build and Test

```bash
# Clone repository
git clone https://github.com/matteo-psnt/unix-shell.git
cd unix-shell

# Install dependencies via vcpkg (readline, ncurses)
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install

# Build and run tests
./run_tests.sh
```

### Run the Shell

```bash
./run_shell.sh
```

## Project Structure

```plain
src/                 - Shell source code
tests/               - GoogleTest-based unit tests
run_tests.sh         - Build and run tests
run_shell.sh         - Build and run the shell interactively
CMakeLists.txt       - CMake configuration
vcpkg.json           - Dependencies (readline, ncurses)
```
