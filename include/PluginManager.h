#pragma once
// PluginManager.h - 插件管理器

#include <IAYPluginManager.h>

namespace ayt::plugin
{

class PluginManagerImpl : public IPluginManager {
public:
    void registerStaticPlugin(IPlugin* plugin) override;
    std::vector<IPlugin*> getStaticPlugins() const override;

    bool loadDynamicPlugin(const std::string& path) override;
    void unloadDynamicPlugin(const char* name) override;
    void reloadPlugin(const char* name) override;

    void discoverAndLoad(const std::string& pluginDir) override;
    void unloadAll() override;

    IPlugin* getPlugin(const char* name) const override;
    bool hasPlugin(const char* name) const override;
    std::vector<const char*> getPluginNames() const override;

    bool checkDependencies(IPlugin* plugin) const override;
    std::vector<const char*> getMissingDependencies(const char* name) const override;

private:
    std::vector<IPlugin*> _staticPlugins;
    std::unordered_map<std::string, std::unique_ptr<IPluginLoader>> _dynamicLoaders;
    std::unordered_map<std::string, IPlugin*> _pluginIndex;
};

} // namespace ayt::plugin