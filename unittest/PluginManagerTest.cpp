#include <AYPlugin.h>
#include <AYPlugin/DynamicLib.h>
#ifdef Memory
#error "AYPlugin public headers must not export the legacy Memory macro"
#endif
#ifdef _WIN32
#include <objbase.h>
#endif
#include <AYTest.h>
#include <AYTestFixtures.h>
#include <filesystem>

namespace ayt::plugin::test
{

TEST_SUITE(PluginManagerTests)

TEST_CASE(Singleton) {
    auto& mgr1 = IPluginManager::instance();
    auto& mgr2 = IPluginManager::instance();
    CHECK(&mgr1 == &mgr2);
}
TEST_CASE(DynamicLoaderCleansFailedInitializationAndDestructorOwnership) {
    DynamicLib keeper(AY_PLUGIN_FIXTURE_PATH);CHECK(keeper.load());auto count=reinterpret_cast<int(*)()>(keeper.getSymbol("FixtureLiveCount"));auto fail=reinterpret_cast<void(*)(int)>(keeper.getSymbol("FixtureFailInitialize"));CHECK(count!=nullptr);CHECK(fail!=nullptr);if(!count||!fail)return;
    fail(1);{auto loader=IPluginLoader::create(AY_PLUGIN_FIXTURE_PATH);CHECK_FALSE(loader->load(AY_PLUGIN_FIXTURE_PATH));CHECK_FALSE(loader->isLoaded());CHECK_INT_EQ(count(),0);}
    fail(0);{auto loader=IPluginLoader::create(AY_PLUGIN_FIXTURE_PATH);CHECK(loader->load(AY_PLUGIN_FIXTURE_PATH));CHECK_INT_EQ(count(),1);CHECK_FALSE(loader->load(AY_PLUGIN_FIXTURE_PATH));CHECK_INT_EQ(count(),1);}CHECK_INT_EQ(count(),0);
}
#ifdef _WIN32
TEST_CASE(DynamicLibrarySupportsUtf8FileNames) {
    ayt::test::ScratchDirectory scratch("plugin-unicode");const auto path=scratch.path()/std::filesystem::u8path("角色插件.dll");std::filesystem::copy_file(AY_PLUGIN_FIXTURE_PATH,path);
    const auto utf=path.u8string();const std::string name(reinterpret_cast<const char*>(utf.data()),utf.size());auto loader=IPluginLoader::create(name);CHECK(loader->load(name));CHECK(loader->isLoaded());loader->unload();CHECK_FALSE(loader->isLoaded());
}
#endif

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
