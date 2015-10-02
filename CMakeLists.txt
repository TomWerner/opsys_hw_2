cmake_minimum_required(VERSION 3.3)
project(homework2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99 -g")

set(SOURCE_FILES grep_proc.c)
set(SOURCE_FILES2 grep_thread.c)
add_executable(process_grep ${SOURCE_FILES})
add_executable(thread_grep ${SOURCE_FILES2})