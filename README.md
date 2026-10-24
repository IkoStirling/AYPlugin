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

加载器析构会执行卸载；工厂缺失、返回空、抛异常或 initialize 失败时回收动态库。
重复 load 拒绝。Windows 路径按 UTF-8 转换为原生宽字符串，支持中文目录；依赖搜索
使用库所在目录与系统默认安全搜索位置。公开接口头不再引入 AYCore 的全局兼容宏，
可以与 Windows COM/SAPI 头共同使用。原生插件仍须与宿主编译器/运行库匹配，
不提供进程隔离。对应真实 DLL 生命周期/中文路径测试位于 unittest。
