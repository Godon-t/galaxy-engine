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

    // One concrete rendering of a pass: one camera, light, cubemap face, etc.
    struct RenderPassInvocation {
        std::vector<RenderItem> items;
        ShaderParameterSet parameters;
        ShaderParameterSet viewParameters;
        UniformBufferBindingSet uniformBindings;
        std::vector<UpdateUBOCommand> updates;
        std::vector<TextureBinding> textureBindings;
        std::optional<RenderTargetHandle> targetFramebuffer;

        vec4 clearColor = vec4(1.0, 0.0, 0.0, 1.0);
        ivec2 viewportSize = ivec2(512);
        ivec2 viewportPosition = ivec2(0);

        // -1 selects a regular 2D attachment; otherwise select an array layer or cubemap face.
        int targetLayer = -1;
    };

    struct RenderPassExecution {
        RenderPassId pass;

        // Zero invocations skip this pass for the frame; several repeat only this pass.
        std::vector<RenderPassInvocation> invocations;
    };

    struct RenderPassInvocationIndex {
        RenderPassId pass;
        std::size_t invocationIndex;
    };

    // Execution phase: dynamic work submitted for one frame to the unique render graph.
    struct RenderGraphExecution {
        explicit RenderGraphExecution(RenderGraph& renderGraph);

        [[nodiscard]] RenderPassExecution& getPass(RenderPassId pass);
        [[nodiscard]] RenderPassInvocation& addInvocation(RenderPassId pass);

        std::vector<RenderPassExecution> passes;
        // Preserve complete view pipelines while their passes share intermediate textures.
        std::vector<RenderPassInvocationIndex> invocationOrder;
        RenderGraph& graph;
    };

} // namespace Galaxy
