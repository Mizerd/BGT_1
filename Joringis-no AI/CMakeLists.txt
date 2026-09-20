cmake_minimum_required(VERSION 3.16)
project(ratas256 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(ratas_core STATIC src/ratas.cpp)
target_include_directories(ratas_core PUBLIC include)

add_executable(ratas src/main.cpp)
target_link_libraries(ratas PRIVATE ratas_core)

add_executable(sanity tests/sanity.cpp)
target_link_libraries(sanity PRIVATE ratas_core)

add_executable(experiments src/experiments.cpp src/std_hashes.cpp)
target_link_libraries(experiments PRIVATE ratas_core)
if(WIN32)
  target_link_libraries(experiments PRIVATE bcrypt)
else()
  find_package(OpenSSL REQUIRED)
  target_link_libraries(experiments PRIVATE OpenSSL::Crypto)
endif()

enable_testing()
add_test(NAME sanity COMMAND sanity)
