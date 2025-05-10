cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(shell ${SOURCE_FILES})

# Link readline library
find_library(READLINE_LIB readline REQUIRED)
target_link_libraries(shell PRIVATE ${READLINE_LIB})
