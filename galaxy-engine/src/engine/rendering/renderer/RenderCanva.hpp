#pragma once

#include "RenderCommand.hpp"

#include "types/Math.hpp"
#include "types/Render.hpp"

using namespace math;

namespace Galaxy {
struct RenderCanva {
    mat4 viewMat;
    mat4 projectionMat;
    renderID framebufferID;
    FramebufferTextureFormat format;
    
    renderID colorTargetID;
    renderID depthTargetID;
    int cubemapIdx;
    bool useBuffer;
    bool storeResult;
    std::string storagePath;
    bool clearBuffer;

    RenderCanva(const mat4& view, const mat4& projection, renderID framebuffer, FramebufferTextureFormat framebufferFormat)
        : viewMat(view)
        , projectionMat(projection)
        , framebufferID(framebuffer)
        , format(framebufferFormat)
        , cubemapIdx(-1)
        , useBuffer(true)
        , colorTargetID(0)
        , depthTargetID(0)
        , storeResult(false)
        , clearBuffer(true)
    {
    }

    RenderCanva(const mat4& view, const mat4& projection, renderID framebuffer, FramebufferTextureFormat framebufferFormat, int cubemapIndex)
        : viewMat(view)
        , projectionMat(projection)
        , framebufferID(framebuffer)
        , format(framebufferFormat)
        , cubemapIdx(cubemapIndex)
        , useBuffer(true)
        , colorTargetID(0)
        , depthTargetID(0)
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