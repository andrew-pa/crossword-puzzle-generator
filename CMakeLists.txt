cmake_minimum_required(VERSION 2.8)

project(crossword)

include_directories(inc/)
file(GLOB_RECURSE SOURCES inc/* src/*)

add_executable(crossword ${SOURCES})
set_property(TARGET crossword PROPERTY CXX_STANDARD 14)
