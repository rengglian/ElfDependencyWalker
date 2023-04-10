# ELF Dependency Reader

ELF Dependency Reader is a C++ library that helps you find the dependencies of an ELF binary or library, similar to the functionality provided by the `readelf -d` command. The library returns a map of dependencies, including their absolute paths and whether they were found or not.

Output will be a dep.txt file

'tar -cvf packages.tar -T dep.txt'

## Prerequisites

- C++17 compatible compiler
- libelf-dev
- CMake (minimum version 3.12)