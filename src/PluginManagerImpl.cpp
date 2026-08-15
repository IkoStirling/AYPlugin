#include "AYPlugin/PluginManager.h"

namespace ayt::plugin
{

void PluginManagerImpl::registerStaticPlugin(IPlugin* plugin) {
    if (!plugin) return;
    _staticPlugins.push_back(plugin);
    _pluginIndex[plugin->getName()] = plugin;
}

std::vector<IPlugin*> PluginManagerImpl::getStaticPlugins() const {
    return _staticPlugins;
}

bool PluginManagerImpl::loadDynamicPlugin(const std::string& path) {
    auto loader = IPluginLoader::create(path);
    if (!loader->load(path)) return false;

    auto* plugin = loader->getPlugin();
    if (!checkDependencies(plugin)) {
        loader->unload();
        return false;
    }

    _dynamicLoaders[plugin->getName()] = std::move(loader);
    _pluginIndex[plugin->getName()] = plugin;
    return true;
}

void PluginManagerImpl::unloadDynamicPlugin(const char* name) {
    auto it = _dynamicLoaders.find(name);
    if (it != _dynamicLoaders.end()) {
        it->second->unload();
        _dynamicLoaders.erase(it);
        _pluginIndex.erase(name);
    }
}

void PluginManagerImpl::reloadPlugin(const char* name) {
    auto it = _dynamicLoaders.find(name);
    if (it == _dynamicLoaders.end()) return;
    std::string path = it->second->getPath();
    unloadDynamicPlugin(name);
    loadDynamicPlugin(path);
}

void PluginManagerImpl::discoverAndLoad(const std::string& pluginDir) {
    (void)pluginDir;
    // TODO: 扫描目录，加载所有 DLL
}

void PluginManagerImpl::unloadAll() {
    for (auto& [name, loader] : _dynamicLoaders) {
        (void)name;
        loader->unload();
    }
    _dynamicLoaders.clear();
    _pluginIndex.clear();
    _staticPlugins.clear();
}

IPlugin* PluginManagerImpl::getPlugin(const char* name) const {
    auto it = _pluginIndex.find(name);
    return (it != _pluginIndex.end()) ? it->second : nullptr;
}

bool PluginManagerImpl::hasPlugin(const char* name) const {
    return _pluginIndex.find(name) != _pluginIndex.end();
}

std::vector<const char*> PluginManagerImpl::getPluginNames() const {
    std::vector<const char*> names;
    for (const auto& [name, plugin] : _pluginIndex) {
        (void)plugin;
        names.push_back(name.c_str());
    }
    return names;
}

bool PluginManagerImpl::checkDependencies(IPlugin* plugin) const {
    if (!plugin) return false;
    for (const char* dep : plugin->getDependencies()) {
        if (!hasPlugin(dep)) return false;
    }
    return true;
}

std::vector<const char*> PluginManagerImpl::getMissingDependencies(const char* name) const {
    std::vector<const char*> missing;
    auto* plugin = getPlugin(name);
    if (!plugin) return missing;
    for (const char* dep : plugin->getDependencies()) {
        if (!hasPlugin(dep)) missing.push_back(dep);
    }
    return missing;
}

IPluginManager& IPluginManager::instance() {
    static PluginManagerImpl inst;
    return inst;
}

} // namespace ayt::plugin
