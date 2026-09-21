#pragma once

#include "engine/rendering/renderer/resources/GpuResourceHandle.hpp"
#include "RenderGraphCompilation.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace Galaxy
{
    class Backend;

    struct RenderPassResources {
        std::string passName;
        std::vector<TextureHandle> inputTextures;
        std::vector<size_t> inputTexturesLocations;
        ProgramHandle program;

        // GLX-TODO: put target handling outside of renderPass
        FramebufferHandle targetFramebufferHandle;
    };

    // Runtime graph: owns the compiled schedule and its allocated GPU resources.
    class RenderGraph {
    public:
        CompiledRenderGraph compilation;

        void build(const CompiledRenderGraph& compiledGraph, Backend& backend);

        [[nodiscard]] std::size_t getPassIndex(RenderPassId id) const {
            return compilation.executionIndexByPass.at(id.index);
        }

        [[nodiscard]] const RenderPassResources& getRenderPass(RenderPassId id) const {
            return m_passes.at(getPassIndex(id));
        }

        [[nodiscard]] const RenderPassResources& getRenderPassAt(std::size_t executionIndex) const {
            return m_passes.at(executionIndex);
        }

        [[nodiscard]] TextureHandle getTextureHandle(GraphTextureId id) const {
            return m_graphTextureHandles.at(id.index);
        }

        [[nodiscard]] std::size_t getPassCount() const {
            return m_passes.size();
        }
        
    private:
        std::vector<RenderPassResources> m_passes;
        std::vector<TextureHandle> m_graphTextureHandles;
    };
} // namespace Galaxy




