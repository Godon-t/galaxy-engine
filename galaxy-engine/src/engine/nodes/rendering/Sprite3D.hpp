#pragma once

#include "nodes/Node3D.hpp"
#include "project/UUID.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "types/Render.hpp"
#include "resource/Material.hpp"
#include "common/BitMask.hpp"

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
    void loadTexture(ResourceHandle<Image> image);

    inline uuid getImageResourceID() const { return m_imageID; }
    ResourceHandle<TextureMaterial> getMaterial() const { return m_materialResource; }
    MaterialHandle getMaterialId() const { return m_materialHandle; }
    BitMask getVisibilityLayer() const { return m_visibilityLayer; }
    void setVisibilityLayer(const BitMask& layer) { m_visibilityLayer = layer; }

protected:
    void enteredRoot() override;

private:
    ResourceHandle<TextureMaterial> m_materialResource;
    MaterialHandle m_materialHandle;
    GeometryHandle m_rect;
    BitMask m_visibilityLayer;

    bool m_initialized;

    uuid m_imageID;
};
} // namespace Galaxy
