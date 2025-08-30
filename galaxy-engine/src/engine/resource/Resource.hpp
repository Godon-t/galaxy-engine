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

    // Called inside main thread
    virtual void onLoadFinish() = 0;
    // Called in a separate thread
    virtual bool load(const std::string& file) = 0;
    // virtual bool reload()                      = 0;

    ResourceState getState() { return m_state; }

private:
    std::atomic<ResourceState> m_state { ResourceState::EMPTY };
    friend class ResourceManager;
};

}
