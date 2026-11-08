#pragma once

#include "CubemapResource.hpp"
#include "Image.hpp"
#include "ResourceHandle.hpp"

#include <variant>

namespace Galaxy {

using EnvironmentMapSource = std::variant<ResourceHandle<Image>, ResourceHandle<CubemapResource>>;

inline ResourceHandle<ResourceBase> getEnvironmentMapResource(const EnvironmentMapSource& source)
{
    return std::visit([](const auto& resource) -> ResourceHandle<ResourceBase> { return resource; }, source);
}

} // namespace Galaxy
