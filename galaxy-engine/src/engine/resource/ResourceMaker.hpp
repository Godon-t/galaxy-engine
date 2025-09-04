#pragma once

#include "ResourceHandle.hpp"

namespace Galaxy {
class IResourceMaker {
public:
    virtual bool loadResource(std::shared_ptr<ResourceBase> resource, const std::string& path) const                      = 0;
    virtual bool loadResource(std::shared_ptr<ResourceBase> resource, const unsigned char* data, const size_t size) const = 0;
    virtual std::shared_ptr<ResourceBase> createResourcePtr() const                                                       = 0;
};

template <typename ResourceType>
class ResourceMaker : public IResourceMaker {
    bool loadResource(std::shared_ptr<ResourceBase> resource, const std::string& path) const override
    {
        return resource->load(path);
    }
    bool loadResource(std::shared_ptr<ResourceBase> resource, const unsigned char* data, const size_t size) const override
    {
        return resource->load(data, size);
    }

    std::shared_ptr<ResourceBase> createResourcePtr() const override
    {
        return std::make_shared<ResourceType>();
    }
};

} // namespace Galaxy
