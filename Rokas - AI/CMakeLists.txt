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

# Experiments (see experiments/run_all.sh).
add_executable(experiments experiments/experiments.cpp experiments/impl_eduhash.cpp)
target_include_directories(experiments PRIVATE experiments)
target_link_libraries(experiments PRIVATE hash_core)
target_compile_definitions(experiments PRIVATE IMPL_NAME="v2")
target_compile_options(experiments PRIVATE ${PROJECT_WARNING_FLAGS})

set(V1_SOURCE "" CACHE FILEPATH "custom_hash.cpp of version 1, for version comparison")
if(V1_SOURCE)
  add_library(hash_core_v1 STATIC ${V1_SOURCE})
  target_include_directories(hash_core_v1 PUBLIC include)
  add_executable(experiments_v1 experiments/experiments.cpp experiments/impl_eduhash.cpp)
  target_include_directories(experiments_v1 PRIVATE experiments)
  target_link_libraries(experiments_v1 PRIVATE hash_core_v1)
  target_compile_definitions(experiments_v1 PRIVATE IMPL_NAME="v1")
  add_executable(reversal_v1 experiments/reversal_v1.cpp)
  target_link_libraries(reversal_v1 PRIVATE hash_core_v1)
  target_compile_options(reversal_v1 PRIVATE ${PROJECT_WARNING_FLAGS})
endif()

set(RATAS_DIR "" CACHE PATH "partner implementation directory, for the pair comparison")
if(RATAS_DIR)
  add_library(ratas_core STATIC ${RATAS_DIR}/src/ratas.cpp)
  target_include_directories(ratas_core PUBLIC ${RATAS_DIR}/include)
  add_executable(experiments_ratas experiments/experiments.cpp experiments/impl_ratas.cpp)
  target_include_directories(experiments_ratas PRIVATE experiments)
  target_link_libraries(experiments_ratas PRIVATE ratas_core)
endif()
