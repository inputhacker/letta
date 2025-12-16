cmake_minimum_required(VERSION 3.15)
project(letta-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# JSON library
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# HTTP client
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)
set(CPR_USE_SYSTEM_CURL OFF)
FetchContent_MakeAvailable(cpr)

include_directories(include)

add_executable(letta-cpp
    src/main.cpp
    src/Agent.cpp
    src/Memory.cpp
    src/LLMClient.cpp
)

target_link_libraries(letta-cpp
    PRIVATE
    nlohmann_json::nlohmann_json
    cpr::cpr
)

# Testing
enable_testing()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(letta-test
    tests/main_test.cpp
    tests/MemoryTest.cpp
    tests/AgentTest.cpp
    src/Memory.cpp
    src/Agent.cpp
    src/LLMClient.cpp
)

target_link_libraries(letta-test
    PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    cpr::cpr
)

include(GoogleTest)
gtest_discover_tests(letta-test)
