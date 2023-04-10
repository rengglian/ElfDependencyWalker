cmake_minimum_required(VERSION 3.16)

project("ElfDependencyWalkter" VERSION 1.0.0.0 DESCRIPTION "Creates a list with all dependencies" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_NAME ${PROJECT_NAME}_${PROJECT_VERSION})

add_subdirectory(src)

