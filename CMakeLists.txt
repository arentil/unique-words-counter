cmake_minimum_required(VERSION 3.30)

project(UniqueWordsCounter VERSION 0.0.1 LANGUAGES CXX)

if (NOT UNIX)
    message(FATAL_ERROR "Linux only")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Wextra -pedantic -Wconversion -Wcast-align -Wunused -Wshadow -Wold-style-cast")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_CXX_FLAGS})

add_executable(uniqueWordsCounter main.cpp unique_word_counter.h)
