#pragma once

#include "Resource.hpp"
#include "pch.hpp"

#include <type_traits>

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

    template <typename OtherType,
        std::enable_if_t<std::is_convertible_v<OtherType*, ResourceType*>, int> = 0>
    ResourceHandle(const ResourceHandle<OtherType>& other)
        : ResourceHandleBase(other.m_resource)
    {
    }

    ~ResourceHandle() = default;

    explicit operator bool() const { return m_resource && getState() == LOADED; }

    ResourceType& getResource() const { return static_cast<ResourceType&>(*m_resource.get()); }
    ResourceType& getResource() { return static_cast<ResourceType&>(*m_resource.get()); }

private:
    template <typename> friend class ResourceHandle;
};

} // namespace Galaxy
