#pragma once

#include "types/Math.hpp"
#include "types/Render.hpp"

namespace Galaxy {
enum RenderCommandType {
    setView,
    setProjection,
    draw,
    clear
};

struct SetViewCommand {
    math::mat4 view;
    // TODO: Shader ?
};

struct SetProjectionCommand {
    math::mat4 projection;
    // TODO: Shader ?
};

struct DrawCommand {
    renderID instanceId;
    math::mat4 model;
};

struct ClearCommand {
    math::vec4 color;
};

struct RenderCommand {
    RenderCommand()
        : type(RenderCommandType::draw)
    {
        new (&draw) DrawCommand();
    }

    RenderCommandType type;
    union {
        SetViewCommand setView;
        SetProjectionCommand setProjection;
        DrawCommand draw;
        ClearCommand clear;
    };
};
} // namespace Galaxy
