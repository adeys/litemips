cmake_minimum_required(VERSION 3.10)
project(lmips)

set(CMAKE_C_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS}  "-g -O3 -march=native -fno-strict-aliasing")

file(GLOB SOURCE_FILES "src/*.c" "src/*/*.c")

include_directories("src" "src/assembler")
add_executable(${PROJECT_NAME} main.c ${SOURCE_FILES})

file(GLOB TEST_SOURCES "tests/*.c" "tests/*/*.c")
add_executable(${PROJECT_NAME}_test ${SOURCE_FILES} ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}_test PUBLIC "src" "tests/lib")
