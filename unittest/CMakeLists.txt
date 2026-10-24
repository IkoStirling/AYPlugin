cmake_minimum_required(VERSION 3.20)

project(AYPluginTest)

set(SOURCES
    TestMain.cpp
    PluginManagerTest.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
add_library(AYPlugin_LoaderFixture SHARED LoaderFixture.cpp)
target_link_libraries(AYPlugin_LoaderFixture PRIVATE AYPlugin)
target_compile_definitions(${PROJECT_NAME} PRIVATE AY_PLUGIN_FIXTURE_PATH="$<TARGET_FILE:AYPlugin_LoaderFixture>")
add_dependencies(${PROJECT_NAME} AYPlugin_LoaderFixture)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../AYCore/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    AYPlugin
    AYTest
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
    $<$<CXX_COMPILER_ID:GNU,Clang>: -Wall -Wextra -Wpedantic>
)

ay_add_test(${PROJECT_NAME})
