#pragma once

#include "ResourceHandle.hpp"
#include "project/Project.hpp"

#include <fstream>
#include <string>
#include <string_view>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
class IResourceMaker {
public:
    static std::string readType(std::string_view path)
    {
        const std::string resPath{path};
        std::ifstream stream(resPath);
        std::string line;
        while (std::getline(stream, line)) {
            auto first = line.find_first_not_of(" \t\r");
            if (first == std::string::npos || line[first] == '#')
                continue;

            // The first non-comment entry is the resource type.
            return YAML::Load(line)["Type"].as<std::string>();
        }

        return {};
    }

    virtual std::string_view getType() const = 0;

    virtual bool loadResource(std::shared_ptr<ResourceBase> resource, const std::string& path) const = 0;
    virtual std::shared_ptr<ResourceBase> createResourcePtr() const                                  = 0;
};

template <typename ResourceType>
class ResourceMaker : public IResourceMaker {
    std::string type;

public:
    ResourceMaker(std::string type): type(type) {}

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

    std::string_view getType() const override { return type; }
};

} // namespace Galaxy
