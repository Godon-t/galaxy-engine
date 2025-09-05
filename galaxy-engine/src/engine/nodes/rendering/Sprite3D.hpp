#pragma once

#include "nodes/Node3D.hpp"
#include "project/UUID.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class Sprite3D : Node3D {
public:
    Sprite3D(std::string name = "Sprite3D")
        : Node3D(name)
        , m_initialized(false)
    {
    }
    ~Sprite3D() override;

    virtual void draw() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void loadTexture(std::string path);

    inline uuid getImageResourceID() const { return m_imageID; }

protected:
    void enteredRoot() override;

private:
    void clear();
    renderID generateRect(vec2 dimmensions);
    renderID m_textureID;
    renderID m_rectID;

    bool m_initialized;

    uuid m_imageID;
};
} // namespace Galaxy
