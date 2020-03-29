cmake_minimum_required(VERSION 3.10)
project(lmips)

set(CMAKE_C_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SOURCE_FILES "src/*.c")

include_directories("src")
add_executable(${PROJECT_NAME} main.c ${SOURCE_FILES})

enable_testing()

file(GLOB TEST_SOURCES "src/*.c" "tests/*.c" "tests/lib/*.c")
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}_test PUBLIC "src" "tests/lib")

add_test(${PROJECT_NAME}_test ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}_test)