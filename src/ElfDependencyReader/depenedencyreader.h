#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <elf.h>
#include <gelf.h>
#include <libelf.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>

struct HeaderStruct
{
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
};

class ElfHeaderReader
{
public:
    void addCustomSearchPaths(const std::vector<std::string> &paths);
    void getDependenciesRecursive(const char *filename, std::map<std::string, std::pair<std::string, bool>> &dependencies);
    void exportDependenciesToFile(const char* file, const std::map<std::string, std::pair<std::string, bool>>& dependencies);

private:
    HeaderStruct readElfHeader(const char *filename);
    std::pair<std::string, bool> searchFile(const char *filename, std::shared_ptr<HeaderStruct> elfHeader);
    bool initialize(const char *file);
    void cleanup();
    std::vector<std::string> getLibrarySearchPaths();
    void followSimlink(const char * filename, std::map<std::string, std::pair<std::string, bool>> &dependencies);

    Elf *e = nullptr;
    std::vector<std::string> customSearchPaths;
};
