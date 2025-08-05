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
