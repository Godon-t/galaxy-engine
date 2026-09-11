#pragma once

#include "commands/RenderCommand.hpp"

#include "types/Math.hpp"
#include "types/Render.hpp"

using namespace math;

namespace Galaxy {
struct RenderCanva {
    mat4 viewMat;
    mat4 projectionMat;
    RenderTargetHandle framebuffer;
    FramebufferTextureFormat format;
    
    TextureHandle colorTarget;
    TextureHandle depthTarget;
    int cubemapIdx;
    bool useBuffer;
    bool storeResult;
    std::string storagePath;
    bool clearBuffer;

    RenderCanva(const mat4& view, const mat4& projection, FramebufferHandle framebufferHandle, FramebufferTextureFormat framebufferFormat)
        : viewMat(view)
        , projectionMat(projection)
        , framebuffer(framebufferHandle)
        , format(framebufferFormat)
        , cubemapIdx(-1)
        , useBuffer(true)
        , storeResult(false)
        , clearBuffer(true)
    {
    }

    RenderCanva(const mat4& view, const mat4& projection, CubemapFramebufferHandle framebufferHandle, FramebufferTextureFormat framebufferFormat, int cubemapIndex)
        : viewMat(view)
        , projectionMat(projection)
        , framebuffer(framebufferHandle)
        , format(framebufferFormat)
        , cubemapIdx(cubemapIndex)
        , useBuffer(true)
        , storeResult(false)
        , clearBuffer(true)
    {
    }

    RenderCanva()
        : useBuffer(false)
    {
    }
    ~RenderCanva() = default;
};
}
