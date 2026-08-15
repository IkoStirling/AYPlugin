# AYPlugin Design

## 1. 概述

AYPlugin 是 AY Engine 的**插件系统**，负责：
- 插件加载与卸载
- 插件生命周期管理
- 依赖检查
- 静态注册（开发期）和动态加载（发布期）

### 1.1 设计目标

- **双模式支持**：静态注册（开发期）+ 动态加载（发布期）
- **依赖管理**：检查插件依赖，确保加载顺序
- **热重载**：运行时重新加载插件
- **沙箱隔离**：插件错误不崩溃主程序

### 1.2 在引擎中的位置

```
┌─────────────────────────────────────────────────────────────────┐
│                        Engine Core                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │               PluginManager                               │   │
│  │  - discoverAndLoad() 扫描并加载                           │   │
│  │  - reload() 热重载                                         │   │
│  │  - getPlugin() 获取插件                                   │   │
│  └────────────────────────┬────────────────────────────────┘   │
│                           │                                      │
│       ┌───────────────────┼───────────────────┐                 │
│       │                   │                   │                 │
│       ▼                   ▼                   ▼                 │
│  ┌───────────┐  ┌───────────────┐  ┌───────────────┐             │
│  │  Static   │  │  Dynamic      │  │   Manifest    │             │
│  │  Plugin   │  │  Plugin       │  │   Registry    │             │
│  │  Registry │  │  Loader       │  │               │             │
│  └───────────┘  └───────────────┘  └───────────────┘             │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. 核心架构

### 2.1 静态注册

编译时自动注册，无需 DLL：

```cpp
// 插件头文件
class MyPlugin : public IPlugin {
public:
    const char* getName() const override { return "MyPlugin"; }
    const char* getVersion() const override { return "1.0.0"; }
    
    bool initialize() override {
        // 注册到系统
        return true;
    }
    
    void shutdown() override {
        // 清理资源
    }
};

// 静态注册宏
REGISTER_PLUGIN(MyPlugin);

// 编译时 g_registrar 构造，自动调用 registerPlugin()
// 插件代码直接编译进 exe，无需 DLL
```

### 2.2 动态加载

```
plugins/                           # 插件目录
├── my_plugin.dll                  # Windows
├── my_plugin.so                  # Linux
└── manifest.json                 # 插件清单

manifest.json:
{
    "name": "MyPlugin",
    "version": "1.0.0",
    "entry": "CreatePlugin",
    "dependencies": ["Core", "Renderer"]
}
```

### 2.3 IPlugin 接口

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual const char* getName() const = 0;
    virtual const char* getVersion() const = 0;
    
    // 生命周期
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // 元数据
    virtual const char* getDescription() const = 0;
    virtual std::vector<const char*> getDependencies() const = 0;
};
```

### 2.4 IPluginLoader 接口

```cpp
class IPluginLoader {
public:
    virtual ~IPluginLoader() = default;

    // 加载插件
    virtual bool load(const std::string& path) = 0;
    
    // 卸载插件
    virtual void unload() = 0;
    
    // 获取插件实例
    virtual IPlugin* getPlugin() const = 0;
    
    // 查询
    virtual bool isLoaded() const = 0;
    virtual const std::string& getPath() const = 0;
};
```

### 2.5 PluginManager

```cpp
class PluginManager {
public:
    static PluginManager& instance();

    // === 静态注册 ===
    void registerStaticPlugin(IPlugin* plugin);
    std::vector<IPlugin*> getStaticPlugins() const;
    
    // === 动态加载 ===
    bool loadDynamicPlugin(const std::string& path);
    void unloadDynamicPlugin(const char* name);
    void reloadPlugin(const char* name);
    
    // === 批量操作 ===
    void discoverAndLoad(const std::string& pluginDir);
    void unloadAll();
    
    // === 查询 ===
    IPlugin* getPlugin(const char* name) const;
    bool hasPlugin(const char* name) const;
    std::vector<const char*> getPluginNames() const;
    
    // === 依赖检查 ===
    bool checkDependencies(IPlugin* plugin) const;
    std::vector<const char*> getMissingDependencies(const char* name) const;

private:
    PluginManager() = default;
    
    std::vector<IPlugin*> _staticPlugins;
    std::unordered_map<std::string, std::unique_ptr<IPluginLoader>> _dynamicLoaders;
    std::unordered_map<std::string, IPlugin*> _pluginIndex;  // name → plugin
};
```

---

## 3. 动态加载实现

### 3.1 平台抽象

```cpp
// AYPlugin/DynamicLib.h - 平台抽象动态库加载
class DynamicLib {
public:
    explicit DynamicLib(const std::string& path);
    ~DynamicLib();
    
    // 加载/卸载
    bool load();
    void unload();
    
    // 符号查找
    void* getSymbol(const char* symbol);
    
    // 查询
    bool isLoaded() const { return _handle != nullptr; }
    const std::string& getPath() const { return _path; }
    
private:
    std::string _path;
    
#if defined(_WIN32)
    using Handle = HMODULE;
#else
    using Handle = void*;
#endif
    Handle _handle;
};

// 入口函数声明
extern "C" IPlugin* CreatePlugin();
```

### 3.2 DynamicPluginLoader

```cpp
class DynamicPluginLoader : public IPluginLoader {
public:
    explicit DynamicPluginLoader(const std::string& path);
    ~DynamicPluginLoader() override;
    
    bool load() override;
    void unload() override;
    IPlugin* getPlugin() const override { return _plugin; }
    bool isLoaded() const override { return _plugin != nullptr; }
    const std::string& getPath() const override { return _lib.getPath(); }

private:
    DynamicLib _lib;
    IPlugin* _plugin = nullptr;
};

// DynamicPluginLoader.cpp
DynamicPluginLoader::DynamicLoader(const std::string& path)
    : _lib(path) {}

bool DynamicPluginLoader::load() {
    if (!_lib.load()) return false;
    
    // 获取入口函数
    auto create = reinterpret_cast<CreatePluginFunc>(
        _lib.getSymbol("CreatePlugin")
    );
    if (!create) return false;
    
    // 创建插件实例
    _plugin = create();
    return _plugin->initialize();
}

void DynamicPluginLoader::unload() {
    if (_plugin) {
        _plugin->shutdown();
        delete _plugin;
        _plugin = nullptr;
    }
    _lib.unload();
}
```

---

## 4. 静态注册实现

### 4.1 StaticPluginRegistry

```cpp
class StaticPluginRegistry {
public:
    static StaticPluginRegistry& instance();
    
    void registerPlugin(IPlugin* plugin) {
        _plugins.push_back(plugin);
        PluginManager::instance().registerStaticPlugin(plugin);
    }
    
    std::vector<IPlugin*> getPlugins() const {
        return _plugins;
    }
    
private:
    std::vector<IPlugin*> _plugins;
};

#define REGISTER_PLUGIN(cls) \
    static_assert(std::is_base_of_v<IPlugin, cls>, #cls " must inherit IPlugin"); \
    namespace { \
        struct cls##_PluginRegistrar { \
            cls##_PluginRegistrar() { \
                StaticPluginRegistry::instance().registerPlugin(new cls()); \
            } \
        }; \
        static cls##_PluginRegistrar g_plugin_registrar; \
    }
```

---

## 5. 依赖检查

### 5.1 依赖图

```cpp
bool PluginManager::checkDependencies(IPlugin* plugin) const {
    auto deps = plugin->getDependencies();
    for (const char* dep : deps) {
        if (!hasPlugin(dep)) {
            AY_LOG_WARN("Plugin '%s' missing dependency: '%s'",
                plugin->getName(), dep);
            return false;
        }
    }
    return true;
}

std::vector<const char*> PluginManager::getLoadOrder() const {
    // Kahn's algorithm 拓扑排序
    // 按依赖关系排序，确保先加载依赖
}
```

### 5.2 Manifest 格式

```json
{
    "name": "MyRendererPlugin",
    "version": "1.0.0",
    "description": "Custom renderer plugin",
    "entry": "CreatePlugin",
    "dependencies": [
        "Core",
        "Renderer"
    ],
    "author": "Developer",
    "license": "MIT"
}
```

---

## 6. 热重载

### 6.1 热重载流程

```cpp
void PluginManager::reloadPlugin(const char* name) {
    // 1. 查找插件
    auto it = _dynamicLoaders.find(name);
    if (it == _dynamicLoaders.end()) {
        AY_LOG_ERROR("Plugin '%s' not found or not dynamically loaded", name);
        return;
    }
    
    // 2. 获取路径
    std::string path = it->second->getPath();
    
    // 3. 卸载
    it->second->unload();
    
    // 4. 重新加载
    it->second->load();
    
    AY_LOG_INFO("Plugin '%s' reloaded successfully", name);
}
```

### 6.2 热重载事件

```cpp
struct PluginReloadedEvent : IEvent {
    static constexpr EventPriority kPriority = EventPriority::Normal;
    const char* pluginName;
    
    uint32_t typeId() const override { return kTypeId; }
    EventPriority priority() const override { return kPriority; }
};
```

---

## 7. 目录结构

```
AYPlugin/
├── design.md
├── CMakeLists.txt
├── interface/
│   ├── AYPlugin/IPlugin.h            # 插件接口
│   ├── AYPlugin/IPluginLoader.h      # 加载器接口
│   └── AYPlugin/IPluginManager.h     # 管理器接口
│
├── include/
│   ├── AYPlugin\Plugin.h             # 主入口
│   ├── AYPlugin/PluginManager.h        # 插件管理器
│   ├── StaticPluginRegistry.h # 静态注册表
│   └── AYPlugin/DynamicLib.h           # 动态库抽象
│
├── src/
│   ├── PluginManager.cpp
│   ├── StaticPluginRegistry.cpp
│   ├── DynamicPluginLoader.cpp
│   └── DynamicLib.cpp         # 平台实现
│
└── unittest/
    ├── CMakeLists.txt
    ├── PluginManagerTest.cpp
    └── TestMain.cpp
```

---

## 8. 实现优先级

### Phase 1: 核心
- [ ] IPlugin 接口
- [ ] 静态注册宏
- [ ] StaticPluginRegistry

### Phase 2: 动态加载
- [ ] DynamicLib 平台抽象
- [ ] IPluginLoader 接口
- [ ] DynamicPluginLoader 实现

### Phase 3: 管理器
- [ ] PluginManager 单例
- [ ] discoverAndLoad() 批量加载
- [ ] 依赖检查

### Phase 4: 热重载
- [ ] reloadPlugin()
- [ ] 热重载事件
- [ ] 错误恢复

---

## 9. 参考

- [O3DE Plugin System](https://docs.o3de.org/docs/user-guide/components/)
- [Unreal Engine Module System](https://docs.unrealengine.com/en-US/Programming/Modules/)
- [Qt Plugin System](https://doc.qt.io/qt-5/plugins.html)