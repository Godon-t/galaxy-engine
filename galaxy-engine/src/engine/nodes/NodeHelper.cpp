#include "NodeHelper.hpp"

#include "Node3D.hpp"
#include "rendering/CameraNode.hpp"
#include "rendering/EnvironmentNode.hpp"
#include "rendering/MeshInstance.hpp"
#include "rendering/Sprite3D.hpp"
#include "rendering/lighting/GINode.hpp"
#include "rendering/lighting/PointLight.hpp"
#include "rendering/lighting/SpotLight.hpp"

#define NODE_CASE(nodeType) \
    if (type == #nodeType)  \
        return std::make_unique<nodeType>();

namespace Galaxy {

std::unique_ptr<Node> constructNode(std::string& type)
{
    NODE_CASE(Node)
    NODE_CASE(Node3D)
    NODE_CASE(MeshInstance)
    NODE_CASE(CameraNode)
    NODE_CASE(Sprite3D)
    NODE_CASE(EnvironmentNode)
    NODE_CASE(SpotLight)
    NODE_CASE(PointLight)
    NODE_CASE(GINode)

    GLX_CORE_ERROR("Unknown type '{0}'", type);
    return nullptr;
}
}
