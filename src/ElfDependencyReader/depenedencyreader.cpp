#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <cstring>

#include "depenedencyreader.h"

bool ElfHeaderReader::initialize(const char *file)
{
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        return false;
    }

    int fd = open(file, O_RDONLY, 0);
    if (fd < 0)
    {
        return false;
    }

    e = elf_begin(fd, ELF_C_READ, nullptr);
    if (e == nullptr)
    {
        close(fd);
        return false;
    }

    if (elf_kind(e) != ELF_K_ELF)
    {
        cleanup();
        return false;
    }

    return true;
}

void ElfHeaderReader::cleanup()
{
    if (e != nullptr)
    {
        elf_end(e);
        e = nullptr;
    }
}

HeaderStruct ElfHeaderReader::readElfHeader(const char *filename)
{
    if (!initialize(filename))
    {
        throw std::runtime_error("Failed to initialize ELF reader");
    }

    GElf_Ehdr ehdr;
    if (gelf_getehdr(e, &ehdr) == nullptr)
    {
        cleanup();
        throw std::runtime_error("Failed to get ELF header");
    }

    HeaderStruct header;
    memcpy(header.e_ident, ehdr.e_ident, EI_NIDENT);
    header.e_type = ehdr.e_type;
    header.e_machine = ehdr.e_machine;
    header.e_version = ehdr.e_version;
    header.e_entry = ehdr.e_entry;
    header.e_phoff = ehdr.e_phoff;
    header.e_shoff = ehdr.e_shoff;
    header.e_flags = ehdr.e_flags;
    header.e_ehsize = ehdr.e_ehsize;
    header.e_phentsize = ehdr.e_phentsize;
    header.e_phnum = ehdr.e_phnum;
    header.e_shentsize = ehdr.e_shentsize;
    header.e_shnum = ehdr.e_shnum;
    header.e_shstrndx = ehdr.e_shstrndx;

    cleanup();
    return header;
}

std::vector<std::string> ElfHeaderReader::getLibrarySearchPaths()
{
    std::vector<std::string> searchPaths;

    // Add the custom search paths
    for (const auto &path : customSearchPaths)
    {
        searchPaths.push_back(path);
    }

    // Add standard search paths
    searchPaths.push_back("/usr/lib");
    searchPaths.push_back("/usr/local/lib");
    searchPaths.push_back("/lib");
    searchPaths.push_back("/lib64");
    searchPaths.push_back("/usr/lib64");

    return searchPaths;
}

std::pair<std::string, bool> ElfHeaderReader::searchFile(const char *filename, std::shared_ptr<HeaderStruct> elfHeader)
{
    std::vector<std::string> searchPaths = getLibrarySearchPaths();
    for (const auto &path : searchPaths)
    {
        std::string fullPath = path + '/' + filename;
        struct stat buffer;

        if (stat(fullPath.c_str(), &buffer) == 0)
        {
            HeaderStruct header = readElfHeader(fullPath.c_str());
            if (/*memcmp(header.e_ident, elfHeader->e_ident, EI_NIDENT) == 0 &&*/
                header.e_type == elfHeader->e_type &&
                header.e_machine == elfHeader->e_machine &&
                header.e_version == elfHeader->e_version)
            {
                return std::pair<std::string, bool>{fullPath, true};
            }
        }
    }

    return std::pair<std::string, bool>{"", false};
}

void ElfHeaderReader::addCustomSearchPaths(const std::vector<std::string> &paths)
{
    customSearchPaths.insert(customSearchPaths.end(), paths.begin(), paths.end());
}

void ElfHeaderReader::findDependenciesRecursively(const char *file, std::map<std::string, std::pair<std::string, bool>> &dependencies)
{
    if (!initialize(file))
    {
        throw std::runtime_error("Failed to initialize ELF reader");
    }

    Elf_Scn *scn = nullptr;
    GElf_Shdr shdr;

    while ((scn = elf_nextscn(e, scn)) != nullptr)
    {
        if (gelf_getshdr(scn, &shdr) != &shdr)
        {
            cleanup();
            throw std::runtime_error("Failed to get section header");
        }

        if (shdr.sh_type == SHT_DYNAMIC)
        {
            Elf_Data *data = nullptr;
            while ((data = elf_getdata(scn, data)) != nullptr)
            {
                size_t dnum = shdr.sh_size / shdr.sh_entsize;

                for (size_t i = 0; i < dnum; ++i)
                {
                    GElf_Dyn dyn;
                    gelf_getdyn(data, i, &dyn);

                    if (dyn.d_tag == DT_NEEDED)
                    {
                        auto name = elf_strptr(e, shdr.sh_link, dyn.d_un.d_val);
                        if (name == nullptr)
                            continue;
                        auto depIter = dependencies.find(name);
                        if (depIter == dependencies.end())
                        {
                            try
                            {
                                HeaderStruct dependency_header = readElfHeader(file);
                                std::string foundFile = searchFile(name, std::make_unique<HeaderStruct>(dependency_header)).first;
                                bool found = !foundFile.empty();
                                dependencies[name] = {foundFile, found};
                                if (found)
                                {
                                    cleanup(); // Clean up the current ELF reader state before the recursive call.
                                    findDependenciesRecursively(foundFile.c_str(), dependencies);
                                }
                            }
                            catch (const std::runtime_error &e)
                            {
                                cleanup(); // Ensure proper cleanup even when an exception occurs.
                                throw e;
                            }
                        }
                    }
                }
            }
        }
    }

    cleanup();
}

void ElfHeaderReader::getDependenciesRecursive(const char *filename, std::map<std::string, std::pair<std::string, bool>> &dependencies)
{

    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        std::cerr << "Error: ELF library out of date!" << std::endl;
    }

    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if (!ifs)
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }

    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    char *buffer = new char[size];
    ifs.read(buffer, size);

    Elf *e = elf_memory(buffer, size);
    if (e == nullptr)
    {
        std::cerr << "Error: Unable to read ELF file: " << elf_errmsg(-1) << std::endl;
        delete[] buffer;
    }

    GElf_Ehdr ehdr;
    if (gelf_getehdr(e, &ehdr) == nullptr)
    {
        std::cerr << "Error: Unable to get ELF header: " << elf_errmsg(-1) << std::endl;
        delete[] buffer;
        elf_end(e);
    }

    size_t shstrndx;
    if (elf_getshdrstrndx(e, &shstrndx) != 0)
    {
        std::cerr << "Error: Unable to get section header string table index: " << elf_errmsg(-1) << std::endl;
        delete[] buffer;
        elf_end(e);
    }

    Elf_Scn *scn = nullptr;
    while ((scn = elf_nextscn(e, scn)) != nullptr)
    {
        GElf_Shdr shdr;
        if (gelf_getshdr(scn, &shdr) == nullptr)
        {
            std::cerr << "Error: Unable to get section header: " << elf_errmsg(-1) << std::endl;
            delete[] buffer;
            elf_end(e);
        }

        if (shdr.sh_type == SHT_DYNAMIC)
        {
            Elf_Data *data = elf_getdata(scn, nullptr);
            if (data == nullptr)
            {
                std::cerr << "Error: Unable to get section data: " << elf_errmsg(-1) << std::endl;
                delete[] buffer;
                elf_end(e);
            }

            int numEntries = shdr.sh_size / shdr.sh_entsize;
            for (int i = 0; i < numEntries; ++i)
            {
                GElf_Dyn dyn;
                gelf_getdyn(data, i, &dyn);
                if (dyn.d_tag == DT_NEEDED)
                {
                    std::string name = elf_strptr(e, shdr.sh_link, dyn.d_un.d_val);
                    if (dependencies.find(name) == dependencies.end()) {
                        
                        auto dependency_header = std::make_shared<HeaderStruct>(readElfHeader(filename));
                        dependencies[name] = searchFile(name.c_str(), dependency_header);
                        if(dependencies[name].second)
                        {
                            getDependenciesRecursive(dependencies[name].first.c_str(), dependencies);
                        }
                    }
                }
            }
            break;
        }
    }
    delete[] buffer;
    elf_end(e);
}


void ElfHeaderReader::exportDependenciesToFile(const char* file, const std::map<std::string, std::pair<std::string, bool>>& dependencies) {
    std::ofstream outputFile(file);

    if (!outputFile.is_open()) {
        throw std::runtime_error("Failed to open output file");
    }

    for (const auto& [depName, depInfo] : dependencies) {
        const auto& [depPath, depFound] = depInfo;
        outputFile << depPath << std::endl;
    }

    outputFile.close();
}