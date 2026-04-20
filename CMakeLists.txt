# icetea — ICU4X C++ wrapper (COCA workspace build)
# Provides: icetea (SHARED), icetea::icetea
# Depends on: COCA::icu4x (pre-built ICU4X C FFI static library from CommonDependencies)

# ── Sources ──────────────────────────────────────────────────────────────
set(ICETEA_SOURCES
    src/icetea.cpp
    src/icu4x_bridge.cpp
    src/unicode_utils.cpp
)
# Use stubs when COCA::icu4x is not available (e.g. wasm, missing profile)
# Check if COCA::icu4x target exists AND has actual link libraries set
# (_COCA_ARCH_AVAILABLE is local to build.cmake scope, so we probe the target instead)
set(_ICETEA_HAS_ICU4X OFF)
if(TARGET COCA::icu4x)
    get_target_property(_icu4x_libs COCA::icu4x INTERFACE_LINK_LIBRARIES)
    if(_icu4x_libs)
        set(_ICETEA_HAS_ICU4X ON)
    endif()
endif()
if(NOT _ICETEA_HAS_ICU4X)
    list(APPEND ICETEA_SOURCES src/icu4x_stubs.c)
endif()

# ── Shared library ───────────────────────────────────────────────────────
if(EMSCRIPTEN)
    add_library(icetea STATIC ${ICETEA_SOURCES})
else()
    add_library(icetea SHARED ${ICETEA_SOURCES})
endif()
add_library(icetea::icetea ALIAS icetea)

target_include_directories(icetea
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(icetea PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_definitions(icetea PRIVATE ICETEA_BUILD_INTERNAL)

if(MSVC)
    target_compile_definitions(icetea PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(icetea PRIVATE /W4 /utf-8 /permissive-)
    target_compile_options(icetea PRIVATE
        $<$<CONFIG:Release>:/Oi /GS- /fp:fast /GL>
        $<$<CONFIG:RelWithDebInfo>:/Oi /fp:fast>
    )
    target_link_options(icetea PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
    )
else()
    target_compile_options(icetea PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── ICU4X linkage (pre-built from CommonDependencies) ────────────────────
if(_ICETEA_HAS_ICU4X)
    target_link_libraries(icetea PRIVATE COCA::icu4x)
endif()

# ── Tests ────────────────────────────────────────────────────────────────
if(TARGET gtest AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
