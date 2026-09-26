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
using RenderTargetHandle = std::variant<FramebufferHandle, CubemapFramebufferHandle>;

enum SetValueTypes {
    BOOL,
    FLOAT,
    INT,
    VEC2,
    VEC3,
    IVEC3,
    MAT4
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


struct UpdateTextureCommand {
    TextureHandle texture;
    TextureFormat newFormat = TextureFormat::NONE;
    unsigned int width;
    unsigned int height;
};

struct SaveFrameBufferCommand {
    std::string path;
    FramebufferHandle framebuffer;
};

using RenderCommand = std::variant<
    SetUniformCommand,
    UpdateTextureCommand,
    UpdateUBOCommand,
    BindUBOCommand,
    SaveFrameBufferCommand>;

} // namespace Galaxy
