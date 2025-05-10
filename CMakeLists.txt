cmake_minimum_required(VERSION 3.15)

project(shell-starter-cpp CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(shell ${SOURCE_FILES})


# Link readline library

# find_package(readline CONFIG REQUIRED)
# target_link_libraries(shell PRIVATE readline::readline)

find_library(READLINE_LIB readline REQUIRED)
target_link_libraries(shell PRIVATE ${READLINE_LIB})