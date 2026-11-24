#include "RenderGraph.hpp"

#include "engine/core/Log.hpp"
#include "engine/rendering/backend/Backend.hpp"

namespace Galaxy {
RenderPassInvocation& RenderGraph::generateInvocation(RenderPassId pass)
{
    auto& invocations = m_invocationsByPass[pass.index];
    invocations.emplace_back();
    invocations.back().passId = pass;
    return invocations.back();
}

void RenderGraph::clearInvocations()
{
    for(auto& invocation: m_invocationsByPass){
        invocation.clear();
    }
}

std::vector<RenderPassInvocation> RenderGraph::getInvocationsInOrder() const
{
    std::vector<RenderPassInvocation> res;

    for(const auto& pass : m_compiled.passes){
        for(const auto& passInvocation: m_invocationsByPass[pass.id.index]){
            res.push_back(passInvocation);
        }
    }

    return res;
}







const GraphTextureHandle RenderPassResources::findInputTexture(std::string_view samplerName) const
{
    for (std::size_t index = 0; index < inputTextureNames.size(); ++index) {
        if (inputTextureNames[index] == samplerName)
            return inputTextures[index];
    }
    
    return GraphTextureHandle();
}

void RenderGraph::initializeTexturesHandle(Backend& backend)
{

    for (const CompiledGraphTexture& texture : m_compiled.textures) {
        const GraphResourceDescTexture& textureDesc = getTextureDescription(texture.textureId);

        if (textureDesc.dimension == TextureDimension::CUBEMAP) {
            const CubemapHandle gpuHandle = backend.instantiateCubemap(
                textureDesc.textureSize.x, 
                textureDesc.format, 
                textureDesc.filter);
            m_graphTextureHandles.at(texture.textureId.index) = gpuHandle;
            backend.setTextureWrap(
                gpuHandle, 
                textureDesc.wrapS, 
                textureDesc.wrapT, 
                textureDesc.wrapR);
            backend.setTextureMipLevels(gpuHandle, textureDesc.mipLevels);
        } else {
            const std::size_t layerCount = textureDesc.dimension == TextureDimension::TEXTURE_2D_ARRAY
            ? textureDesc.arrayLayers
            : 0U;
            const TextureHandle gpuHandle = backend.instantiateTexture(
                textureDesc.format, 
                textureDesc.textureSize, 
                textureDesc.filter, 
                layerCount);
            m_graphTextureHandles.at(texture.textureId.index) = gpuHandle;
            backend.setTextureWrap(gpuHandle, textureDesc.wrapS, textureDesc.wrapT);
            backend.setTextureMipLevels(gpuHandle, textureDesc.mipLevels);
        }
    }
}

void RenderGraph::recompile(Backend& backend)
{
    m_compiled = CompiledRenderGraph(m_declaration);
    m_invocationsByPass.resize(m_declaration.getPasseIds().size());

    for (const CompiledRenderPass& pass : m_compiled.passes)
        m_invocationsByPass[pass.id.index].reserve(1);

    m_passes.clear();
    m_graphTextureHandles.clear();
    m_graphTextureHandles.resize(m_declaration.textures.size());

    initializeTexturesHandle(backend);
    int temporarySize = 1024;


    std::vector<GraphFramebufferHandle> targetFramebuffers(m_declaration.targets.size());
    m_passes.reserve(m_compiled.passes.size());

    for (const CompiledRenderPass& pass : m_compiled.passes) {
        const PassDescRender& passDesc = getPassDescription(pass.id);
        const TargetDescRender& passTarget = m_declaration.targets.at(passDesc.targetId.index);
        const bool cubemapTarget = targetUsesCubemaps(passTarget);

        RenderPassResources passResources;
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
                    ? getTextureDescription(*passTarget.depthAttachment).arrayLayers
                    : 0U;
                const FramebufferTextureFormat format = hasDepth && colorCount > 0
                    ? FramebufferTextureFormat::DEPTH24RGBA8
                    : hasDepth ? FramebufferTextureFormat::DEPTH24STENCIL8 : FramebufferTextureFormat::RGBA8;

                targetFramebuffer = backend.instantiateFrameBuffer(
                    temporarySize,
                    temporarySize,
                    format,
                    static_cast<unsigned int>(colorCount),
                    depthLayerCount);
            }
        }
        passResources.targetFramebufferHandle = targetFramebuffer;

        for (const InputDescTexture& inputTexture : passDesc.inputTextures) {
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

bool RenderGraph::targetUsesCubemaps(const TargetDescRender& target)
{
    std::optional<bool> usesCubemaps;
    auto inspectAttachment = [&](GraphResourceIdTexture id) {
        const bool attachmentUsesCubemaps = getTextureDescription(id).dimension == TextureDimension::CUBEMAP;
        if (usesCubemaps && *usesCubemaps != attachmentUsesCubemaps) {
            GLX_CORE_ERROR("A render target cannot mix cubemap and non-cubemap attachments");
            return false;
        }
        usesCubemaps = attachmentUsesCubemaps;
        return true;
    };

    for (GraphResourceIdTexture colorAttachment : target.colorAttachments) {
        if (!inspectAttachment(colorAttachment))
            return false;
    }
    if (target.depthAttachment && !inspectAttachment(*target.depthAttachment))
        return false;

    return usesCubemaps.value_or(false);
}

void RenderGraph::prepareResources(Backend& backend)
{
    if(m_declaration.isDirty()){
        recompile(backend);
        m_declaration.noMoreDirty();

        m_lastFB = std::get<FramebufferHandle>(m_passes.back().targetFramebufferHandle);
    }

    for(auto& texture : m_compiled.textures){
        auto& textureDesc = getTextureDescription(texture.textureId);
        if(textureDesc.matchViewSize){
            auto& textureHandle = getTextureHandle(texture.textureId);
            
            switch(textureDesc.dimension){
                case TextureDimension::TEXTURE_2D:
                case TextureDimension::TEXTURE_2D_ARRAY:
                    backend.setTextureSize(std::get<TextureHandle>(textureHandle), m_viewSize);
                    break;
                case TextureDimension::CUBEMAP:
                    backend.setTextureSize(std::get<CubemapHandle>(textureHandle), m_viewSize.x);
                    break;
            }
        }
    }
}

bool RenderGraph::hasFramebuffer(const GraphFramebufferHandle& handle)
{
    return std::visit([](const auto& typedHandle) { return static_cast<bool>(typedHandle); }, handle);
}
} // namespace Galaxy
