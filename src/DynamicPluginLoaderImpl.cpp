#include "AYPlugin/DynamicPluginLoader.h"
#include <memory>

namespace ayt::plugin
{

DynamicPluginLoaderImpl::DynamicPluginLoaderImpl(const std::string& path)
    : _lib(path)
{
}
DynamicPluginLoaderImpl::~DynamicPluginLoaderImpl(){unload();}

bool DynamicPluginLoaderImpl::load(const std::string& path)
{
    if (_plugin) { _lastError="Plugin is already loaded";return false; }
    if (path != _lib.getPath()) {
        _lastError = "Plugin path does not match loader path";
        return false;
    }
    if (!_lib.load()) return false;

    auto create = reinterpret_cast<CreatePluginFunc>(_lib.getSymbol("CreatePlugin"));
    if (!create) {
        _lastError = "Failed to find CreatePlugin function";
        _lib.unload();
        return false;
    }

    try { _plugin = create(); } catch (...) {_lib.unload();_lastError="Plugin creation failed";return false;}
    if (!_plugin) {
        _lastError = "CreatePlugin returned null";
        _lib.unload();
        return false;
    }

    try {if (_plugin->initialize()) return true;}catch(...){}
    _lastError="Plugin initialization failed";unload();return false;
}

void DynamicPluginLoaderImpl::unload()
{
    if (_plugin) {
        try {_plugin->shutdown();}catch(...){}
        delete _plugin;
        _plugin = nullptr;
    }
    _lib.unload();
}

std::unique_ptr<IPluginLoader> IPluginLoader::create(const std::string& path) {
    return std::make_unique<DynamicPluginLoaderImpl>(path);
}

} // namespace ayt::plugin
