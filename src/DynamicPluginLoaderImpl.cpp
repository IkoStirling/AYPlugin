#include "AYPlugin/DynamicPluginLoader.h"
#include <memory>

namespace ayt::plugin
{

DynamicPluginLoaderImpl::DynamicPluginLoaderImpl(const std::string& path)
    : _lib(path)
{
}

bool DynamicPluginLoaderImpl::load(const std::string& path)
{
    if (path != _lib.getPath()) {
        _lastError = "Plugin path does not match loader path";
        return false;
    }
    if (!_lib.load()) return false;

    auto create = reinterpret_cast<CreatePluginFunc>(_lib.getSymbol("CreatePlugin"));
    if (!create) {
        _lastError = "Failed to find CreatePlugin function";
        return false;
    }

    _plugin = create();
    if (!_plugin) {
        _lastError = "CreatePlugin returned null";
        return false;
    }

    return _plugin->initialize();
}

void DynamicPluginLoaderImpl::unload()
{
    if (_plugin) {
        _plugin->shutdown();
        delete _plugin;
        _plugin = nullptr;
    }
    _lib.unload();
}

std::unique_ptr<IPluginLoader> IPluginLoader::create(const std::string& path) {
    return std::make_unique<DynamicPluginLoaderImpl>(path);
}

} // namespace ayt::plugin
