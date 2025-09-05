#pragma once

#include "types/Math.hpp"
#include "types/Render.hpp"

namespace Galaxy {
enum RenderCommandType {
    setActiveProgram,
    setView,
    setProjection,
    clear,
    draw,
    bindTexture
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

struct BindTextureCommand {
    renderID instanceID;
    char* uniformName;
};

enum BaseProgramEnum {
    PBR,
    TEXTURE
};
struct SetActiveProgramCommand {
    BaseProgramEnum program;
};

struct RenderCommand {
    RenderCommand()
        : type(RenderCommandType::draw)
    {
        new (&draw) DrawCommand();
    }

    RenderCommandType type;
    union {
        SetActiveProgramCommand setActiveProgram;
        SetViewCommand setView;
        SetProjectionCommand setProjection;
        ClearCommand clear;
        DrawCommand draw;
        BindTextureCommand bindTexture;
    };
};
} // namespace Galaxy
