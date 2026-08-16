# AYPlugin

AYPlugin 提供动态库加载、插件生命周期、依赖检查和插件管理契约。

## 公开接口

```cpp
#include <AYPlugin.h>
#include <AYPlugin/IPlugin.h>
#include <AYPlugin/IPluginLoader.h>
#include <AYPlugin/IPluginManager.h>
```

稳定接口位于 `interface/AYPlugin/`，加载器实现头位于 `include/AYPlugin/`。

## 依赖

- AYCore
- AYTest（仅测试）

静态注册、动态加载和生命周期约束见 [design.md](design.md)。
