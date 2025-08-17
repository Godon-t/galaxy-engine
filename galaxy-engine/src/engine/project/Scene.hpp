#pragma once

#include "pch.hpp"

#include "Project.hpp"
#include "engine/nodes/Node.hpp"
#include "engine/types/uuid.hpp"

namespace Galaxy {
class Scene {
public:
    Scene(std::string path = "")
        : m_scenePath(path)
    {
    }
    bool load(std::shared_ptr<Node> node);
    bool save();
    bool saveAs(const char* path, uuid id);
    bool isValid();
    std::shared_ptr<Node> getNodePtr() { return m_rootNode; }

    inline void setUuid(uuid id) { m_uuid = id; }
    inline uuid getUuid() { return m_uuid; }

private:
    uuid m_uuid;
    std::string m_scenePath;
    std::shared_ptr<Node> m_rootNode;
};
}