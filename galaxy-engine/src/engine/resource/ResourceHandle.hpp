#pragma once

#include "Resource.hpp"
#include "pch.hpp"

namespace Galaxy {
class ResourceManager;

class ResourceHandleBase {
public:
    ResourceHandleBase() = default;
    ResourceHandleBase(std::shared_ptr<ResourceBase> resource)
        : m_resource(resource)
    {
    }
    ~ResourceHandleBase() = default;

    // bool isReady();
    // bool isLoaded();
    // bool isLoading();
    inline ResourceState getState() const { return m_resource->getState(); }

protected:
    std::shared_ptr<ResourceBase> m_resource;
    friend class ResourceManager;
};

template <typename ResourceType>
class ResourceHandle : public ResourceHandleBase {
public:
    ResourceHandle() = default;
    ResourceHandle(std::shared_ptr<ResourceType> resource)
        : ResourceHandleBase(std::static_pointer_cast<ResourceBase>(resource))
    {
    }

    ~ResourceHandle() = default;

    explicit operator bool() const { return m_resource && getState() == LOADED; }

    const ResourceType& getResource() const { return static_cast<ResourceType&>(*m_resource.get()); }
    ResourceType& getResource() { return static_cast<ResourceType&>(*m_resource.get()); }
};

} // namespace Galaxy
