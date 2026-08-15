#pragma once
// AYPlugin/DynamicLib.h - 动态库抽象

#include <AYCore.h>
#include <string>

#if defined(_WIN32)
    #define DYNAMIC_LIB_EXT ".dll"
    #include <windows.h>
#else
    #define DYNAMIC_LIB_EXT ".so"
    #include <dlfcn.h>
#endif

namespace ayt::plugin
{

class DynamicLib {
public:
    explicit DynamicLib(const std::string& path);
    ~DynamicLib();

    bool load();
    void unload();

    void* getSymbol(const char* symbol);

    bool isLoaded() const { return _handle != nullptr; }
    const std::string& getPath() const { return _path; }

    const char* getLastError() const;

private:
    std::string _path;

#if defined(_WIN32)
    using Handle = HMODULE;
#else
    using Handle = void*;
#endif
    Handle _handle = nullptr;
    std::string _lastError;
};

} // namespace ayt::plugin