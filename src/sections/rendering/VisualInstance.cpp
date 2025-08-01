#include <src/sections/rendering/VisualInstance.hpp>

#include <src/sections/rendering/OpenglHelper.hpp>
#include "VisualInstance.hpp"


VisualInstance::VisualInstance(): VAO(0), VBO(0), EBO(0) {}

VisualInstance::~VisualInstance(){
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
    }
}

VisualInstance::VisualInstance(VisualInstance &&other)
{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    nbOfIndices = other.nbOfIndices;

    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
}

VisualInstance &VisualInstance::operator=(VisualInstance &&other)
{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    nbOfIndices = other.nbOfIndices;

    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;

    return *this;
}
void VisualInstance::init(std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices)
{
    nbOfIndices = indices.size();

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    
    int vertexSize = sizeof(Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    checkOpenGLErrors("Visual instance init");
}

void VisualInstance::draw()
{
    glBindVertexArray(VAO);
    
    glDrawElements(
                GL_TRIANGLES,      // mode
                nbOfIndices,
                GL_UNSIGNED_SHORT,   // type
                (void*)0           // element array buffer offset
                );

    glBindVertexArray(0);

    checkOpenGLErrors("Visual instance draw");
}
