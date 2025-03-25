cmake_minimum_required(VERSION 3.15)

project(work-logger
        VERSION 0.1
        DESCRIPTION "Work logging tool for freelancers"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    cli11
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(cli11)

add_subdirectory(app)
add_subdirectory(src)

