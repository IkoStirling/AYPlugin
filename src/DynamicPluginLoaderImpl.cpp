#include "DynamicPluginLoader.h"
#include "DynamicLib.h"

namespace ayt::plugin
{

class DynamicPluginLoaderImpl : public IPluginLoader {
public:
    explicit DynamicPluginLoaderImpl(const std::string& path)
        : _lib(path) {}

    bool load() override {
        if (!_lib.load()) return false;

        auto create = reinterpret_cast<CreatePluginFunc>(
            _lib.getSymbol("CreatePlugin")
        );
        if (!create) {
            _lastError = "Failed to find CreatePlugin function";
            return false;
        }

        _plugin = create();
        if (!_plugin) {
            _lastError = "CreatePlugin returned null";
            return false;
        }

        return _plugin->initialize();
    }

    void unload() override {
        if (_plugin) {
            _plugin->shutdown();
            delete _plugin;
            _plugin = nullptr;
        }
        _lib.unload();
    }

    IPlugin* getPlugin() const override { return _plugin; }
    bool isLoaded() const override { return _plugin != nullptr; }
    const std::string& getPath() const override { return _lib.getPath(); }
    const char* getLastError() const { return _lastError.c_str(); }

private:
    DynamicLib _lib;
    IPlugin* _plugin = nullptr;
    std::string _lastError;
};

std::unique_ptr<IPluginLoader> IPluginLoader::create(const std::string& path) {
    return std::make_unique<DynamicPluginLoaderImpl>(path);
}

} // namespace ayt::plugin