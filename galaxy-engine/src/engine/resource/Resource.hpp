#pragma once

#include "ResourceDeserializer.hpp"
#include "ResourceSerializer.hpp"
#include "project/UUID.hpp"

#include <atomic>
#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
class ResourceManager;

enum ResourceState {
    EMPTY,
    LOADING,
    LOADED,
    FAILED
};

class ResourceBase {
public:
    ResourceBase()          = default;
    virtual ~ResourceBase() = default;

    // Called in a separate thread
    virtual bool load(YAML::Node& data)                                                                   = 0;
    virtual bool initGres(const std::string& path, uuid resourceID, const std::string& externalPath = "") = 0;
    virtual bool save()                                                                                   = 0;
    // virtual bool reload()                     = 0;

    void onLoaded(std::function<void()> callback)
    {
        if (getState() == LOADED) {
            callback();
        } else {
            m_loadedCallbacks.push_back(std::move(callback));
        }
    }

    inline ResourceState getState() { return m_state; }
    inline uuid getResourceID() { return m_resourceID; }
    inline bool isInternal() { return m_isInternal; }

protected:
    std::string m_resourcePath;
    uuid m_resourceID;
    // TODO: Unclear
    bool m_isInternal = false;

private:
    void notifyLoaded()
    {
        for (auto& cb : m_loadedCallbacks)
            cb();
        m_loadedCallbacks.clear();
    }

    std::atomic<ResourceState> m_state { ResourceState::EMPTY };
    std::vector<std::function<void()>> m_loadedCallbacks;

    friend class ResourceManager;
    friend class ResourceSerializer;
    friend class ResourceDeserializer;
};

}
