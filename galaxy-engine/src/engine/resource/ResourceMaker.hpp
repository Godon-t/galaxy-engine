#pragma once

#include "ResourceHandle.hpp"
#include "project/Project.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
class IResourceMaker {
public:
    virtual bool loadResource(std::shared_ptr<ResourceBase> resource, const std::string& path) const = 0;
    virtual std::shared_ptr<ResourceBase> createResourcePtr() const                                  = 0;
};

template <typename ResourceType>
class ResourceMaker : public IResourceMaker {
    bool loadResource(std::shared_ptr<ResourceBase> resource, const std::string& path) const override
    {
        YAML::Node data;

        std::ifstream stream(Project::getProjectRootPath() + path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        data = YAML::Load(strStream.str());

        return resource->load(data);
    }

    std::shared_ptr<ResourceBase> createResourcePtr() const override
    {
        return std::make_shared<ResourceType>();
    }
};

} // namespace Galaxy
