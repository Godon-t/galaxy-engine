#pragma once

#include "data/Camera.hpp"
#include "types/Render.hpp"

#include "nodes/Node3D.hpp"

namespace Galaxy {
class CameraNode : public Node3D {
private:
    bool m_current;
    std::shared_ptr<Camera> m_cameraData;

protected:
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override {};

public:
    camID id;
    CameraNode(std::string name = "CameraNode");
    ~CameraNode() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    virtual void updateTransformAndChilds(const mat4& matrix) override;
    virtual void forceUpdateTransformAndChilds(const mat4& matrix) override;

    void setCurrent(bool state);
    bool getCurrent();

    inline virtual void draw() override { Node3D::draw(); }

    vec3 getForward() const;
    const std::shared_ptr<Camera> getCamera();

    void handleInput(const Event& event) override;
};
}
