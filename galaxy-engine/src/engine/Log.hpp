#pragma once

#include "pch.hpp"

#include "Core.hpp"
#include "spdlog/spdlog.h"

namespace Galaxy
{
    class API Log{
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {return s_CoreLogger;}
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {return s_ClientLogger;}
    };
}


#define GLX_CORE_TRACE(...)     ::Galaxy::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define GLX_CORE_INFO(...)      ::Galaxy::Log::GetCoreLogger()->info(__VA_ARGS__);
#define GLX_CORE_WARN(...)      ::Galaxy::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define GLX_CORE_ERROR(...)     ::Galaxy::Log::GetCoreLogger()->error(__VA_ARGS__);
#define GLX_CORE_FATAL(...)     ::Galaxy::Log::GetCoreLogger()->fatal(__VA_ARGS__);


#define GLX_TRACE(...)          ::Galaxy::Log::GetClientLogger()->trace(__VA_ARGS__);
#define GLX_INFO(...)           ::Galaxy::Log::GetClientLogger()->info(__VA_ARGS__);
#define GLX_WARN(...)           ::Galaxy::Log::GetClientLogger()->warn(__VA_ARGS__);
#define GLX_ERROR(...)          ::Galaxy::Log::GetClientLogger()->error(__VA_ARGS__);
#define GLX_FATAL(...)          ::Galaxy::Log::GetClientLogger()->fatal(__VA_ARGS__);

