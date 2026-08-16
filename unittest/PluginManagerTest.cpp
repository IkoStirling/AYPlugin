#include <AYPlugin.h>
#include <AYTest.h>

namespace ayt::plugin::test
{

TEST_SUITE(PluginManagerTests)

TEST_CASE(Singleton) {
    auto& mgr1 = IPluginManager::instance();
    auto& mgr2 = IPluginManager::instance();
    CHECK(&mgr1 == &mgr2);
}

TEST_CASE(RegisterStatic) {
    auto& mgr = IPluginManager::instance();
    mgr.unloadAll();

    struct TestPlugin : public IPlugin {
        const char* getName() const override { return "Test"; }
        const char* getVersion() const override { return "1.0.0"; }
        const char* getDescription() const override { return "Test plugin"; }
        bool initialize() override { return true; }
        void shutdown() override {}
        std::vector<const char*> getDependencies() const override { return {}; }
    };

    TestPlugin plugin;
    mgr.registerStaticPlugin(&plugin);

    CHECK_TRUE(mgr.hasPlugin("Test"));
    CHECK(mgr.getPlugin("Test") == &plugin);
}

TEST_CASE(DependencyCheck) {
    auto& mgr = IPluginManager::instance();
    mgr.unloadAll();

    struct BasePlugin : public IPlugin {
        const char* getName() const override { return "Base"; }
        const char* getVersion() const override { return "1.0.0"; }
        const char* getDescription() const override { return "Base plugin"; }
        bool initialize() override { return true; }
        void shutdown() override {}
        std::vector<const char*> getDependencies() const override { return {}; }
    };

    struct DepPlugin : public IPlugin {
        const char* getName() const override { return "Dep"; }
        const char* getVersion() const override { return "1.0.0"; }
        const char* getDescription() const override { return "Depends on Base"; }
        bool initialize() override { return true; }
        void shutdown() override {}
        std::vector<const char*> getDependencies() const override { return {"Base"}; }
    };

    BasePlugin base;
    DepPlugin dep;

    mgr.registerStaticPlugin(&base);
    mgr.registerStaticPlugin(&dep);

    CHECK_TRUE(mgr.checkDependencies(&base));
    CHECK_TRUE(mgr.checkDependencies(&dep));
}

TEST_SUITE_END

} // namespace ayt::plugin::test
