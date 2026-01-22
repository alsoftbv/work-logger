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

FetchContent_Declare(
    json
    QUIET
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_Declare(
    libharu
    QUIET
    GIT_REPOSITORY https://github.com/libharu/libharu.git
    GIT_TAG v2.4.4
)

FetchContent_MakeAvailable(cli11 json libharu)

option(BUILD_TESTING "Build tests" OFF)

add_subdirectory(app)
add_subdirectory(src)

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        QUIET
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

