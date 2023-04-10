#include <iostream>
#include <iomanip>

#include "ElfDependencyReader/depenedencyreader.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <elf-file>" << std::endl;
        return 1;
    }

    const char *filename = argv[1];
    ElfHeaderReader reader;    // Add custom search paths
    
    std::vector<std::string> customPaths = {"/usr/lib/x86_64-linux-gnu"};
    reader.addCustomSearchPaths(customPaths);

    std::map<std::string, std::pair<std::string, bool>> dependencies;
    reader.getDependenciesRecursive(filename, dependencies);
    for (const auto& dep : dependencies) {
        std::cout << std::setw(40) << std::left << dep.first << "\t" << (dep.second.second ? "Found" : "Not found") << "\t" << dep.second.first << "\t" << std::endl;
    }

    reader.exportDependenciesToFile("dep.txt", dependencies);

    return 0;
}