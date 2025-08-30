#pragma once

#include "ResourceMaker.hpp"
#include "Texture.hpp"
#include "engine/core/ThreadPool.hpp"

#include <queue>

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
        cache[path]                            = resource;

        m_threadPool.enqueue([resource, maker = makerIt->second.get(), path, this] {
            if (maker->loadResource(resource, path)) {
                std::unique_lock<std::mutex> lock(m_pendingLoadMutex);
                m_loadedResources.push(resource);
            } else {
                GLX_CORE_ERROR("Failed to load resource '{0}'", path);
                resource->m_state = ResourceState::FAILED;
            }
        });

        return ResourceHandle<ResourceType>(std::static_pointer_cast<ResourceType>(resource));
    }

    void updatePendingLoads()
    {
        std::unique_lock<std::mutex> lock(m_pendingLoadMutex);
        while (!m_loadedResources.empty()) {
            auto& resource = m_loadedResources.front();
            resource->onLoadFinish();
            resource->m_state = ResourceState::LOADED;
            m_loadedResources.pop();
        }
    }

private:
    ResourceManager()
        : m_threadPool(resourcePoolSize)
    {
        registerMaker<Texture>();
    }
    ThreadPool m_threadPool;
    std::mutex m_pendingLoadMutex;

    std::unordered_map<size_t, std::unique_ptr<IResourceMaker>> makers;
    std::unordered_map<std::string, std::shared_ptr<ResourceBase>> cache;
    std::queue<std::shared_ptr<ResourceBase>> m_loadedResources;
};

} // namespace Galaxy
