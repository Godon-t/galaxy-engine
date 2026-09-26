#pragma once

#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Math.hpp"
#include "types/Render.hpp"

#include <cstring>
#include <string>
#include <variant>
#include <vector>

namespace Galaxy {

// These are transient messages consumed by the rendering backend. GPU resource
// ownership lives in GpuResourceRegistry and is referenced only through handles.

struct SetViewCommand {
    math::mat4 view;
};

struct SetProjectionCommand {
    math::mat4 projection;
};

struct RawDrawCommand {
    GeometryHandle geometry;
};

struct DrawCommand {
    GeometryHandle geometry;
    math::mat4 model;
};

struct ClearCommand {
    math::vec4 color;
};

struct DepthMaskCommand {
    bool state;
};

struct UseTextureCommand {
    TextureHandle texture;
    std::string uniformName;
    bool important = false;
};

struct UseCubemapCommand {
    CubemapHandle cubemap;
    std::string uniformName;
};

struct AttachTextureToFramebufferCommand {
    TextureHandle texture;
    FramebufferHandle framebuffer;
    int attachmentIdx;
};

struct AttachCubemapToFramebufferCommand {
    CubemapHandle cubemap;
    CubemapFramebufferHandle framebuffer;
    int colorIdx;
};

struct BindMaterialCommand {
    MaterialHandle material;
};


using RenderTargetHandle = std::variant<FramebufferHandle, CubemapFramebufferHandle>;

struct BindFrameBufferCommand {
    RenderTargetHandle target;
    int depthLayerIdx = 0;
    int cubemapFaceIdx = -1;
    bool bind;
};

enum SetValueTypes {
    BOOL,
    FLOAT,
    INT,
    VEC2,
    VEC3,
    IVEC3,
    MAT4
};

struct SetFramebufferAsTextureUniformCommand {
    RenderTargetHandle framebuffer;
    std::string uniformName;
    int textureIdx;
};

struct UpdateUBOCommand {
    BufferHandle ubo;
    std::vector<std::byte> data;

    template<typename T>
    static UpdateUBOCommand make(BufferHandle handle, const T& payload)
    {
        UpdateUBOCommand command;
        command.ubo = handle;
        command.data.resize(sizeof(T));
        std::memcpy(command.data.data(), &payload, sizeof(T));
        return command;
    }
};

struct BindUBOCommand {
    BufferHandle ubo;
    unsigned int idx;
};

struct SetUniformCommand {
    SetValueTypes type;
    std::string uniformName;
    // TODO: replace with std::variant
    union {
        bool valueBool;
        struct {
            float x, y, z;
        } valueVec3;
        struct {
            int x, y, z;
        } valueIVec3;
        struct {
            float x, y;
        } valueVec2;
        float valueFloat;
        int valueInt;
    };
    math::mat4 matrixValue;
};

struct SetViewportCommand {
    math::vec2 position;
    math::vec2 size;
};

struct UpdateTextureCommand {
    TextureHandle texture;
    TextureFormat newFormat = TextureFormat::NONE;
    unsigned int width;
    unsigned int height;
};

struct UpdateCubemapCommand {
    CubemapHandle cubemap;
    unsigned int resolution;
};

struct DebugMsgCommand {
    std::string msg;
};

struct DrawDebugLineCommand {
    math::vec3 start;
    math::vec3 end;
};

struct SaveFrameBufferCommand {
    std::string path;
    FramebufferHandle framebuffer;
};

using RenderCommand = std::variant<
    SetViewCommand,
    SetProjectionCommand,
    ClearCommand,
    DepthMaskCommand,
    DrawCommand,
    RawDrawCommand,
    UseTextureCommand,
    UseCubemapCommand,
    AttachTextureToFramebufferCommand,
    AttachCubemapToFramebufferCommand,
    BindMaterialCommand,
    BindFrameBufferCommand,
    SetUniformCommand,
    SetViewportCommand,
    UpdateTextureCommand,
    UpdateCubemapCommand,
    SetFramebufferAsTextureUniformCommand,
    UpdateUBOCommand,
    BindUBOCommand,
    DebugMsgCommand,
    DrawDebugLineCommand,
    SaveFrameBufferCommand>;

} // namespace Galaxy
