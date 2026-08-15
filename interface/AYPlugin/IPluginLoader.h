#pragma once
// AYPlugin/IPluginLoader.h - 插件加载器接口

#include <AYPlugin/IPlugin.h>

namespace ayt::plugin
{

class IPluginLoader {
public:
    virtual ~IPluginLoader() = default;

    static std::unique_ptr<IPluginLoader> create(const std::string& path);

    virtual bool load(const std::string& path) = 0;
    virtual void unload() = 0;
    virtual IPlugin* getPlugin() const = 0;
    virtual bool isLoaded() const = 0;
    virtual const std::string& getPath() const = 0;
};

} // namespace ayt::plugin
