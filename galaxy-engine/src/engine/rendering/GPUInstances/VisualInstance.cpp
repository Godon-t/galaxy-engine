#include "VisualInstance.hpp"
#include "pch.hpp"

#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
CullMode VisualInstance::s_cullMode = FRONT_CULLING;

Sphere& VisualInstance::getBoundingVolume()
{
    return m_boundingVolume;
}

VisualInstance::VisualInstance()
    : m_VAO(0)
    , m_VBO(0)
    , m_EBO(0)
    , m_cullMode(FRONT_CULLING)
{
}

VisualInstance::~VisualInstance()
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
}

VisualInstance::VisualInstance(VisualInstance&& other)
{
    m_VAO            = other.m_VAO;
    m_VBO            = other.m_VBO;
    m_EBO            = other.m_EBO;
    m_nbOfIndices    = other.m_nbOfIndices;
    m_boundingVolume = other.m_boundingVolume;

    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
}

VisualInstance& VisualInstance::operator=(VisualInstance&& other)
{
    m_VAO            = other.m_VAO;
    m_VBO            = other.m_VBO;
    m_EBO            = other.m_EBO;
    m_nbOfIndices    = other.m_nbOfIndices;
    m_boundingVolume = other.m_boundingVolume;

    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;

    return *this;
}
void VisualInstance::init(const std::vector<Vertex>& vertices, const std::vector<short unsigned int>& indices)
{
    m_nbOfIndices = indices.size();

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

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
