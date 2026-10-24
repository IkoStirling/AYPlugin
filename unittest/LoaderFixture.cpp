#include <AYPlugin/IPlugin.h>
#include <atomic>
#if defined(_WIN32)
#define AY_FIXTURE_EXPORT __declspec(dllexport)
#else
#define AY_FIXTURE_EXPORT __attribute__((visibility("default")))
#endif
namespace {std::atomic<int> live=0,fail=0;
class Fixture:public ayt::plugin::IPlugin {
public:
    Fixture(){++live;}~Fixture()override{--live;}
    const char* getName()const override{return "LoaderFixture";}const char* getVersion()const override{return "1";}const char* getDescription()const override{return "Loader lifecycle fixture";}
    bool initialize()override{return !fail;}void shutdown()override{}std::vector<const char*> getDependencies()const override{return {};}
};}
extern "C" AY_FIXTURE_EXPORT ayt::plugin::IPlugin* CreatePlugin(){return new Fixture;}
extern "C" AY_FIXTURE_EXPORT int FixtureLiveCount(){return live;}
extern "C" AY_FIXTURE_EXPORT void FixtureFailInitialize(int value){fail=value;}
