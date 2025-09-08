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
    virtual bool import(std::shared_ptr<ResourceBase> resource, const std::string& file) const       = 0;
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
    bool import(std::shared_ptr<ResourceBase> resource, const std::string& file) const override
    {
        std::string path, extension;
        Project::extractExtension(file, path, extension);
        path += std::string(".gres");
        uuid resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, path);

        resource->initGres(path, resourceID);
        resource->save();
    }

    std::shared_ptr<ResourceBase> createResourcePtr() const override
    {
        return std::make_shared<ResourceType>();
    }
};

} // namespace Galaxy
