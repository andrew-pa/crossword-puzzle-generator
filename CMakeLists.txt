cmake_minimum_required(VERSION 2.8)

project(crossword)

add_subdirectory(depd/cpr)

include_directories(inc/ ${CPR_INCLUDE_DIRS} depd/rapidjson/include)
file(GLOB_RECURSE SOURCES inc/* src/*)

add_executable(crossword ${SOURCES})
target_link_libraries(crossword ${CPR_LIBRARIES})
set_property(TARGET crossword PROPERTY CXX_STANDARD 14)
