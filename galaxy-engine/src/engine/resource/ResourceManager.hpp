#pragma once

#include "ResourceMaker.hpp"
#include "Texture.hpp"

namespace Galaxy {
class ResourceManager {
public:
    static ResourceManager& getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    template <typename ResourceType>
    void registerMaker()
    {
        makers[typeid(ResourceType).hash_code()] = std::make_unique<ResourceMaker<ResourceType>>();
    }

    template <typename ResourceType>
    ResourceHandle<ResourceType> load(const std::string& path)
    {
        auto it = cache.find(path);
        if (it != cache.end()) {
            return ResourceHandle<ResourceType>(std::static_pointer_cast<ResourceType>(it->second));
        }

        auto makerIt = makers.find(typeid(ResourceType).hash_code());
        GLX_CORE_ASSERT(makerIt != makers.end(), "No resource maker for file: '{0}'", path);

        std::shared_ptr<ResourceBase> resource = makerIt->second->createResourcePtr();
        resource->m_state                      = ResourceState::LOADING;
        if (makerIt->second->loadResource(resource, path)) {
            resource->m_state = ResourceState::LOADED;
        } else {
            resource->m_state = ResourceState::FAILED;
        }

        cache[path] = resource;

        return ResourceHandle<ResourceType>(std::static_pointer_cast<ResourceType>(resource));
    }

private:
    ResourceManager()
    {
        registerMaker<Texture>();
    }
    std::unordered_map<size_t, std::unique_ptr<IResourceMaker>> makers;
    std::unordered_map<std::string, std::shared_ptr<ResourceBase>> cache;
};

} // namespace Galaxy
