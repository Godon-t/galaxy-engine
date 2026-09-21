#include "RenderGraph.hpp"

#include "engine/rendering/renderer/Backend.hpp"
#include "engine/core/Log.hpp"

namespace Galaxy {

void RenderGraph::build(const CompiledRenderGraph& compiledGraph, Backend& backend)
{
    compilation = compiledGraph;
    m_passes.clear();
    m_graphTextureHandles.clear();

    constexpr unsigned int temporarySize = 512;
    m_graphTextureHandles.reserve(compiledGraph.textures.size());
    for (const CompiledGraphTexture& texture : compiledGraph.textures) {
        const GraphTextureDesc& textureDesc = compiledGraph.getTextureDescription(texture.textureId);
        auto gpuHandle = backend.instantiateTexture(textureDesc.format, math::vec2(temporarySize));
        m_graphTextureHandles.push_back(gpuHandle);
        backend.setTextureWrap(gpuHandle, textureDesc.wrapS, textureDesc.wrapT);
    }

    std::vector<FramebufferHandle> targetFramebuffers(compiledGraph.declaration.targets.size());
    m_passes.reserve(compiledGraph.passes.size());

    for (const CompiledRenderPass& pass : compiledGraph.passes) {
        const RenderPassDesc& passDesc = compiledGraph.getPassDescription(pass.id);
        const RenderTargetDesc& passTarget = compiledGraph.declaration.targets.at(passDesc.targetId.index);

        RenderPassResources passResources;
        passResources.passName = passDesc.name;
        passResources.program = passDesc.associatedProgram;

        FramebufferHandle& targetFramebuffer = targetFramebuffers.at(pass.renderTargetId.index);
        if (!targetFramebuffer) {
            const bool hasDepth = passTarget.depthAttachment.has_value();
            const std::size_t colorCount = passTarget.colorAttachments.size();
            const unsigned int depthLayerCount = hasDepth
                ? compiledGraph.getTextureDescription(*passTarget.depthAttachment).arrayLayers
                : 0U;
            const FramebufferTextureFormat format = hasDepth && colorCount > 0
                ? FramebufferTextureFormat::DEPTH24RGBA8
                : hasDepth ? FramebufferTextureFormat::DEPTH24STENCIL8 : FramebufferTextureFormat::RGBA8;

            targetFramebuffer = backend.instanciateFrameBuffer(
                temporarySize,
                temporarySize,
                format,
                static_cast<unsigned int>(colorCount),
                depthLayerCount);
        }
        passResources.targetFramebufferHandle = targetFramebuffer;

        for (const SampledTextureInputDesc& inputTexture : passDesc.inputTextures) {
            passResources.inputTextures.push_back(getTextureHandle(inputTexture.texture));
            passResources.inputTexturesLocations.push_back(
                backend.getUniformLocation(passDesc.associatedProgram, inputTexture.samplerName));
        }

        for (std::size_t attachmentIndex = 0;
             attachmentIndex < passTarget.colorAttachments.size();
             ++attachmentIndex) {
            backend.attachColorTextureToFramebuffer(
                getTextureHandle(passTarget.colorAttachments[attachmentIndex]),
                passResources.targetFramebufferHandle,
                static_cast<int>(attachmentIndex));
        }

        if (passTarget.depthAttachment) {
            backend.attachDepthTextureToFramebuffer(
                getTextureHandle(*passTarget.depthAttachment),
                passResources.targetFramebufferHandle);
        }

        m_passes.push_back(std::move(passResources));
    }
}

} // namespace Galaxy
