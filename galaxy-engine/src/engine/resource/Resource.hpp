#pragma once

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

    virtual bool load(const std::string& file) = 0;
    // virtual bool reload()                      = 0;

    ResourceState getState() { return m_state; }

private:
    ResourceState m_state = ResourceState::EMPTY;
    friend class ResourceManager;
};

}
