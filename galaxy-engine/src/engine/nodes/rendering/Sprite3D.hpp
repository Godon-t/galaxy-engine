#pragma once

#include "nodes/Node3D.hpp"
#include "project/UUID.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class Sprite3D : public Node3D {
public:
    Sprite3D(std::string name = "Sprite3D")
        : Node3D(name)
        , m_initialized(false)
        , m_imageID(0)
    {
    }
    ~Sprite3D() override;

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void loadTexture(std::string path);

    inline uuid getImageResourceID() const { return m_imageID; }

protected:
    void enteredRoot() override;

private:
    TextureHandle m_texture;
    GeometryHandle m_rect;

    bool m_initialized;

    uuid m_imageID;
};
} // namespace Galaxy
