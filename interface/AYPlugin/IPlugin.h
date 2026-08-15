#pragma once
// AYPlugin/IPlugin.h - 插件接口

#include <AYCore.h>

namespace ayt::plugin
{

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual const char* getName() const = 0;
    virtual const char* getVersion() const = 0;
    virtual const char* getDescription() const = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual std::vector<const char*> getDependencies() const = 0;
};

// 插件入口函数类型
using CreatePluginFunc = IPlugin* (*)();

} // namespace ayt::plugin