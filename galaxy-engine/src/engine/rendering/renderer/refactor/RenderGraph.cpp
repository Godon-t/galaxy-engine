#include "RenderGraph.hpp"

#include "engine/core/Log.hpp"
#include "engine/rendering/renderer/Backend.hpp"

#include <optional>

namespace Galaxy {
const GraphTextureHandle RenderPassResources::findInputTexture(
    std::string_view samplerName) const
{
    for (std::size_t index = 0; index < inputTextureNames.size(); ++index) {
        if (inputTextureNames[index] == samplerName)
            return inputTextures[index];
    }
    
    return GraphTextureHandle();
}

bool targetUsesCubemaps(const CompiledRenderGraph& graph, const RenderTargetDesc& target)
{
    std::optional<bool> usesCubemaps;
    auto inspectAttachment = [&](GraphTextureId id) {
        const bool attachmentUsesCubemaps =
            graph.getTextureDescription(id).dimension == TextureDimension::Cubemap;
        if (usesCubemaps && *usesCubemaps != attachmentUsesCubemaps) {
            GLX_CORE_ERROR("A render target cannot mix cubemap and non-cubemap attachments");
            return false;
        }
        usesCubemaps = attachmentUsesCubemaps;
        return true;
    };

    for (GraphTextureId colorAttachment : target.colorAttachments) {
        if (!inspectAttachment(colorAttachment))
            return false;
    }
    if (target.depthAttachment && !inspectAttachment(*target.depthAttachment))
        return false;

    return usesCubemaps.value_or(false);
}

void RenderGraph::build(const CompiledRenderGraph& compiledGraph, Backend& backend)
{
    compilation = compiledGraph;
    m_passes.clear();
    m_graphTextureHandles.clear();
    m_graphTextureHandles.resize(compiledGraph.declaration.textures.size());

    constexpr unsigned int temporarySize = 512;
    for (const CompiledGraphTexture& texture : compiledGraph.textures) {
        const GraphTextureDesc& textureDesc = compiledGraph.getTextureDescription(texture.textureId);

        if (textureDesc.dimension == TextureDimension::Cubemap) {
            const CubemapHandle gpuHandle = backend.instantiateCubemap(
                temporarySize, textureDesc.format, textureDesc.filter);
            m_graphTextureHandles.at(texture.textureId.index) = gpuHandle;
            backend.setTextureWrap(
                gpuHandle, textureDesc.wrapS, textureDesc.wrapT, textureDesc.wrapR);
        } else {
            const std::size_t layerCount = textureDesc.dimension == TextureDimension::Texture2DArray
                ? textureDesc.arrayLayers
                : 0U;
            const TextureHandle gpuHandle = backend.instantiateTexture(
                textureDesc.format, math::vec2(temporarySize), textureDesc.filter, layerCount);
            m_graphTextureHandles.at(texture.textureId.index) = gpuHandle;
            backend.setTextureWrap(gpuHandle, textureDesc.wrapS, textureDesc.wrapT);
        }
    }

    std::vector<GraphFramebufferHandle> targetFramebuffers(compiledGraph.declaration.targets.size());
    m_passes.reserve(compiledGraph.passes.size());

    for (const CompiledRenderPass& pass : compiledGraph.passes) {
        const RenderPassDesc& passDesc = compiledGraph.getPassDescription(pass.id);
        const RenderTargetDesc& passTarget = compiledGraph.declaration.targets.at(passDesc.targetId.index);
        const bool cubemapTarget = targetUsesCubemaps(compiledGraph, passTarget);

        RenderPassResources passResources;
        passResources.passName = passDesc.name;
        passResources.program = passDesc.associatedProgram;

        GraphFramebufferHandle& targetFramebuffer = targetFramebuffers.at(pass.renderTargetId.index);
        if (!hasFramebuffer(targetFramebuffer)) {
            const bool hasDepth = passTarget.depthAttachment.has_value();
            const std::size_t colorCount = passTarget.colorAttachments.size();

            if (cubemapTarget) {
                targetFramebuffer = backend.instantiateCubemapFrameBuffer(
                    temporarySize, static_cast<unsigned int>(colorCount));
            } else {
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
        }
        passResources.targetFramebufferHandle = targetFramebuffer;

        for (const SampledTextureInputDesc& inputTexture : passDesc.inputTextures) {
            passResources.inputTextures.push_back(getTextureHandle(inputTexture.texture));
            passResources.inputTextureNames.push_back(inputTexture.samplerName);
            passResources.inputTexturesLocations.push_back(
                backend.getUniformLocation(passDesc.associatedProgram, inputTexture.samplerName));
        }

        for (std::size_t attachmentIndex = 0;
             attachmentIndex < passTarget.colorAttachments.size();
             ++attachmentIndex) {
            const GraphTextureHandle& textureHandle =
                getTextureHandle(passTarget.colorAttachments[attachmentIndex]);

            if (cubemapTarget) {
                const auto* cubemap = std::get_if<CubemapHandle>(&textureHandle);
                const auto* framebuffer = std::get_if<CubemapFramebufferHandle>(&targetFramebuffer);
                if (cubemap == nullptr || framebuffer == nullptr) {
                    GLX_CORE_ERROR("Cubemap render target has incompatible color attachments");
                    continue;
                }
                backend.attachColorCubemapToFramebuffer(
                    *cubemap, *framebuffer, static_cast<int>(attachmentIndex));
            } else {
                const auto* texture2D = std::get_if<TextureHandle>(&textureHandle);
                const auto* framebuffer = std::get_if<FramebufferHandle>(&targetFramebuffer);
                if (texture2D == nullptr || framebuffer == nullptr) {
                    GLX_CORE_ERROR("2D render target has incompatible color attachments");
                    continue;
                }
                backend.attachColorTextureToFramebuffer(
                    *texture2D, *framebuffer, static_cast<int>(attachmentIndex));
            }
        }

        if (passTarget.depthAttachment) {
            const GraphTextureHandle& textureHandle = getTextureHandle(*passTarget.depthAttachment);
            if (cubemapTarget) {
                const auto* cubemap = std::get_if<CubemapHandle>(&textureHandle);
                const auto* framebuffer = std::get_if<CubemapFramebufferHandle>(&targetFramebuffer);
                if (cubemap != nullptr && framebuffer != nullptr)
                    backend.attachDepthCubemapToFramebuffer(*cubemap, *framebuffer);
                else
                    GLX_CORE_ERROR("Cubemap render target has an incompatible depth attachment");
            } else {
                const auto* texture2D = std::get_if<TextureHandle>(&textureHandle);
                const auto* framebuffer = std::get_if<FramebufferHandle>(&targetFramebuffer);
                if (texture2D != nullptr && framebuffer != nullptr)
                    backend.attachDepthTextureToFramebuffer(*texture2D, *framebuffer);
                else
                    GLX_CORE_ERROR("2D render target has an incompatible depth attachment");
            }
        }

        m_passes.push_back(std::move(passResources));
    }
}

bool RenderGraph::hasFramebuffer(const GraphFramebufferHandle& handle)
{
    return std::visit([](const auto& typedHandle) { return static_cast<bool>(typedHandle); }, handle);
}
} // namespace Galaxy
