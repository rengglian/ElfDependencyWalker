set(ALL_HEADERS ElfDependencyReader/depenedencyreader.h)

set(ALL_SOURCES ElfDependencyReader/depenedencyreader.cpp
    main.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Elf REQUIRED)

add_executable(${EXECUTABLE_NAME} ${ALL_HEADERS} ${ALL_SOURCES} )

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${ELF_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${ELF_LIBRARIES})
