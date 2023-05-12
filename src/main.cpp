#include <iostream>
#include <iomanip>

#include "ElfDependencyReader/depenedencyreader.h"

int main(int argc, char **argv)
{
    std::vector<std::string> customPaths;
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <elf-file> <optional-custom-paht1> <optional-custom-paht2> ..." << std::endl;
        return 1;
    }

    for(int i = 2; i < argc; i++)
    {
        customPaths.push_back(argv[i]);    
    }

    const char *filename = argv[1];
    ElfHeaderReader reader;    // Add custom search paths
    reader.addCustomSearchPaths(customPaths);

    std::map<std::string, std::pair<std::string, bool>> dependencies;
    reader.getDependenciesRecursive(filename, dependencies);
    for (const auto& dep : dependencies) {
        std::cout << std::setw(40) << std::left << dep.first << "\t" << (dep.second.second ? "Found" : "Not found") << "\t" << dep.second.first << "\t" << std::endl;
    }

    reader.exportDependenciesToFile("dep.txt", dependencies);

    return 0;
}