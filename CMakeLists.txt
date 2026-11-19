cmake_minimum_required(VERSION 3.28)

project(EnsoRetreat
    VERSION 1.0
    DESCRIPTION "Enso Retreat: break reminder"
    LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

if(WIN32)
    set(_retreat_app_default ON)
else()
    set(_retreat_app_default OFF)
endif()

option(RETREAT_BUILD_APP    "Build the Windows application (retreat.exe)" ${_retreat_app_default})
option(RETREAT_BUILD_PYTHON "Build the Enso Python extension (retreatlib.pyd)" OFF)
option(RETREAT_BUILD_TESTS  "Build the portable core smoke test" ON)
set(RETREAT_PYD_OUTPUT_DIR "" CACHE PATH "Where to place retreatlib.pyd (default: build dir)")

if((RETREAT_BUILD_APP OR RETREAT_BUILD_PYTHON) AND NOT WIN32)
    message(FATAL_ERROR "The Retreat UI is Windows-only; configure with -DRETREAT_BUILD_APP=OFF")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Static CRT, as in the original Visual Studio project.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(retreat_options INTERFACE)
if(MSVC)
    target_compile_options(retreat_options INTERFACE
        /permissive- /utf-8 /W3 /GR- /EHsc
        /wd4005 /wd5050 /wd5105 /wd5106)
else()
    target_compile_options(retreat_options INTERFACE -Wall -Wno-unused-variable)
endif()

add_subdirectory(src/core)

if(RETREAT_BUILD_APP OR RETREAT_BUILD_PYTHON)
    add_subdirectory(src/win32)
endif()

if(RETREAT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
