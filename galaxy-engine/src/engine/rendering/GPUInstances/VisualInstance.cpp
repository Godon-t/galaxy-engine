#include "VisualInstance.hpp"
#include "pch.hpp"

#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

#include <utility>

using namespace Galaxy::geometry;

namespace Galaxy {
CullMode VisualInstance::s_cullMode = FRONT_CULLING;

Sphere& VisualInstance::getBoundingVolume()
{
    return m_boundingVolume;
}

VisualInstance::VisualInstance()
{
}

VisualInstance::~VisualInstance()
{
    destroy();
}

void VisualInstance::destroy()
{
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
    }
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
    }

    m_VAO         = 0;
    m_VBO         = 0;
    m_EBO         = 0;
    m_nbOfIndices = 0;
}

VisualInstance::VisualInstance(VisualInstance&& other) noexcept
    : m_nbOfIndices(std::exchange(other.m_nbOfIndices, 0))
    , m_VAO(std::exchange(other.m_VAO, 0))
    , m_VBO(std::exchange(other.m_VBO, 0))
    , m_EBO(std::exchange(other.m_EBO, 0))
    , m_cullMode(other.m_cullMode)
    , m_boundingVolume(other.m_boundingVolume)
{
}

VisualInstance& VisualInstance::operator=(VisualInstance&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_VAO            = std::exchange(other.m_VAO, 0);
    m_VBO            = std::exchange(other.m_VBO, 0);
    m_EBO            = std::exchange(other.m_EBO, 0);
    m_nbOfIndices    = std::exchange(other.m_nbOfIndices, 0);
    m_cullMode       = other.m_cullMode;
    m_boundingVolume = other.m_boundingVolume;

    return *this;
}
void VisualInstance::init(const std::vector<Vertex>& vertices, const std::vector<short unsigned int>& indices)
{
    destroy();
    m_nbOfIndices = indices.size();

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

    int vertexSize = sizeof(Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(5 * sizeof(float)));

    glBindVertexArray(0);

    checkOpenGLErrors("Visual instance init");



    vec3 min{99999 , 99999, 99999};
    vec3 max{-99999,-99999,-99999};
    for(auto& vertex : vertices){
        for(int i=0; i<3; i++){
            if(vertex.position[i] < min[i]) min[i] = vertex.position[i];
            if(vertex.position[i] > max[i]) max[i] = vertex.position[i];
        }
    }
    m_boundingVolume.center = (min + max) * 0.5f;
    m_boundingVolume.radius = length(min - max) * 0.5f;
}

void VisualInstance::draw()
{
    glBindVertexArray(m_VAO);

    if (m_cullMode != s_cullMode) {
        if (m_cullMode == FRONT_CULLING) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if (m_cullMode == BACK_CULLING) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } else {
            glDisable(GL_CULL_FACE);
        }
        s_cullMode = m_cullMode;
    }

    glDrawElements(
        GL_TRIANGLES, // mode
        m_nbOfIndices,
        GL_UNSIGNED_SHORT, // type
        (void*)0 // element array buffer offset
    );

    glBindVertexArray(0);

    checkOpenGLErrors("Visual instance draw");
}
}
