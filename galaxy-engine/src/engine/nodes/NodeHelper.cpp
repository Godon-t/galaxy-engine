#include "NodeHelper.hpp"

#include "Node3D.hpp"
#include "rendering/Camera.hpp"
#include "rendering/EnvironmentNode.hpp"
#include "rendering/MeshInstance.hpp"
#include "rendering/MultiMeshInstance.hpp"
#include "rendering/Sprite3D.hpp"
#include "rendering/lighting/SpotLight.hpp"

namespace Galaxy {
std::unique_ptr<Node> constructNode(std::string& type)
{
    if (type == "Node") {
        return std::make_unique<Node>();
    } else if (type == "Node3D") {
        return std::make_unique<Node3D>();
    } else if (type == "MeshInstance") {
        return std::make_unique<MeshInstance>();
    } else if (type == "MultiMeshInstance") {
        return std::make_unique<MultiMeshInstance>();
    } else if (type == "Camera") {
        return std::make_unique<Camera>();
    } else if (type == "Sprite3D") {
        return std::make_unique<Sprite3D>();
    } else if (type == "EnvironmentNode") {
        return std::make_unique<EnvironmentNode>();
    } else if (type == "SpotLight") {
        return std::make_unique<SpotLight>();
    } else {
        GLX_CORE_ERROR("Unknown type '{0}'", type);
        return nullptr;
    }
}
}
