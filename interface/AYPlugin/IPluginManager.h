#pragma once
// AYPlugin/IPluginManager.h - 插件管理器接口

#include <AYPlugin/IPlugin.h>

namespace ayt::plugin
{

class IPluginManager {
public:
    virtual ~IPluginManager() = default;

    static IPluginManager& instance();

    virtual void registerStaticPlugin(IPlugin* plugin) = 0;
    virtual std::vector<IPlugin*> getStaticPlugins() const = 0;

    virtual bool loadDynamicPlugin(const std::string& path) = 0;
    virtual void unloadDynamicPlugin(const char* name) = 0;
    virtual void reloadPlugin(const char* name) = 0;

    virtual void discoverAndLoad(const std::string& pluginDir) = 0;
    virtual void unloadAll() = 0;

    virtual IPlugin* getPlugin(const char* name) const = 0;
    virtual bool hasPlugin(const char* name) const = 0;
    virtual std::vector<const char*> getPluginNames() const = 0;

    virtual bool checkDependencies(IPlugin* plugin) const = 0;
    virtual std::vector<const char*> getMissingDependencies(const char* name) const = 0;
};

#define REGISTER_PLUGIN(cls) \
    static_assert(std::is_base_of_v<IPlugin, cls>, #cls " must inherit IPlugin"); \
    namespace { \
        struct cls##_PluginRegistrar { \
            cls##_PluginRegistrar() { \
                ayt::plugin::IPluginManager::instance().registerStaticPlugin(new cls()); \
            } \
        }; \
        static cls##_PluginRegistrar g_plugin_registrar; \
    }

} // namespace ayt::plugin
