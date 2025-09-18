#pragma once

#include "types/Math.hpp"
#include "types/Render.hpp"

namespace Galaxy {
enum RenderCommandType {
    setActiveProgram,
    setView,
    setProjection,
    clear,
    depthMask,
    draw,
    bindTexture,
    bindCubemap,
    bindMaterial
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

struct DepthMaskCommand {
    bool state;
};

struct BindTextureCommand {
    renderID instanceID;
    char* uniformName;
};

struct BindCubemapCommand {
    renderID instanceID;
    char* uniformName;
};

struct BindMaterialCommand {
    renderID materialRenderID;
};

struct SetActiveProgramCommand {
    ProgramType program;
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
        DepthMaskCommand depthMask;
        DrawCommand draw;
        BindTextureCommand bindTexture;
        BindCubemapCommand bindCubemap;
        BindMaterialCommand bindMaterial;
    };
};
} // namespace Galaxy
