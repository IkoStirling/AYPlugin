#pragma once
// AYPlugin/DynamicPluginLoader.h - 动态插件加载器

#include <AYPlugin/IPluginLoader.h>
#include <AYPlugin/DynamicLib.h>

namespace ayt::plugin
{

class DynamicPluginLoaderImpl : public IPluginLoader {
public:
    explicit DynamicPluginLoaderImpl(const std::string& path);

    bool load(const std::string& path) override;
    void unload() override;
    IPlugin* getPlugin() const override { return _plugin; }
    bool isLoaded() const override { return _plugin != nullptr; }
    const std::string& getPath() const override { return _lib.getPath(); }
    const char* getLastError() const { return _lastError.c_str(); }

private:
    DynamicLib _lib;
    IPlugin* _plugin = nullptr;
    std::string _lastError;
};

} // namespace ayt::plugin