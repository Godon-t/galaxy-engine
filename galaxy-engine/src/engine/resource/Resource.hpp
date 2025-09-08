#pragma once

#include "ResourceDeserializer.hpp"
#include "ResourceSerializer.hpp"
#include "project/UUID.hpp"

#include <atomic>
#include <filesystem>

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
    bool load(const std::string& file)
    {
        std::filesystem::path filePath(file.c_str());

        std::string fileExtension = filePath.extension().string();
        if (fileExtension == std::string(".gres"))
            return loadGres(file);
        else {
            return import(file);
        }
    }
    virtual bool load(const unsigned char* data, size_t size) = 0;
    virtual bool save()                                       = 0;
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

    virtual bool loadExtern(const std::string& path) = 0;
    virtual bool loadGres(const std::string& file)   = 0;
    virtual bool import(const std::string& file)     = 0;

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
