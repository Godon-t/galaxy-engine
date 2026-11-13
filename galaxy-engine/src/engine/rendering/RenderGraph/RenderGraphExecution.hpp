#pragma once

#include "rendering/Commands.hpp"
#include "rendering/RenderGraph/RenderGraphDeclaration.hpp"
#include "rendering/frontend/RenderItem.hpp"
#include "engine/types/Math.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Galaxy
{
    class RenderGraph;
    class Program;

    struct UniformBufferBinding {
        BufferHandle buffer;
        std::uint32_t bindingPoint;
    };

    using UniformBufferBindingSet =
        std::vector<UniformBufferBinding>;

    bool applyShaderParameter(
        const Program& program,
        const std::string& name,
        const ShaderParameterValue& parameter);

    using ShaderParameterSet = std::unordered_map<std::string, ShaderParameterValue>;

    struct TextureBinding {
        std::variant<TextureHandle, CubemapHandle> texture;
        std::string samplerName;
    };

    struct RenderPassInvocation {
        std::vector<RenderItem> items;
        ShaderParameterSet parameters;
        ShaderParameterSet viewParameters;
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

    struct RenderPassExecution {
        RenderPassId pass;

        std::vector<RenderPassInvocation> invocations;
    };

    struct RenderGraphExecution {
        explicit RenderGraphExecution(RenderGraph& renderGraph);

        [[nodiscard]] RenderPassExecution& getPass(RenderPassId pass);
        [[nodiscard]] RenderPassInvocation& addInvocation(RenderPassId pass);
        void addInvocation(RenderPassId pass, RenderPassInvocation invocation);

        std::vector<RenderPassExecution> passes;
        RenderGraph& graph;
    };

} // namespace Galaxy
