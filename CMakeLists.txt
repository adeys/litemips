cmake_minimum_required(VERSION 3.10)
project(lmips)

set(CMAKE_C_STANDARD 11)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SOURCE_FILES "src/*.c" "src/*.h")


add_executable(${PROJECT_NAME} main.c ${SOURCE_FILES})

enable_testing()

file(GLOB TEST_SOURCES "tests/*.c" "tests/lib/*")
add_executable(${PROJECT_NAME}_test ${SOURCE_FILES} ${TEST_SOURCES})

add_test(${PROJECT_NAME}_test ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}_test)