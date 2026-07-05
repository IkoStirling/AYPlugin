#include "PluginManager.h"

namespace ayt::plugin
{

class PluginManagerImpl : public IPluginManager {
public:
    void registerStaticPlugin(IPlugin* plugin) override {
        if (!plugin) return;
        _staticPlugins.push_back(plugin);
        _pluginIndex[plugin->getName()] = plugin;
    }

    std::vector<IPlugin*> getStaticPlugins() const override {
        return _staticPlugins;
    }

    bool loadDynamicPlugin(const std::string& path) override {
        auto loader = IPluginLoader::create(path);
        if (!loader->load()) {
            return false;
        }

        auto* plugin = loader->getPlugin();
        if (!checkDependencies(plugin)) {
            loader->unload();
            return false;
        }

        _dynamicLoaders[plugin->getName()] = std::move(loader);
        _pluginIndex[plugin->getName()] = plugin;
        return true;
    }

    void unloadDynamicPlugin(const char* name) override {
        auto it = _dynamicLoaders.find(name);
        if (it != _dynamicLoaders.end()) {
            it->second->unload();
            _dynamicLoaders.erase(it);
            _pluginIndex.erase(name);
        }
    }

    void reloadPlugin(const char* name) override {
        auto it = _dynamicLoaders.find(name);
        if (it == _dynamicLoaders.end()) return;

        std::string path = it->second->getPath();
        unloadDynamicPlugin(name);
        loadDynamicPlugin(path);
    }

    void discoverAndLoad(const std::string& pluginDir) override {
        // TODO: 扫描目录，加载所有 DLL
    }

    void unloadAll() override {
        for (auto& [name, loader] : _dynamicLoaders) {
            loader->unload();
        }
        _dynamicLoaders.clear();
        _pluginIndex.clear();
        _staticPlugins.clear();
    }

    IPlugin* getPlugin(const char* name) const override {
        auto it = _pluginIndex.find(name);
        return (it != _pluginIndex.end()) ? it->second : nullptr;
    }

    bool hasPlugin(const char* name) const override {
        return _pluginIndex.find(name) != _pluginIndex.end();
    }

    std::vector<const char*> getPluginNames() const override {
        std::vector<const char*> names;
        for (const auto& [name, plugin] : _pluginIndex) {
            names.push_back(name.c_str());
        }
        return names;
    }

    bool checkDependencies(IPlugin* plugin) const override {
        for (const char* dep : plugin->getDependencies()) {
            if (!hasPlugin(dep)) {
                return false;
            }
        }
        return true;
    }

    std::vector<const char*> getMissingDependencies(const char* name) const override {
        std::vector<const char*> missing;
        auto* plugin = getPlugin(name);
        if (!plugin) return missing;

        for (const char* dep : plugin->getDependencies()) {
            if (!hasPlugin(dep)) {
                missing.push_back(dep);
            }
        }
        return missing;
    }

private:
    std::vector<IPlugin*> _staticPlugins;
    std::unordered_map<std::string, std::unique_ptr<IPluginLoader>> _dynamicLoaders;
    std::unordered_map<std::string, IPlugin*> _pluginIndex;
};

IPluginManager& IPluginManager::instance() {
    static PluginManagerImpl inst;
    return inst;
}

} // namespace ayt::plugin