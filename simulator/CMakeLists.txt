cmake_minimum_required (VERSION 3.1)
project (sim65816)

set (CMAKE_CXX_STANDARD 14)

# All warnings on
set (CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
file(GLOB sources_65816 src/*.cpp src/opcodes/*.cpp)

add_executable(sim65816 ${sources_65816})
target_include_directories(sim65816 PRIVATE ${PROJECT_SOURCE_DIR}/include)
