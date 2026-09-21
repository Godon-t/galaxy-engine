#pragma once

#include "engine/rendering/renderer/resources/GpuResourceHandle.hpp"
#include "engine/types/Render.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Galaxy {

enum class DefaultRenderTexture {
    OpaqueColor
};

[[nodiscard]] inline constexpr std::string_view toString(DefaultRenderTexture texture) noexcept
{
    switch (texture) {
    case DefaultRenderTexture::OpaqueColor:
        return "opaque_color_texture";
    }

    return {};
}

struct GenericId {
    using Index = std::uint32_t;

    inline static constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

    Index index = InvalidIndex;

    [[nodiscard]] bool isValid() const noexcept { return index != InvalidIndex; }
    explicit operator bool() const noexcept { return isValid(); }

    friend bool operator==(GenericId lhs, GenericId rhs) noexcept
    {
        return lhs.index == rhs.index;
    }

    friend bool operator!=(GenericId lhs, GenericId rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

using GraphTextureId = GenericId;
using RenderPassId = GenericId;
using TargetId = GenericId;

struct GraphTextureDesc {
    std::string name;
    TextureFormat format = TextureFormat::RGBA;
    std::uint32_t arrayLayers = 0;

    TextureWrap wrapS = TextureWrap::REPEAT;
    TextureWrap wrapT = TextureWrap::REPEAT;

    // GLX-TODO: should describe state like wrap, filtering, etc ?
    bool imported = false;

    [[nodiscard]] bool hasDepth() const
    {
        return format == TextureFormat::DEPTH24STENCIL8 || format == TextureFormat::DEPTH;
    }
};

struct RenderTargetDesc {
    std::vector<GraphTextureId> colorAttachments;
    std::optional<GraphTextureId> depthAttachment;
};

enum class BlendMode {
    Disabled,
    Alpha
};

struct RenderState {
    bool depthTest = true;
    BlendMode blend = BlendMode::Disabled;
    bool clear = true;

    // GLX-TODO: handle it the right way, the material should have the info or something else
    bool supportPBR = false;
};

struct SampledTextureInputDesc {
    GraphTextureId texture;
    std::string samplerName;
};

struct RenderPassDesc {
    std::string name;
    ProgramHandle associatedProgram;
    RenderState state;

    std::vector<SampledTextureInputDesc> inputTextures;
    TargetId targetId;
};

// Declaration phase: describes resources, targets and passes without allocating GPU objects.
struct RenderGraphDeclaration {
    std::vector<GraphTextureDesc> textures;
    std::vector<RenderPassDesc> passes;
    std::vector<RenderTargetDesc> targets;

    [[nodiscard]] GraphTextureId addTexture(GraphTextureDesc desc)
    {
        const auto id = GraphTextureId { static_cast<GraphTextureId::Index>(textures.size()) };
        textures.push_back(std::move(desc));
        return id;
    }

    [[nodiscard]] RenderPassId addPass(RenderPassDesc desc)
    {
        const auto id = RenderPassId { static_cast<RenderPassId::Index>(passes.size()) };
        passes.push_back(std::move(desc));
        return id;
    }

    [[nodiscard]] TargetId addTarget(RenderTargetDesc desc)
    {
        const auto id = TargetId { static_cast<TargetId::Index>(targets.size()) };
        targets.push_back(std::move(desc));
        return id;
    }
};

} // namespace Galaxy
