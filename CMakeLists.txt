cmake_minimum_required(VERSION 3.3)
project(homework2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES process_grep.c list/arraylist.c list/arraylist.h)
set(SOURCE_FILES2 thread_grep.c list/arraylist.c list/arraylist.h)
add_executable(process_grep ${SOURCE_FILES})
add_executable(thread_grep ${SOURCE_FILES2})