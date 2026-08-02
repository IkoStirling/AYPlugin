cmake_minimum_required(VERSION 3.20)

project(AYPlugin
    VERSION 0.1.0
    DESCRIPTION "AYPlugin — DynamicLib + PluginManager contract"
)

set(SOURCES
    src/DynamicLibImpl.cpp
    src/DynamicPluginLoaderImpl.cpp
    src/PluginManagerImpl.cpp
)

set(HEADERS
    interface/IAYPlugin.h
    interface/IAYPluginLoader.h
    interface/IAYPluginManager.h
    include/AYPlugin.h
    include/DynamicLib.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    AYCore
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
    $<$<CXX_COMPILER_ID:GNU,Clang>: -Wall -Wextra -Wpedantic>
)

message(STATUS "AYPlugin v0.1.0 (DynamicLib + PluginManager baseline contract) — see ${CMAKE_CURRENT_SOURCE_DIR}/design.md")