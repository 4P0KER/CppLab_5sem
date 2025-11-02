#include "PluginLoader.h"
#include "Calculator.h"
#include "ICalcFunction.h"
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
                    DWORD error = GetLastError();
                    std::cerr << "ERROR: Failed to load DLL (Error " << error << ")" << std::endl;
                    continue;
                }

                std::cout << "DLL loaded successfully" << std::endl;
                bool pluginLoaded = false;

                auto createBinaryOp = reinterpret_cast<IBinaryOperator * (*)()>(
                    GetProcAddress(hDll, "createBinaryOperator"));

                if (createBinaryOp) {
                    std::cout << "Found createBinaryOperator export" << std::endl;
                    try {
                        IBinaryOperator* rawOp = createBinaryOp();
                        BinaryOperatorPtr op(rawOp);
                        char symbol = op->getSymbol();
                        calculator.registerBinaryOperator(symbol, op);
                        loadedLibraries.push_back(hDll);
                        std::cout << "SUCCESS: Loaded binary operator '" << symbol << "'" << std::endl;
                        loadedAny = true;
                        pluginLoaded = true;
                    }
                    catch (const std::exception& e) {
                        std::cerr << "ERROR creating binary operator: " << e.what() << std::endl;
                        FreeLibrary(hDll);
                    }
                }
                else {
                    std::cout << "No createBinaryOperator export found" << std::endl;
                }

                if (!pluginLoaded) {
                    auto createFunc = reinterpret_cast<ICalcFunction * (*)()>(
                        GetProcAddress(hDll, "createFunction"));

                    if (createFunc) {
                        std::cout << "Found createFunction export" << std::endl;
                        try {
                            ICalcFunction* rawFunc = createFunc();
                            CalcFunctionPtr func(rawFunc);
                            std::string funcName = func->getName();
                            calculator.registerFunction(funcName, func);
                            loadedLibraries.push_back(hDll);
                            std::cout << "SUCCESS: Loaded function: " << funcName << std::endl;
                            loadedAny = true;
                            pluginLoaded = true;
                        }
                        catch (const std::exception& e) {
                            std::cerr << "ERROR creating function: " << e.what() << std::endl;
                            FreeLibrary(hDll);
                        }
                    }
                    else {
                        std::cout << "No createFunction export found" << std::endl;
                    }
                }

                if (!pluginLoaded) {
                    std::cerr << "ERROR: No valid exports found, unloading DLL" << std::endl;
                    FreeLibrary(hDll);
                }
            }
        }

        std::cout << "=== Plugin loading complete ===" << std::endl;
        return loadedAny;
    }
    catch (const std::exception& e) {
        std::cerr << "EXCEPTION in plugin loader: " << e.what() << std::endl;
        return false;
    }
}

void PluginLoader::unloadAll() {
    for (HMODULE hDll : loadedLibraries) {
        FreeLibrary(hDll);
    }
    loadedLibraries.clear();
    std::cout << "All plugins unloaded" << std::endl;
}