#pragma once

#include <atomic>

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
    virtual bool load(const std::string& file) = 0;
    // virtual bool reload()                      = 0;

    void onLoaded(std::function<void()> callback)
    {
        if (getState() == LOADED) {
            callback();
        } else {
            m_loadedCallbacks.push_back(std::move(callback));
        }
    }

    ResourceState getState() { return m_state; }

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
};

}
