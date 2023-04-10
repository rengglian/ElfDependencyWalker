# Find the libelf library and header files
# ELF_INCLUDE_DIRS - where to find elf.h
# ELF_LIBRARIES    - List of libraries when using libelf

find_path(ELF_INCLUDE_DIR NAMES elf.h)
find_library(ELF_LIBRARY NAMES elf)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Elf DEFAULT_MSG ELF_LIBRARY ELF_INCLUDE_DIR)

if(ELF_FOUND)
  set(ELF_INCLUDE_DIRS ${ELF_INCLUDE_DIR})
  set(ELF_LIBRARIES ${ELF_LIBRARY})
endif()
