#include "PluginLoader.h"
#include "Calculator.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

PluginLoader::~PluginLoader() {
    unloadAll();
}

bool PluginLoader::loadPluginsFromDirectory(const std::string& directory, Calculator& calculator) {
    std::cout << "=== Loading plugins from: " << directory << " ===" << std::endl;

    try {
        if (!fs::exists(directory)) {
            std::cout << "Directory does not exist: " << directory << std::endl;
            return false;
        }

        bool loadedAny = false;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".dll") {
                std::cout << "Found DLL: " << entry.path().filename() << std::endl;

                HMODULE hDll = LoadLibraryA(entry.path().string().c_str());
                if (!hDll) {
                    std::cerr << "Failed to load DLL" << std::endl;
                    continue;
                }

                loadedLibraries.push_back(hDll);
                loadedAny = true;
                std::cout << "DLL loaded successfully" << std::endl;
            }
        }

        std::cout << "=== Plugin loading complete ===" << std::endl;
        return loadedAny;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in plugin loader: " << e.what() << std::endl;
        return false;
    }
}

void PluginLoader::unloadAll() {
    for (HMODULE hDll : loadedLibraries) {
        FreeLibrary(hDll);
    }
    loadedLibraries.clear();
}