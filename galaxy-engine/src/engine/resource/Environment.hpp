#pragma once

#include "Image.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"

#include <array>

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
        m_skyboxCubemapID = std::move(other.m_skyboxCubemapID);
    }

    bool load(YAML::Node& data) override;
    bool save(bool recursive = true) override
    {
        return ResourceSerializer::serialize(*this);
    }

private:
    friend class ResourceSerializer;

    std::array<ResourceHandle<Image>, 6> m_skybox;
    renderID m_skyboxCubemapID; // TODO: Bad design, you shouldn't have to to store renderID inside resource I think
};
} // namespace Galaxy
