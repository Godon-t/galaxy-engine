#pragma once

#include "pch.hpp"

#include "UUID.hpp"
#include "engine/nodes/Node.hpp"

namespace Galaxy {
class Scene {
public:
    std::shared_ptr<Node> getNodePtr() { return m_rootNode; }
    void setNodePtr(std::shared_ptr<Node> node) { m_rootNode = node; }

    inline void setUuid(uuid id) { m_uuid = id; }
    inline uuid getUuid() { return m_uuid; }

private:
    uuid m_uuid;
    std::shared_ptr<Node> m_rootNode;
};
}