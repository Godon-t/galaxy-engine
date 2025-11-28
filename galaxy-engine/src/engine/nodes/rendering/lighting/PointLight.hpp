#pragma once

#include "Light.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {

class PointLight : public Light {
protected:
    virtual void enteredRoot() override;

public:
    PointLight(std::string name = "PointLight");
    ~PointLight() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    virtual void draw() override;

    void updateLight();

private:
    lightID m_lightID;
    renderID m_visualCubeID;

    // bool m_castShadows;
};
}
