cmake_minimum_required(VERSION 3.20)

project(AYPluginTest)

set(SOURCES
    TestMain.cpp
    PluginManagerTest.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

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

ay_configure_test_tmpdir(${PROJECT_NAME})
