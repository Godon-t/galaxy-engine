#pragma once

#include "engine/rendering/GPUInstances/GpuResourceHandle.hpp"
#include "RenderGraphCompilation.hpp"
#include "types/Math.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <vector>


namespace Galaxy
{
    class Backend;

    // GLX-TODO: the use of variant doesnt seem to be the right choice
    using GraphTextureHandle = std::variant<TextureHandle, CubemapHandle>;
    using GraphFramebufferHandle = std::variant<FramebufferHandle, CubemapFramebufferHandle>;

    struct RenderPassResources {
        std::string passName;
        std::vector<GraphTextureHandle> inputTextures;
        std::vector<std::string> inputTextureNames;
        std::vector<size_t> inputTexturesLocations;
        ProgramHandle program;

        const GraphTextureHandle findInputTexture(
            std::string_view samplerName) const;

        // GLX-TODO: put target handling outside of renderPass
        GraphFramebufferHandle targetFramebufferHandle;
    };

    class RenderGraph {
    public:
        CompiledRenderGraph compilation;

        void build(const CompiledRenderGraph& compiledGraph, Backend& backend);

        std::size_t getPassIndex(RenderPassId id) const {
            return compilation.executionIndexByPass.at(id.index);
        }

        const RenderPassResources& getRenderPass(RenderPassId id) const {
            return m_passes.at(getPassIndex(id));
        }

        const RenderPassResources& getRenderPassAt(std::size_t executionIndex) const {
            return m_passes.at(executionIndex);
        }

        const GraphTextureHandle& getTextureHandle(GraphTextureId id) const {
            return m_graphTextureHandles.at(id.index);
        }

        std::size_t getPassCount() const {
            return m_passes.size();
        }

        void prepareResources(Backend& backend);
        void setViewSize(math::ivec2 newSize){m_viewSize = newSize;}

        static bool hasFramebuffer(const GraphFramebufferHandle& handle);
        
    private:
        std::vector<RenderPassResources> m_passes;
        std::vector<GraphTextureHandle> m_graphTextureHandles;
        math::ivec2 m_viewSize;
    };
} // namespace Galaxy




