#pragma once

#include "core/pch.hpp"
#include "engine/rendering/GPUInstances/GpuResourceHandle.hpp"
#include "engine/types/Render.hpp"
#include "engine/types/Math.hpp"

#include <cstdint>
#include <utility>
#include <limits>

namespace Galaxy {
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




using RenderPassId = GenericId;
using RenderTargetId = GenericId;

using GraphResourceIdTexture = GenericId;


struct GraphResourceDescTexture{
    std::string name;
    TextureFormat format = TextureFormat::RGBA;
    TextureDimension dimension = TextureDimension::TEXTURE_2D;
    math::ivec2 textureSize = math::ivec2(512);
    bool matchViewSize = false; // GLX-TODO: match fb size rather ?
    std::uint32_t arrayLayers = 0; // GLX-TODO: only usable when array texture, should be there ?

    TextureFiltering filter = TextureFiltering::LINEAR;

    TextureWrap wrapS = TextureWrap::REPEAT;
    TextureWrap wrapT = TextureWrap::REPEAT;
    TextureWrap wrapR = TextureWrap::CLAMP_TO_EDGE;

    unsigned int mipLevels = 1;

    bool hasDepth() const
    {
        return format == TextureFormat::DEPTH24STENCIL8 || format == TextureFormat::DEPTH;
    }
};


struct TargetDescRender {
    std::vector<GraphResourceIdTexture> colorAttachments;
    std::optional<GraphResourceIdTexture> depthAttachment;
};


struct InputDescTexture {
    GraphResourceIdTexture texture;
    std::string samplerName;
};

struct PassDescRender {
    std::string name;
    ProgramHandle associatedProgram;

    // RenderState
    BlendMode blend = BlendMode::DISABLED;
    bool depthTest = true;
    bool depthWrite = true;
    bool clear = true;
    bool bindMaterials = false;
    ////////
    
    std::vector<InputDescTexture> inputTextures;

    RenderTargetId targetId;
};

struct RenderGraphDeclaration {
    std::vector<GraphResourceDescTexture> textures;
    std::vector<PassDescRender> passes;
    std::vector<TargetDescRender> targets;

    GraphResourceIdTexture addResource(GraphResourceDescTexture desc)
    {
        const auto id = GraphResourceIdTexture { static_cast<GraphResourceIdTexture::Index>(textures.size()) };
        textureIds.push_back(id);
        textures.push_back(std::move(desc));
        m_dirty = true;
        return id;
    }

    RenderPassId addPass(PassDescRender desc)
    {
        const auto id = RenderPassId { static_cast<RenderPassId::Index>(passes.size()) };
        passeIds.push_back(id);
        passes.push_back(std::move(desc));
        m_dirty = true;
        return id;
    }

    RenderTargetId addTarget(TargetDescRender desc)
    {
        const auto id = RenderTargetId { static_cast<RenderTargetId::Index>(targets.size()) };
        targetIds.push_back(id);
        targets.push_back(std::move(desc));
        m_dirty = true;
        return id;
    }

    GraphResourceDescTexture getGraphResource(const GraphResourceIdTexture& textureId){
        return textures[textureId.index];
    }

    std::vector<RenderPassId> getPasseIds() const { return passeIds; }
    std::vector<GraphResourceIdTexture> getTextureIds() const { return textureIds; }
    std::vector<RenderTargetId> getTargetIds() const { return targetIds; }

    bool isDirty() const {return m_dirty;}
    // GLX-TODO: bad method name
    void noMoreDirty(){m_dirty = false;}

    private:
    std::vector<RenderPassId> passeIds;
    std::vector<GraphResourceIdTexture> textureIds;
    std::vector<RenderTargetId> targetIds;

    bool m_dirty = true;
};

} // namespace Galaxy
