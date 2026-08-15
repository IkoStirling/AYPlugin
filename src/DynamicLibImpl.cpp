#include "AYPlugin/DynamicLib.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace ayt::plugin
{

DynamicLib::DynamicLib(const std::string& path)
    : _path(path) {}

DynamicLib::~DynamicLib() {
    unload();
}

bool DynamicLib::load() {
    if (_handle) return true;

#if defined(_WIN32)
    _handle = LoadLibraryA(_path.c_str());
    if (!_handle) {
        _lastError = "Failed to load library";
        return false;
    }
#else
    _handle = dlopen(_path.c_str(), RTLD_NOW);
    if (!_handle) {
        _lastError = dlerror();
        return false;
    }
#endif
    return true;
}

void DynamicLib::unload() {
    if (!_handle) return;

#if defined(_WIN32)
    FreeLibrary(_handle);
#else
    dlclose(_handle);
#endif
    _handle = nullptr;
}

void* DynamicLib::getSymbol(const char* symbol) {
    if (!_handle) return nullptr;

#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(_handle, symbol));
#else
    return dlsym(_handle, symbol);
#endif
}

const char* DynamicLib::getLastError() const {
    return _lastError.c_str();
}

} // namespace ayt::plugin