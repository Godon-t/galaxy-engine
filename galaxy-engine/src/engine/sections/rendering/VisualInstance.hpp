#pragma once

#include "engine/types/Math.hpp"

#include <GL/glew.h>

using namespace math;

struct Vertex {
    vec3 position;
    vec2 texCoord;
};

class VisualInstance {
    int nbOfIndices = 0;
    GLuint VAO, VBO, EBO;

public:
    VisualInstance();
    ~VisualInstance();

    VisualInstance(const VisualInstance&) = delete;
    VisualInstance& operator=(const VisualInstance&) = delete;

    VisualInstance(VisualInstance&&);
    VisualInstance& operator=(VisualInstance&&);


    void init(std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices);
    void draw();
    
};
