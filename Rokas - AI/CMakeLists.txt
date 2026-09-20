cmake_minimum_required(VERSION 3.20)

project(hash_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(PROJECT_WARNING_FLAGS /W4)
else()
  set(PROJECT_WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

# The hashing core: bytes in, 32 bytes out, no I/O of any kind.
add_library(hash_core STATIC src/custom_hash.cpp)
target_include_directories(hash_core PUBLIC include)
target_compile_options(hash_core PRIVATE ${PROJECT_WARNING_FLAGS})

# Command line front end.
add_executable(hash-generator src/main.cpp)
target_link_libraries(hash-generator PRIVATE hash_core)
target_compile_options(hash-generator PRIVATE ${PROJECT_WARNING_FLAGS})

# Small correctness checks (not the later statistical experiments).
add_executable(sanity-checks tests/sanity_checks.cpp)
target_link_libraries(sanity-checks PRIVATE hash_core)
target_compile_options(sanity-checks PRIVATE ${PROJECT_WARNING_FLAGS})

enable_testing()
add_test(NAME sanity COMMAND sanity-checks)
