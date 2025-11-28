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
    rawDraw,
    useTexture,
    useCubemap,
    attachTextureToFramebuffer,
    attachCubemapToFramebuffer,
    bindMaterial,
    bindFrameBuffer,
    unbindFrameBuffer,
    initPostProcess,
    setUniform,
    setViewport,
    updateCubemap,
    saveFrameBuffer,

    debugMsg
};

struct SetViewCommand {
    math::mat4 view;
    // TODO: Shader ?
};

struct SetProjectionCommand {
    math::mat4 projection;
    // TODO: Shader ?
};

struct RawDrawCommand {
    renderID instanceID;
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

struct UseTextureCommand {
    renderID instanceID;
    char* uniformName;
};

struct UseCubemapCommand {
    renderID instanceID;
    char* uniformName;
};

struct AttachTextureToFramebufferCommand {
    renderID textureID;
    renderID framebufferID;
    int attachmentIdx;
};

struct AttachCubemapToFramebufferCommand {
    renderID cubemapID;
    renderID framebufferID;
    bool depth;
};

struct BindMaterialCommand {
    renderID materialRenderID;
};

struct SetActiveProgramCommand {
    ProgramType program;
};

struct BindFrameBufferCommand {
    renderID frameBufferID;
    int cubemapFaceIdx = -1;
};

struct InitPostProcessCommand {
    renderID frameBufferID;
};

enum SetValueTypes {
    BOOL,
    FLOAT,
    INT,
    VEC2,
    VEC3,
    MAT4
};
struct SetUniformCommand {
    SetValueTypes type;
    char* uniformName;
    union {
        bool valueBool;
        struct {
            float x, y, z;
        } valueVec3;
        struct {
            float x, y;
        } valueVec2;
        float valueFloat;
        int valueInt;
    };
    // TODO: fix this !!!
    math::mat4 matrixValue;
};

struct SetViewportCommand {
    math::vec2 position;
    math::vec2 size;
};

struct UpdateCubemapCommand {
    renderID targetID;
    unsigned int resolution;
};

struct DebugMsgCommand {
    char* msg;
};

struct SaveFrameBufferCommand {
    char* path;
    renderID frameBufferID;
};

struct RenderCommand {
    ~RenderCommand() = default;

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
        RawDrawCommand rawDraw;
        UseTextureCommand useTexture;
        UseCubemapCommand useCubemap;
        AttachTextureToFramebufferCommand attachTextureToFramebuffer;
        AttachCubemapToFramebufferCommand attachCubemapToFramebuffer;
        BindMaterialCommand bindMaterial;
        BindFrameBufferCommand bindFrameBuffer;
        InitPostProcessCommand initPostProcess;
        SetUniformCommand setUniform;
        SetViewportCommand setViewport;
        UpdateCubemapCommand updateCubemap;

        DebugMsgCommand debugMsg;
        SaveFrameBufferCommand saveFrameBuffer;
    };
};
} // namespace Galaxy
