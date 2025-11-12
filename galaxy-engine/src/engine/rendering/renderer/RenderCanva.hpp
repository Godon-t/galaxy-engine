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
    renderID targetTextureID;
    FramebufferTextureFormat format;
    std::unordered_map<renderID, std::vector<RenderCommand>> materialToSubmitCommand;
    std::vector<RenderCommand> commands;
    int cubemapIdx;

    RenderCanva(const mat4& view, const mat4& projection, renderID framebuffer, FramebufferTextureFormat framebufferFormat)
        : viewMat(view)
        , projectionMat(projection)
        , framebufferID(framebuffer)
        , format(framebufferFormat)
        , cubemapIdx(-1)
        , targetTextureID(0)
    {
    }

    RenderCanva(const mat4& view, const mat4& projection, renderID framebuffer, FramebufferTextureFormat framebufferFormat, int cubemapIndex)
        : viewMat(view)
        , projectionMat(projection)
        , framebufferID(framebuffer)
        , format(framebufferFormat)
        , cubemapIdx(cubemapIndex)
        , targetTextureID(0)
    {
    }
    ~RenderCanva() = default;
};
}