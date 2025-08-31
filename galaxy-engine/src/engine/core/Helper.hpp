#pragma once

#include "pch.hpp"

#ifndef RESOURCE_ROOT
#define RESOURCE_ROOT "./"
#endif

namespace Galaxy {
inline std::string engineRes(const std::string& relativePath)
{
    return std::string(RESOURCE_ROOT) + relativePath;
}
} // namespace galaxy