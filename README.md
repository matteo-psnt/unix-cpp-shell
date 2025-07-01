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
* GoogleTest unit suite + Tcl/Expect end-to-end tests  

## Getting Started

### Prerequisites

* CMake 3.15 or higher
* C++23-compatible compiler
* **Expect** (for integration tests)  
  - **macOS**: pre-installed  
  - **Ubuntu/Debian**:  
    ```bash
    sudo add-apt-repository universe   # first time only
    sudo apt-get update
    sudo apt-get install expect
    ```

### Build and Test

```bash
# Clone repository
git clone https://github.com/matteo-psnt/unix-shell.git
cd unix-shell

# Bootstrap and install vcpkg deps
git clone https://github.com/microsoft/vcpkg.git vcpkg
chmod +x vcpkg/bootstrap-vcpkg.sh run_tests.sh run_shell.sh
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
tests/               - GoogleTest unit tests and End-to-End integration tests
run_tests.sh         - Build & run all tests
run_shell.sh         - Build & start the shell interactively
CMakeLists.txt       - Build configuration
vcpkg.json           - vcpkg manifest (readline, ncurses)
```
