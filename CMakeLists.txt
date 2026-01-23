cmake_minimum_required(VERSION 3.15)

project(work-logger
        VERSION 0.1.1
        DESCRIPTION "Work logging tool for freelancers"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output binaries to a contained location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build static libraries for a self-contained binary
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
cmake_policy(SET CMP0077 NEW)

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

# Build libharu as static, without PNG (only JPEG logos supported)
set(LIBHPDF_STATIC ON CACHE BOOL "" FORCE)
set(LIBHPDF_SHARED OFF CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON)

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

