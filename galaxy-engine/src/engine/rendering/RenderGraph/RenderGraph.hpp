#pragma once
#include "pch.hpp"

#include "rendering/Commands.hpp"
#include "rendering/frontend/RenderItem.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "rendering/GPUInstances/Program.hpp"
#include "rendering/RenderGraph/RenderGraphDeclaration.hpp"
#include "RenderGraphCompilation.hpp"

#include "types/Math.hpp"



namespace Galaxy
{
    class Backend;

    // GLX-TODO: the use of variant doesnt seem to be the right choice
    using GraphTextureHandle = std::variant<TextureHandle, CubemapHandle>;
    using GraphFramebufferHandle = std::variant<FramebufferHandle, CubemapFramebufferHandle>;


    struct UniformBufferBinding {
        BufferHandle buffer;
        std::uint32_t bindingPoint;
    };

    struct TextureBinding {
        std::variant<TextureHandle, CubemapHandle> texture;
        std::string samplerName;
    };

    using UniformBufferBindingSet = std::vector<UniformBufferBinding>;
    using ShaderParameterSet = std::unordered_map<std::string, ShaderParameterValue>;

    struct RenderPassInvocation {
        RenderPassId passId;

        std::vector<RenderItem> items;
        ShaderParameterSet parameters;
        UniformBufferBindingSet uniformBindings;
        std::vector<UpdateUBOCommand> updates;
        std::vector<TextureBinding> textureBindings;
        std::optional<RenderTargetHandle> targetFramebuffer;
        unsigned int targetMip = 0;
        // TODO-GLX: maybe define textures / cubemap as targets and not framebuffers
        std::optional<CubemapHandle> targetColorCubemap;

        vec4 clearColor = vec4(1.0, 0.0, 0.0, 1.0);
        ivec2 viewportSize = ivec2(512);
        ivec2 viewportPosition = ivec2(0);

        // -1 selects a regular 2D attachment; otherwise select an array layer or cubemap face.
        int targetLayer = -1;
    };


    struct RenderPassResources {
        std::vector<GraphTextureHandle> inputTextures;
        std::vector<std::string> inputTextureNames;
        std::vector<size_t> inputTexturesLocations;
        ProgramHandle program;

        const GraphTextureHandle findInputTexture(std::string_view samplerName) const;

        // GLX-TODO: put target handling outside of renderPass
        GraphFramebufferHandle targetFramebufferHandle;
    };

    class RenderGraph {
    public:
        std::size_t getPassIndex(RenderPassId id) const {
            return m_compiled.executionIndexByPass.at(id.index);
        }

        const RenderPassResources& getRenderPass(RenderPassId id) const {
            return m_passes.at(getPassIndex(id));
        }

        const GraphTextureHandle& getTextureHandle(GraphResourceIdTexture id) const {
            return m_graphTextureHandles.at(id.index);
        }

        FramebufferHandle getLastFramebufferHandle(){return m_lastFB;}
        static bool hasFramebuffer(const GraphFramebufferHandle& handle);

        std::size_t getPassCount() const {
            return m_passes.size();
        }

        const GraphResourceDescTexture& getTextureDescription(GraphResourceIdTexture id) const {
            return m_declaration.textures[id.index];
        }
        const PassDescRender& getPassDescription(RenderPassId id) const {
            return m_declaration.passes[id.index];
        }

        RenderGraphDeclaration& getDeclaration() { return m_declaration; }

        RenderPassInvocation& generateInvocation(RenderPassId pass);
        void clearInvocations();

        std::vector<RenderPassInvocation> getInvocationsInOrder() const;

        void prepareResources(Backend& backend);
        void setViewSize(math::ivec2 newSize){m_viewSize = newSize;}
        
    private:
        void initializeTexturesHandle(Backend& backend);
        void recompile(Backend& backend);

        bool targetUsesCubemaps(const TargetDescRender& target);

        CompiledRenderGraph m_compiled;
        RenderGraphDeclaration m_declaration;

        std::vector<RenderPassResources> m_passes;
        std::vector<GraphTextureHandle> m_graphTextureHandles;
        math::ivec2 m_viewSize;

        std::vector<std::vector<RenderPassInvocation>> m_invocationsByPass;

        FramebufferHandle m_lastFB;
    };
} // namespace Galaxy




