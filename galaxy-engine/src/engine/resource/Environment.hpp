#pragma once

#include "Image.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"

#include <array>
#include <utility>

namespace Galaxy {
class Environment : public ResourceBase {
public:
    Environment() = default;
    Environment(std::array<ResourceHandle<Image>, 6>& skybox)
        : m_skybox(skybox)
    {
    }
    Environment(Environment&& other) noexcept
    {
        m_isInternal      = std::move(other.m_isInternal);
        m_resourceID      = std::move(other.m_resourceID);
        m_resourcePath    = std::move(other.m_resourcePath);
        m_skybox          = std::move(other.m_skybox);
        m_skyboxCubemap = std::exchange(other.m_skyboxCubemap, {});
    }

    bool load(YAML::Node& data) override;
    bool save(bool recursive = true) override
    {
        return ResourceSerializer::serialize(*this);
    }

    inline std::array<ResourceHandle<Image>, 6> getSkybox() { return m_skybox; }

private:
    friend class ResourceSerializer;

    std::array<ResourceHandle<Image>, 6> m_skybox;
    CubemapHandle m_skyboxCubemap;
};
} // namespace Galaxy
