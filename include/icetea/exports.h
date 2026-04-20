#pragma once

/// @file exports.h
/// @brief DLL export/import macros and compiler optimization hints for icetea.

#if defined(ICETEA_BUILD_INTERNAL)
    #if defined(_MSC_VER)
        #define ICETEA_API __declspec(dllexport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define ICETEA_API __attribute__((visibility("default")))
    #else
        #define ICETEA_API
    #endif
#else
    #if defined(_MSC_VER)
        #define ICETEA_API __declspec(dllimport)
    #else
        #define ICETEA_API
    #endif
#endif

// ── Optimization hints ────────────────────────────────────────────────────
#if defined(_MSC_VER)
    #define ICETEA_NOINLINE __declspec(noinline)
    #define ICETEA_COLD     __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define ICETEA_NOINLINE __attribute__((noinline))
    #define ICETEA_COLD     __attribute__((noinline, cold))
#else
    #define ICETEA_NOINLINE
    #define ICETEA_COLD
#endif
