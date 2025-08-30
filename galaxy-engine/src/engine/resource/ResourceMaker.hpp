#pragma once

#include "ResourceHandle.hpp"

namespace Galaxy {
class IResourceMaker {
public:
    virtual bool loadResource(std::shared_ptr<ResourceBase>& resource, const std::string& path) const = 0;
    virtual std::shared_ptr<ResourceBase> createResourcePtr() const                                   = 0;
};

template <typename ResourceType>
class ResourceMaker : public IResourceMaker {
    bool loadResource(std::shared_ptr<ResourceBase>& resource, const std::string& path) const override
    {
        return resource->load(path);
    }
    std::shared_ptr<ResourceBase> createResourcePtr() const override
    {
        return std::make_shared<ResourceType>();
    }
};

} // namespace Galaxy
