#include <src/nodes/MeshInstance.hpp>

#include <iostream>

#include <src/sections/rendering/VisualInstance.hpp>
#include <src/sections/rendering/Renderer.hpp>
#include "MeshInstance.hpp"

void MeshInstance::generateTriangle()
{
    std::vector<Vertex> triangleVertices;
    Vertex v1, v2, v3;
    v1.position = vec3(-1, -1, 0);
    v1.texCoord = vec2(0,0);
    v2.position = vec3(1, -1, 0);
    v2.texCoord = vec2(1,0);
    v3.position = vec3(1, 1, 0);
    v3.texCoord = vec2(0,1);
    
    triangleVertices.push_back(v1);
    triangleVertices.push_back(v2);
    triangleVertices.push_back(v3);
    
    std::vector<short unsigned int> triangleIndices;
    triangleIndices.push_back(0);
    triangleIndices.push_back(1);
    triangleIndices.push_back(2);

    renderId = Renderer::getInstance().instanciateMesh(getTransform(), triangleVertices, triangleIndices);
}

MeshInstance::~MeshInstance()
{
    Renderer::getInstance().clearMesh(renderId);
}

void MeshInstance::handleInputFromBot(const InputAction& inputAction)
{
    if(inputAction.action.name == "forward" && inputAction.action.clicked){
        std::cout << "Input working \n";
    }
}
