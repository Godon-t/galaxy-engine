#pragma once

#if defined(__GNUC__) || defined(__clang__)
#ifdef GALAXY_ENGINE_BUILD
#define API __attribute__((visibility("default")))
#else
#define API
#endif
#elif defined(_WIN32) || defined(_WIN64)
#ifdef GALAXY_ENGINE_BUILD
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif

// They are defined in log.hpp
#define GLX_CORE_ERROR
#define GLX_ERROR
#define GALAXY_ENABLE_ASSERTS

#ifdef GALAXY_ENABLE_ASSERTS
#ifdef _WIN32
#define GLX_DEBUGBREAK() __debugbreak()
#else
#include <csignal>
#define GLX_DEBUGBREAK() raise(SIGTRAP)
#endif

#define GLX_ASSERT(x, ...)                                   \
    {                                                        \
        if (!(x)) {                                          \
            GLX_ERROR("Assertion failed: {0}", __VA_ARGS__); \
            GLX_DEBUGBREAK();                                \
        }                                                    \
    }
#define GLX_CORE_ASSERT(x, ...)                                   \
    {                                                             \
        if (!(x)) {                                               \
            GLX_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); \
            GLX_DEBUGBREAK();                                     \
        }                                                         \
    }
#else
#define GLX_ASSERT(x, ...)
#define GLX_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
