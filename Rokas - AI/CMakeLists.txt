cmake_minimum_required(VERSION 3.20)

project(hash_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Without a build type CMake passes no optimization flags (about 7 times slower).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
  set(PROJECT_WARNING_FLAGS /W4)
else()
  set(PROJECT_WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

# Hash core only, no I/O.
add_library(hash_core STATIC src/custom_hash.cpp)
target_include_directories(hash_core PUBLIC include)
target_compile_options(hash_core PRIVATE ${PROJECT_WARNING_FLAGS})
# GCC would reorder the XORs in step() onto the multiply chain; keeping the
# written order is about 4 % faster.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(hash_core PRIVATE -fno-tree-reassoc)
endif()

add_executable(hash-generator src/main.cpp)
target_link_libraries(hash-generator PRIVATE hash_core)
target_compile_options(hash-generator PRIVATE ${PROJECT_WARNING_FLAGS})

add_executable(sanity-checks tests/sanity_checks.cpp)
target_link_libraries(sanity-checks PRIVATE hash_core)
target_compile_options(sanity-checks PRIVATE ${PROJECT_WARNING_FLAGS})

enable_testing()
add_test(NAME sanity COMMAND sanity-checks)

# Run by experiments/run_all.sh.
add_executable(experiments experiments/experiments.cpp experiments/impl_dihash.cpp)
target_include_directories(experiments PRIVATE experiments)
target_link_libraries(experiments PRIVATE hash_core)
target_compile_definitions(experiments PRIVATE IMPL_NAME="di")
target_compile_options(experiments PRIVATE ${PROJECT_WARNING_FLAGS})
